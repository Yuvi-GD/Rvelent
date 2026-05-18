#include "SDLRenderer.h"
#include <iostream>

namespace Rvelent {

    static SDL_Renderer* g_Renderer = nullptr;

    bool SDLRenderer::Init(SDL_Window* window, int width, int height) 
    {
        std::cout << "[Renderer] Initializing SDL_Renderer..." << std::endl;

        if (!window) {
            std::cout << "[Renderer] ERROR: Invalid window!" << std::endl;
            return false;
        }

        // Create SDL_Renderer (GPU accelerated)
        g_Renderer = SDL_CreateRenderer(window, nullptr);
        if (!g_Renderer) {
            std::cout << "[Renderer] ERROR: Failed to create renderer: " << SDL_GetError() << std::endl;
            return false;
        }

        std::cout << "[Renderer] SUCCESS! Using SDL_Renderer" << std::endl;
        std::cout << "[Renderer] Backend: " << SDL_GetRendererName(g_Renderer) << std::endl;
        return true;
    }

    void SDLRenderer::BeginFrame() {
        // Clear screen with dark purple color
        SDL_SetRenderDrawColor(g_Renderer, 0x44, 0x33, 0x55, 0xFF);
        SDL_RenderClear(g_Renderer);
    }

    void SDLRenderer::EndFrame() {
        // Present the frame
        SDL_RenderPresent(g_Renderer);
    }
    
    void SDLRenderer::OnResize(int width, int height) {
        // SDL_Renderer handles this automatically
    }

    void SDLRenderer::Shutdown() {
        if (g_Renderer) {
            SDL_DestroyRenderer(g_Renderer);
            g_Renderer = nullptr;
            std::cout << "[Renderer] Shutdown complete" << std::endl;
        }
    }

    SDL_Renderer* SDLRenderer::GetRenderer() {
        return g_Renderer;
    }

}
