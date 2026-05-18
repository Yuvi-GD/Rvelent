#include "RvelentEngine.h"
#include <iostream>
#include <SDL3/SDL.h>
#include "imgui_impl_sdl3.h"

// Include the Concrete Renderer Implementation
#include "Modules/Render_Wicked/WickedRenderer.h"

namespace Rvelent {

    RvelentEngine::RvelentEngine() {}
    RvelentEngine::~RvelentEngine() {}

    bool RvelentEngine::Init() {
        std::cout << "[Engine] Initializing..." << std::endl;
        
        // 1. Create Window
        m_Window = std::make_unique<Window>(WindowProps("Rvelent Engine (Wicked)", 1600, 900));

        // 2. Create Renderer (Wicked)
        // This is where we wire up the specific backend
        m_Renderer = std::make_unique<WickedRenderer>();

        // 3. Initialize Renderer
        RenderInitParams params;
        params.windowHandle = m_Window->GetNativeHandle();
        params.sdlWindow = m_Window->GetWindow();
        params.width = m_Window->GetWidth();
        params.height = m_Window->GetHeight();
        params.fullscreen = false;

        if (!m_Renderer->Initialize(params)) {
             std::cout << "[Engine] Renderer failed to initialize." << std::endl;
             return false;
        }
        
        // 4. Create Editor
        m_Editor = std::make_unique<EditorLayer>(m_Window.get(), m_Renderer.get());
        m_Editor->Init();
        
        m_RendererInitialized = true;
        m_IsRunning = true;
        return true;
    }

    void RvelentEngine::Run() {
        std::cout << "[Engine] Entering Main Loop..." << std::endl;

        while (m_IsRunning) {
            // Event Loop
            m_Window->OnUpdate([this](const SDL_Event& event) {
                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    if (m_Renderer) m_Renderer->Resize(event.window.data1, event.window.data2);
                }
                ImGui_ImplSDL3_ProcessEvent(&event); // Forward events to ImGui
            });

            if (m_Window->IsCloseRequested()) m_IsRunning = false;

            // Renderer Update
            if (m_Renderer) {
                m_Renderer->Update();
                m_Renderer->BeginFrame();
                
                // Draw Editor UI
                if (m_Editor) {
                    m_Editor->Begin();
                    m_Editor->End();
                }
                
                m_Renderer->EndFrame();
            }
        }
    }

    void RvelentEngine::Shutdown() {
        if (m_Renderer) {
            m_Renderer->Shutdown();
        }
    }
}