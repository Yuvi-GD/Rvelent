#include "WickedImgui.h"
#include "WickedEngine.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include <iostream>
#include <vector>
#include <cstddef>

namespace Rvelent {

    // Simple shaders embedded for now to avoid file I/O issues during dev
    // Ideally these should be loaded from .cso files
    // But since I can't easily compile them here without a tool, 
    // I will try to load them from the Example_ImGui folder if they exist, 
    // or rely on the user to ensure they are built. 
    // Actually, Wicked Engine usually compiles shaders on demand if source is present?
    // Let's assume the build process copies the .cso files or we load them from source.
    // The example loaded them as .cso. 
    // I will use wi::renderer::LoadShader which handles .cso or source.

    WickedImgui::WickedImgui() {}
    WickedImgui::~WickedImgui() { Shutdown(); }

    void WickedImgui::Initialize(SDL_Window* window) {
        if (m_Initialized) return;
        std::cout << "[WickedImgui] Initializing..." << std::endl;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        std::cout << "[WickedImgui] ImGui context created" << std::endl;
        
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        std::cout << "[WickedImgui] Initializing SDL3 backend..." << std::endl;
        ImGui_ImplSDL3_InitForVulkan(window);
        
        io.BackendRendererName = "Wicked";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        // Don't create device objects yet - graphics device might not be ready
        // Will be created on first frame if needed
        std::cout << "[WickedImgui] Initialization complete (device objects deferred)" << std::endl;
        m_Initialized = true;
    }

    void WickedImgui::Shutdown() {
        if (!m_Initialized) return;
        InvalidateDeviceObjects();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_Initialized = false;
    }

