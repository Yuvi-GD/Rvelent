#pragma once
#include "Runtime/Platform/Window.h"
#include "Runtime/Interfaces/IRenderer.h"

namespace Rvelent {

    class EditorLayer {
    public:
        EditorLayer(Window* window, IRenderer* renderer);
        ~EditorLayer();

        void Init();
        void Begin();
        void End();
        
        void DrawTitleBar();
        void DrawTabBar();
        void DrawToolbar();
        void DrawBottomBar();
        void DrawLayout();
        void SetupDockingLayout();

    private:
        Window* m_Window;
        IRenderer* m_Renderer;
        
        // Panel States
        bool m_ShowViewport = true;
        bool m_ShowOutliner = true;
        bool m_ShowProperties = true;
        bool m_ShowConsole = true;
        bool m_DockingLayoutInitialized = false;

        // Viewport State
        uint32_t m_ViewportWidth = 0;
        uint32_t m_ViewportHeight = 0;
    };
}