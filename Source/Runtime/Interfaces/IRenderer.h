#pragma once

#include <string>
#include <vector>

struct SDL_Window; // Forward declaration in global scope

namespace Rvelent {

    struct RenderInitParams {
        void* windowHandle;
        struct SDL_Window* sdlWindow; // For ImGui_ImplSDL3
        int width;
        int height;
        bool fullscreen;
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // Lifecycle
        virtual bool Initialize(const RenderInitParams& params) = 0;
        virtual void Shutdown() = 0;

        // Frame
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        // Scene Management (Placeholder for now)
        virtual void Update() = 0;
        virtual void Resize(int width, int height) = 0;
        
        // Debug/Editor
        virtual void DrawImgui() = 0;

        // Texture Retrieval for Viewport
        virtual void* GetSceneTexture() const = 0;
    };
}
