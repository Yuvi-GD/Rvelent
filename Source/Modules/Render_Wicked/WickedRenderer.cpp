#include "WickedRenderer.h"
#include <iostream>

namespace Rvelent {

    WickedRenderer::WickedRenderer() {}
    WickedRenderer::~WickedRenderer() {}

    bool WickedRenderer::Initialize(const RenderInitParams& params) {
        std::cout << "[WickedRenderer] Initializing..." << std::endl;

        m_App.SetWindow((wi::platform::window_type)params.windowHandle);
        m_App.canvas.init(params.width, params.height);
        m_App.Initialize();
        
        // Enable Wicked Engine's OWN debug info (FPS, resolution) to verify scene renders
        m_App.infoDisplay.active = true;
        m_App.infoDisplay.fpsinfo = true;
        m_App.infoDisplay.resolution = true;
        m_App.infoDisplay.device_name = true;
        
        // Setup 3D Scene
        wi::scene::Scene& scene = wi::scene::GetScene();
        
        // Directional Light
        wi::ecs::Entity sun = scene.Entity_CreateLight("Sun");
        wi::scene::LightComponent* sun_comp = scene.lights.GetComponent(sun);
        if (sun_comp) {
            sun_comp->SetType(wi::scene::LightComponent::DIRECTIONAL);
            sun_comp->intensity = 5.0f;
            sun_comp->color = XMFLOAT3(1, 1, 1);
        }
        wi::scene::TransformComponent* sun_trans = scene.transforms.GetComponent(sun);
        if(sun_trans) sun_trans->RotateRollPitchYaw(XMFLOAT3(XM_PIDIV4, XM_PIDIV4, 0));

        // Ground plane
        scene.Entity_CreatePlane("Floor");

        // Cube
        wi::ecs::Entity cube = scene.Entity_CreateCube("Cube");
        wi::scene::TransformComponent* cube_trans = scene.transforms.GetComponent(cube);
        if(cube_trans) cube_trans->Translate(XMFLOAT3(0, 1, 0));

        // Weather 
        wi::ecs::Entity weather_ent = wi::ecs::CreateEntity();
        wi::scene::WeatherComponent& weather = scene.weathers.Create(weather_ent);
        weather.ambient = XMFLOAT3(0.5f, 0.5f, 0.5f);
        
        // Grid
        wi::renderer::SetToDrawGridHelper(true);
        
        // Camera
        m_RenderPath.camera = &wi::scene::GetCamera();
        m_RenderPath.camera->Eye = XMFLOAT3(0, 5, -8);
        m_RenderPath.camera->At = XMFLOAT3(0, 0, 0);
        m_RenderPath.camera->Up = XMFLOAT3(0, 1, 0);
        m_RenderPath.camera->SetDirty();
        m_RenderPath.camera->UpdateCamera();

        // Activate render path
        m_App.ActivatePath(&m_RenderPath, 0);

        // ImGui
        m_Imgui.Initialize(params.sdlWindow);
        m_App.imgui = &m_Imgui;

        std::cout << "[WickedRenderer] Init done. Canvas: " << params.width << "x" << params.height << std::endl;
        return true;
    }

    void WickedRenderer::Shutdown() {
        m_Imgui.Shutdown();
    }

    void WickedRenderer::Update() {}

    void WickedRenderer::BeginFrame() {
        m_Imgui.NewFrame();
    }

    void WickedRenderer::EndFrame() {
        m_App.drawData = m_Imgui.EndFrame();
        m_App.Run();
    }

    void WickedRenderer::Resize(int width, int height) {
        if (width <= 0 || height <= 0) return;
        m_App.canvas.init(width, height);
        m_RenderPath.ResizeBuffers();
    }

    void WickedRenderer::DrawImgui() {}
    
    void* WickedRenderer::GetSceneTexture() const {
        // Try rtMain first (the actual 3D scene render target)
        if (m_RenderPath.rtMain.IsValid()) {
            return (void*)&m_RenderPath.rtMain;
        }
        const wi::graphics::Texture* tex = m_RenderPath.GetLastPostprocessRT();
        if (tex && tex->IsValid()) return (void*)tex;
        return nullptr;
    }

}
