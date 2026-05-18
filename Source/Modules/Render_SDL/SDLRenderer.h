#pragma once
#include <SDL3/SDL.h>

namespace Rvelent {

    class SDLRenderer {
    public:
        // Initialize SDL_Renderer
        static bool Init(SDL_Window* window, int width, int height);
        
        // Prepare a new frame
        static void BeginFrame();
        
        // Submit the frame to the GPU
        static void EndFrame();

        // Handle window resize
        static void OnResize(int width, int height);

        // Shutdown renderer and free resources
        static void Shutdown();

        // Get the SDL_Renderer (for ImGui backend)
        static SDL_Renderer* GetRenderer();
    };

}
