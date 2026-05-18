#pragma once
#include <string>
#include <SDL3/SDL.h> // We wrap SDL here so other files don't need to include it
#include <functional>

#include "Runtime/Interfaces/IWindow.h"

namespace Rvelent {


    class Window : public IWindow {
    public:
        Window(const WindowProps& props = WindowProps());
        virtual ~Window();

        using EventCallback = std::function<void(const SDL_Event&)>;
        void OnUpdate(const EventCallback& callback) override; // Polls events

        uint32_t GetWidth() const override { return (uint32_t)m_Data.Width; }
        uint32_t GetHeight() const override { return (uint32_t)m_Data.Height; }
        
        // Get SDL window and window ID
        void* GetNativeWindow() const override { return m_Window; }
        struct SDL_Window* GetWindow() const { return m_Window; } // Added for ImGui
        SDL_WindowID GetWindowID() const { return SDL_GetWindowID(m_Window); }
        // Helper specifically for Windows HWND if needed elsewhere
        void* GetNativeHandle() const override;

        bool IsCloseRequested() const override { return m_CloseRequested; }
        
        void SetVSync(bool enabled) override;
        bool IsVSync() const override;

    private:
        void Init(const WindowProps& props);
        void Shutdown();

        struct ::SDL_Window* m_Window;
        bool m_CloseRequested = false;

        struct WindowData {
            std::string Title;
            int Width;
            int Height;
            bool VSync = true;
        };
        WindowData m_Data;
    };
}