#include "Window.h"
#include <iostream>
#include <SDL3/SDL.h>

namespace Rvelent {

    Window::Window(const WindowProps& props) {
        Init(props);
    }

    Window::~Window() {
        Shutdown();
    }

    void* Window::GetNativeHandle() const {
        if (!m_Window) {
            std::cerr << "[Window] Error: Window is NULL!" << std::endl;
            return nullptr;
        }

        // Get the properties ID
        SDL_PropertiesID props = SDL_GetWindowProperties(m_Window);
        
        // Explicitly ask for the Win32 HWND
        void* handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

        if (handle == nullptr) {
            // Fallback: Try the string literal
            handle = SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", NULL);
        }

        if (handle == nullptr) {
            std::cerr << "[Window] ERROR: Could not retrieve Native Window Handle!" << std::endl;
        } else {
            std::cout << "[Window] Native handle retrieved: " << handle << std::endl;
        }

        return handle;
    }

    // 1. Define the callback function at the top of Window.cpp
    // Define the resize border size
    const int RESIZE_BORDER = 5;

    SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data) {
        int w, h;
        SDL_GetWindowSize(win, &w, &h);

        // 1. Allow resizing from the edges (Optional but good for borderless)
        if (area->x < RESIZE_BORDER && area->y < RESIZE_BORDER) return SDL_HITTEST_RESIZE_TOPLEFT;
        if (area->x > w - RESIZE_BORDER && area->y < RESIZE_BORDER) return SDL_HITTEST_RESIZE_TOPRIGHT;
        if (area->x < RESIZE_BORDER && area->y > h - RESIZE_BORDER) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
        if (area->x > w - RESIZE_BORDER && area->y > h - RESIZE_BORDER) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
        if (area->x < RESIZE_BORDER) return SDL_HITTEST_RESIZE_LEFT;
        if (area->x > w - RESIZE_BORDER) return SDL_HITTEST_RESIZE_RIGHT;
        if (area->y < RESIZE_BORDER) return SDL_HITTEST_RESIZE_TOP;
        if (area->y > h - RESIZE_BORDER) return SDL_HITTEST_RESIZE_BOTTOM;

        // 2. The Title Bar Logic (Top 30 pixels)
        if (area->y < 30) {
            // SAFETY CHECK: If we are over the buttons (Right side), DO NOT DRAG
            // We reserve 150 pixels on the right for buttons
            if (area->x > w - 150) {
                return SDL_HITTEST_NORMAL; // Let ImGui handle the click!
            }
            
            // Otherwise, drag the window
            return SDL_HITTEST_DRAGGABLE;
        }

        // 3. The rest of the screen (Viewport, Content Browser, etc.)
        // MUST return NORMAL so ImGui can receive mouse clicks
        return SDL_HITTEST_NORMAL;
    }
    void Window::Init(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;
        m_Data.VSync = true; // Default to VSync on

        std::cout << "[Window] Creating Window: " << props.Title << std::endl;

        // 1. Initialize SDL Video
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cout << "[Window] Error: Could not init SDL: " << SDL_GetError() << std::endl;
            return;
        }

        // 2. Create the Window (borderless for custom ImGui titlebar)
        m_Window = SDL_CreateWindow(
            m_Data.Title.c_str(),
            m_Data.Width,
            m_Data.Height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS
        );

        if (!m_Window) {
            std::cout << "[Window] Error: Could not create window!" << std::endl;
            return;
        }

        // Pump events to let SDL fully process the window creation
        SDL_PumpEvents();

        // Verify window is shown
        SDL_ShowWindow(m_Window);
        SDL_RaiseWindow(m_Window);
        
        // Pump again to process the show command
        SDL_PumpEvents();
        
        std::cout << "[Window] Window created and ready" << std::endl;

        // ACTIVATE DRAGGING for custom titlebar
        SDL_SetWindowHitTest(m_Window, HitTestCallback, NULL);
    }

    void Window::Shutdown() {
        SDL_DestroyWindow(m_Window);
        SDL_Quit();
    }

    void Window::OnUpdate(const EventCallback& callback) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Forward event to callback (e.g., ImGui) first
            if (callback) {
                callback(event);
            }

            if (event.type == SDL_EVENT_QUIT) {
                m_CloseRequested = true;
            }

            // ADD THIS: Emergency Quit Button
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    m_CloseRequested = true;
                }
            }
        }
    }
    void Window::SetVSync(bool enabled) {
        // SDL3 VSync: 1 = VSync, 0 = Immediate
        SDL_SetWindowSurfaceVSync(m_Window, enabled ? 1 : 0);
        m_Data.VSync = enabled;
    }

    bool Window::IsVSync() const {
        return m_Data.VSync;
    }

    // --- Factory Implementation ---
    IWindow* IWindow::Create(const WindowProps& props) {
        return new Window(props);
    }

}