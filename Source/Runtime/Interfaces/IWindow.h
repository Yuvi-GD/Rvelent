#pragma once
#include <string>
#include <functional>
#include <cstdint>

struct SDL_Window;
union SDL_Event;

namespace Rvelent {

    struct WindowProps {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Rvelent Engine",
                    uint32_t width = 1600,
                    uint32_t height = 900)
            : Title(title), Width(width), Height(height) {}
    };

    /**
     * @brief Abstract Window Interface
     * 
     * Abstracts the platform-specific window creation and management.
     * Currently implemented by SDL3, but allows for future GLFW/Win32/Cocoa backends.
     */
    class IWindow {
    public:
        virtual ~IWindow() = default;

        virtual void OnUpdate(const std::function<void(const SDL_Event&)>& eventCallback) = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        // Platform-specific handle (void* to keep it generic)
        virtual void* GetNativeWindow() const = 0; // Returns SDL_Window* or framework window
        virtual void* GetNativeHandle() const = 0; // Returns HWND, NSWindow, X11 Window, etc.
        
        // Window attributes
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;
        virtual bool IsCloseRequested() const = 0;

        // Factory method to create a platform-specific window
        static IWindow* Create(const WindowProps& props = WindowProps());
    };

}
