#pragma once

#include "Runtime/Interfaces/IRenderer.h"
#include "WickedEngine.h"
#include "WickedImgui.h"
#include "wiRenderPath3D.h"

namespace Rvelent {

    // Subclass wi::Application so we can inject ImGui into Compose(),
    // which runs INSIDE the active swap chain render pass.
    class RvelentApp : public wi::Application {
    public:
        WickedImgui* imgui = nullptr;
        ImDrawData* drawData = nullptr; // Set before Run() by WickedRenderer

        void Compose(wi::graphics::CommandList cmd) override {
            wi::Application::Compose(cmd); // Wicked's own UI (backlog, profiler)
            if (imgui && drawData) {
                imgui->Render(drawData, cmd); // Our ImGui, inside the render pass
            }
        }
    };

    class WickedRenderer : public IRenderer {
    public:
        WickedRenderer();
        ~WickedRenderer() override;

        bool Initialize(const RenderInitParams& params) override;
        void Shutdown() override;

        void BeginFrame() override;
        void EndFrame() override;

        void Update() override;
        void Resize(int width, int height) override;
        void DrawImgui() override;
        void* GetSceneTexture() const override;

    private:
        RvelentApp m_App;
        WickedImgui m_Imgui;
        wi::RenderPath3D m_RenderPath;
    };
}
