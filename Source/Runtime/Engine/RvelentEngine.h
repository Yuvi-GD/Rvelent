#pragma once
#include <iostream>
#include <memory>
#include "Runtime/Platform/Window.h"
#include "Runtime/Interfaces/IRenderer.h"
#include "Editor/EditorLayer.h"

namespace Rvelent {

    class RvelentEngine {
    public:
        RvelentEngine();
        ~RvelentEngine();

        bool Init();
        void Run();
        void Shutdown();

    private:
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<IRenderer> m_Renderer; // Abstract Interface
        std::unique_ptr<EditorLayer> m_Editor;

        bool m_IsRunning = false;
        bool m_RendererInitialized = false;
    };
}