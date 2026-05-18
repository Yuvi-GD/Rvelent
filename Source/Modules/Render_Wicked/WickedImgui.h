#pragma once

#include "WickedEngine.h"
#include <SDL3/SDL.h>
#include "imgui.h"

namespace Rvelent {

    class WickedImgui {
    public:
        WickedImgui();
        ~WickedImgui();

        void Initialize(struct SDL_Window* window);
        void Shutdown();

        void NewFrame();
        // Always call this to finalize ImGui frame (calls ImGui::Render)
        // Returns the ImDrawData* which can then be passed to Render()
        ImDrawData* EndFrame();
        // Call inside render pass to draw. ImGui::Render() must have been called by EndFrame() before this.
        void Render(ImDrawData* draw_data, wi::graphics::CommandList cmd);
        
        // Input handling if needed, but ImGui_ImplSDL3 handles most
        void ProcessEvent(const SDL_Event& event);

    private:
        void CreateDeviceObjects();
        void InvalidateDeviceObjects();

        wi::graphics::Texture fontTexture;
        wi::graphics::Sampler sampler;
        wi::graphics::PipelineState imguiPSO;
        wi::graphics::Shader imguiVS;
        wi::graphics::Shader imguiPS;
        wi::graphics::InputLayout inputLayout;
        
        bool m_Initialized = false;
    };
}