    void WickedImgui::NewFrame() {
        if (!m_Initialized) return;
        
        // Re-create font texture if missing
        if (!fontTexture.IsValid()) {
            std::cout << "[WickedImgui] NewFrame: Creating device objects..." << std::endl;
            CreateDeviceObjects();
            std::cout << "[WickedImgui] NewFrame: Device objects done. FontValid=" << fontTexture.IsValid() << std::endl;
        }

        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    ImDrawData* WickedImgui::EndFrame() {
        if (!m_Initialized) return nullptr;
        ImGui::Render();
        return ImGui::GetDrawData();
    }

    void WickedImgui::Render(ImDrawData* drawData, wi::graphics::CommandList cmd) {
        if (!m_Initialized) return;

        // Create device objects on first render if not yet created
        if (!fontTexture.IsValid()) {
            CreateDeviceObjects();
            if (!fontTexture.IsValid()) return;
        }

        if (!drawData || drawData->TotalVtxCount == 0) return;

        // Avoid rendering when minimized
        int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
        int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0) return;

        wi::graphics::GraphicsDevice* device = wi::graphics::GetDevice();
        using namespace wi::graphics;

        // Allocate transient buffers
        const uint64_t vbSize = sizeof(ImDrawVert) * drawData->TotalVtxCount;
        const uint64_t ibSize = sizeof(ImDrawIdx) * drawData->TotalIdxCount;
        auto vertexBufferAllocation = device->AllocateGPU(vbSize, cmd);
        auto indexBufferAllocation = device->AllocateGPU(ibSize, cmd);

        ImDrawVert* vertexCPUMem = reinterpret_cast<ImDrawVert*>(vertexBufferAllocation.data);
        ImDrawIdx* indexCPUMem = reinterpret_cast<ImDrawIdx*>(indexBufferAllocation.data);

        for (int n = 0; n < drawData->CmdListsCount; n++) {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            memcpy(vertexCPUMem, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(indexCPUMem, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vertexCPUMem += cmd_list->VtxBuffer.Size;
            indexCPUMem += cmd_list->IdxBuffer.Size;
        }

        // Setup Orthographic Projection
        {
            float L = drawData->DisplayPos.x;
            float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
            float T = drawData->DisplayPos.y;
            float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
            
            struct ImGuiConstants {
                float mvp[4][4];
            };
            ImGuiConstants constants;
            float mvp[4][4] = {
                { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
                { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
                { 0.0f,         0.0f,           0.5f,       0.0f },
                { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
            };
            memcpy(&constants.mvp, mvp, sizeof(mvp));
            device->BindDynamicConstantBuffer(constants, 0, cmd);
        }

        const GPUBuffer* vbs[] = { &vertexBufferAllocation.buffer };
        const uint32_t strides[] = { sizeof(ImDrawVert) };
        const uint64_t offsets[] = { vertexBufferAllocation.offset };
        device->BindVertexBuffers(vbs, 0, 1, strides, offsets, cmd);
        device->BindIndexBuffer(&indexBufferAllocation.buffer, IndexBufferFormat::UINT16, indexBufferAllocation.offset, cmd);

        Viewport viewport;
        viewport.width = (float)fb_width;
        viewport.height = (float)fb_height;
        device->BindViewports(1, &viewport, cmd);

        device->BindPipelineState(&imguiPSO, cmd);
        device->BindSampler(&sampler, 0, cmd);

        // Render command lists
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        ImVec2 clip_off = drawData->DisplayPos;

        for (int n = 0; n < drawData->CmdListsCount; n++) {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback) {
                     if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                        // Re-bind state if needed
                     }
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                }
                else {
                    // Apply Scissor
                    ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
                    ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
                    if (clip_max.x < clip_min.x || clip_max.y < clip_min.y) continue;

                    Rect scissor;
                    scissor.left = (int32_t)clip_min.x;
                    scissor.top = (int32_t)clip_min.y;
                    scissor.right = (int32_t)clip_max.x;
                    scissor.bottom = (int32_t)clip_max.y;
                    device->BindScissorRects(1, &scissor, cmd);

                    // Bind Texture
                    const Texture* texture = (const Texture*)pcmd->GetTexID();
                    if(texture) device->BindResource(texture, 0, cmd);
                    
                    device->DrawIndexed(pcmd->ElemCount, global_idx_offset + pcmd->IdxOffset, global_vtx_offset + pcmd->VtxOffset, cmd);
                }
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    void WickedImgui::ProcessEvent(const SDL_Event& event) {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }



    void WickedImgui::CreateDeviceObjects() {
         wi::graphics::GraphicsDevice* device = wi::graphics::GetDevice();
         if (!device) {
             std::cout << "[WickedImgui] ERROR: Graphics device not available!" << std::endl;
             return;
         }
         
         using namespace wi::graphics;
         std::cout << "[WickedImgui] Creating device objects..." << std::endl;

         // Fonts
         ImGuiIO& io = ImGui::GetIO();
         unsigned char* pixels;
         int width, height;
         io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

         TextureDesc texDesc;
         texDesc.width = width;
         texDesc.height = height;
         texDesc.mip_levels = 1;
         texDesc.array_size = 1;
         texDesc.format = Format::R8G8B8A8_UNORM;
         texDesc.bind_flags = BindFlag::SHADER_RESOURCE;
         
         SubresourceData data;
         data.data_ptr = pixels;
         data.row_pitch = width * 4;
         data.slice_pitch = data.row_pitch * height;
         
         device->CreateTexture(&texDesc, &data, &fontTexture);
         io.Fonts->SetTexID((ImTextureID)&fontTexture);

         // Sampler
         SamplerDesc samplerDesc;
         samplerDesc.address_u = TextureAddressMode::WRAP;
         samplerDesc.address_v = TextureAddressMode::WRAP;
         samplerDesc.address_w = TextureAddressMode::WRAP;
         samplerDesc.filter = Filter::MIN_MAG_MIP_LINEAR;
         device->CreateSampler(&samplerDesc, &sampler);

         // Shaders - manually load from compiled binaries
         std::cout << "[WickedImgui] Loading compiled shaders..." << std::endl;
         
         // Load shader binaries manually using absolute paths
         wi::vector<uint8_t> vs_bytecode, ps_bytecode;
         
         bool vs_read = wi::helper::FileRead("ImGuiVS.cso", vs_bytecode);
         bool ps_read = wi::helper::FileRead("ImGuiPS.cso", ps_bytecode);
         
         if (!vs_read || !ps_read) {
             std::cout << "[WickedImgui] ERROR: Failed to read shader files! (VS:" << vs_read << ", PS:" << ps_read << ")" << std::endl;
             std::cout << "[WickedImgui] Looking in current directory for ImGuiVS.cso and ImGuiPS.cso" << std::endl;
             return;
         }
         
         bool vs_created = device->CreateShader(ShaderStage::VS, vs_bytecode.data(), vs_bytecode.size(), &imguiVS);
         bool ps_created = device->CreateShader(ShaderStage::PS, ps_bytecode.data(), ps_bytecode.size(), &imguiPS);
         
         if (!vs_created || !ps_created) {
             std::cout << "[WickedImgui] ERROR: Failed to create shaders! (VS:" << vs_created << ", PS:" << ps_created << ")" << std::endl;
             return;
         }
         
         std::cout << "[WickedImgui] Shaders loaded and created successfully" << std::endl;

         // Input Layout is stored as a class member because Wicked Engine DX12 defers PSO creation
         // and stores the pointer directly. A stack variable would become invalid.
         inputLayout.elements = {
             { "POSITION", 0, Format::R32G32_FLOAT, 0, (uint32_t)offsetof(ImDrawVert, pos), InputClassification::PER_VERTEX_DATA },
             { "TEXCOORD", 0, Format::R32G32_FLOAT, 0, (uint32_t)offsetof(ImDrawVert, uv), InputClassification::PER_VERTEX_DATA },
             { "COLOR", 0, Format::R8G8B8A8_UNORM, 0, (uint32_t)offsetof(ImDrawVert, col), InputClassification::PER_VERTEX_DATA },
         };

         // PSO
         PipelineStateDesc psoDesc;
         psoDesc.vs = &imguiVS;
         psoDesc.ps = &imguiPS;
         psoDesc.il = &inputLayout;
         psoDesc.dss = wi::renderer::GetDepthStencilState(wi::enums::DSSTYPE_DEPTHREAD);
         psoDesc.rs = wi::renderer::GetRasterizerState(wi::enums::RSTYPE_DOUBLESIDED);
         psoDesc.bs = wi::renderer::GetBlendState(wi::enums::BSTYPE_TRANSPARENT);
         psoDesc.pt = PrimitiveTopology::TRIANGLELIST;
         
         device->CreatePipelineState(&psoDesc, &imguiPSO);
         std::cout << "[WickedImgui] PSO created successfully" << std::endl;
     }

     void WickedImgui::InvalidateDeviceObjects() {
         fontTexture = wi::graphics::Texture();
         sampler = wi::graphics::Sampler();
         imguiPSO = wi::graphics::PipelineState();
         imguiVS = wi::graphics::Shader();
         imguiPS = wi::graphics::Shader();
         inputLayout = wi::graphics::InputLayout();
     }

}
