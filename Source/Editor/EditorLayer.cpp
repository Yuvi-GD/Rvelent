#include "EditorLayer.h"
#include "Runtime/Platform/Window.h"
#include <SDL3/SDL.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl3.h"

namespace Rvelent {

    static void SetupUnrealStyle();

    EditorLayer::EditorLayer(Window* window, IRenderer* renderer) 
        : m_Window(window)
        , m_Renderer(renderer)
        , m_ShowViewport(true)
        , m_ShowOutliner(true)
        , m_ShowProperties(true)
        , m_ShowConsole(true)
        , m_DockingLayoutInitialized(false) {
    }

    EditorLayer::~EditorLayer() {}

    void EditorLayer::Init() {
        ImGuiIO& io = ImGui::GetIO();
        
        ImFontConfig fontConfig;
        fontConfig.OversampleH = 2;
        fontConfig.OversampleV = 2;
        ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, &fontConfig);
        if (font) {
            io.FontDefault = font;
        }

        // Apply theme FIRST, then override spacing for 20px font
        SetupUnrealStyle();
        
        ImGuiStyle& style = ImGui::GetStyle();
        // Scale spacing for 20px font
        style.WindowPadding   = ImVec2(10, 10);
        style.FramePadding    = ImVec2(8, 5);
        style.ItemSpacing     = ImVec2(10, 7);
        style.ItemInnerSpacing = ImVec2(8, 5);
        style.IndentSpacing   = 22.0f;
        style.ScrollbarSize   = 16.0f;
        style.GrabMinSize     = 12.0f;
        style.WindowRounding  = 3.0f;
        style.FrameRounding   = 4.0f;
        style.GrabRounding    = 3.0f;
        style.TabRounding     = 4.0f;
    }

    void EditorLayer::Begin() {
        if (!m_DockingLayoutInitialized) {
            SetupDockingLayout();
            m_DockingLayoutInitialized = true;
        }
        
        // PassthruCentralNode = 3D scene shows through the empty center area
        ImGui::DockSpaceOverViewport(ImGui::GetID("RvelentDock"), ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        
        DrawTitleBar();
        DrawTabBar();
        DrawToolbar();
        DrawBottomBar();
        DrawLayout();
    }

    void EditorLayer::End() {}

    // === TITLE BAR (Menu + Window Controls) ===
    void EditorLayer::DrawTitleBar() {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 7));
        
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    SDL_Event quitEvent;
                    quitEvent.type = SDL_EVENT_QUIT;
                    SDL_PushEvent(&quitEvent);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) { ImGui::EndMenu(); }
            if (ImGui::BeginMenu("Window")) { ImGui::EndMenu(); }

            float btnW = 40.0f;
            float barH = ImGui::GetWindowHeight();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (btnW * 3));

            if (ImGui::Button("-", ImVec2(btnW, barH)))
                SDL_MinimizeWindow((SDL_Window*)m_Window->GetNativeWindow());

            bool maxed = SDL_GetWindowFlags((SDL_Window*)m_Window->GetNativeWindow()) & SDL_WINDOW_MAXIMIZED;
            if (ImGui::Button(maxed ? "[]" : "[ ]", ImVec2(btnW, barH))) {
                if (maxed) SDL_RestoreWindow((SDL_Window*)m_Window->GetNativeWindow());
                else       SDL_MaximizeWindow((SDL_Window*)m_Window->GetNativeWindow());
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("X", ImVec2(btnW, barH))) {
                SDL_Event q; q.type = SDL_EVENT_QUIT; SDL_PushEvent(&q);
            }
            ImGui::PopStyleColor(2);

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();
    }

    // === MAIN PANELS ===
    void EditorLayer::DrawLayout() {
        // NO Viewport window — the center is empty and PassthruCentralNode
        // makes it transparent, so the 3D scene from Wicked Engine shows through!

        // --- OUTLINER ---
        if (ImGui::Begin("Outliner", &m_ShowOutliner)) {
            if (ImGui::TreeNode("Map")) {
                ImGui::TreeNodeEx("Main Camera", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
                ImGui::TreeNodeEx("Directional Light", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
                if (ImGui::TreeNode("Player")) {
                    ImGui::TreeNodeEx("Mesh", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
                    ImGui::TreeNodeEx("Collision", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();

        // --- DETAILS ---
        if (ImGui::Begin("Details", &m_ShowProperties)) {
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "StaticMeshActor");
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                static float pos[3] = { 0, 1, 0 };
                static float rot[3] = { 0, 0, 0 };
                static float scl[3] = { 1, 1, 1 };
                ImGui::Text("Location"); ImGui::SameLine(80);
                ImGui::SetNextItemWidth(-1); ImGui::DragFloat3("##Pos", pos, 0.1f);
                ImGui::Text("Rotation"); ImGui::SameLine(80);
                ImGui::SetNextItemWidth(-1); ImGui::DragFloat3("##Rot", rot, 1.0f);
                ImGui::Text("Scale");    ImGui::SameLine(80);
                ImGui::SetNextItemWidth(-1); ImGui::DragFloat3("##Scl", scl, 0.1f);
            }
            if (ImGui::CollapsingHeader("Static Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
                static char mesh[128] = "SM_Cube_01";
                static bool shadow = true;
                ImGui::Text("Mesh");     ImGui::SameLine(80); ImGui::SetNextItemWidth(-1); ImGui::InputText("##SM", mesh, 128);
                ImGui::Text("Shadow");   ImGui::SameLine(80); ImGui::Checkbox("##Sh", &shadow);
            }
            if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
                static char mat[128] = "M_Basic_Grid";
                ImGui::Text("Slot 0");   ImGui::SameLine(80); ImGui::SetNextItemWidth(-1); ImGui::InputText("##Mat", mat, 128);
            }
            if (ImGui::CollapsingHeader("Physics")) {
                static bool sim = false;
                static float mass = 1.0f;
                ImGui::Text("Simulate"); ImGui::SameLine(80); ImGui::Checkbox("##Sim", &sim);
                ImGui::Text("Mass");     ImGui::SameLine(80); ImGui::SetNextItemWidth(-1); ImGui::DragFloat("##Mass", &mass, 0.1f, 0.1f, 999.0f);
            }
        }
        ImGui::End();

        // --- CONTENT DRAWER ---
        if (ImGui::Begin("Content Drawer", &m_ShowConsole)) {
            static char searchBuf[64] = "";
            ImGui::Text("Search:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(200); ImGui::InputText("##Srch", searchBuf, 64);
            ImGui::Separator();

            float thumb = 80.0f, pad = 14.0f;
            float panelW = ImGui::GetContentRegionAvail().x;
            int cols = (int)(panelW / (thumb + pad));
            if (cols < 1) cols = 1;

            if (ImGui::BeginTable("CB", cols)) {
                const char* items[] = {"Blueprints","Materials","Meshes","Textures","Audio","Levels","Scripts"};
                for (int i = 0; i < 7; i++) {
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                    ImGui::Button("##I", ImVec2(thumb, thumb));
                    ImGui::PopStyleVar();
                    float tw = ImGui::CalcTextSize(items[i]).x;
                    float off = (thumb - tw) * 0.5f;
                    if (off > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
                    ImGui::Text("%s", items[i]);
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();

        // --- OUTPUT LOG ---
        if (ImGui::Begin("Output Log")) {
            ImGui::TextDisabled("Waiting for output...");
            ImGui::Text("[System] Engine initialized successfully.");
            ImGui::Text("[Renderer] Wicked Engine DX12 Backend loaded.");
            ImGui::TextColored(ImVec4(1,1,0,1), "[Warning] No shaders cached, compiling at runtime.");
        }
        ImGui::End();
    }

    // === TAB BAR ===
    void EditorLayer::DrawTabBar() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin("TabBar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::BeginTabBar("Tabs")) {
                if (ImGui::BeginTabItem("ThirdPersonMap")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("BP_Player"))      { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("FirstPersonMap")) { ImGui::EndTabItem(); }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // === BOTTOM BAR ===
    void EditorLayer::DrawBottomBar() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        if (ImGui::Begin("BottomBar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar)) {
            if (ImGui::Button("Content Drawer")) { m_ShowConsole = !m_ShowConsole; } ImGui::SameLine();
            if (ImGui::Button("Output Log")) {} ImGui::SameLine();
            ImGui::TextDisabled("|"); ImGui::SameLine();
            ImGui::Text("Console...");

            float rp = ImGui::GetWindowWidth() - 300.0f;
            if (rp > ImGui::GetCursorPosX()) ImGui::SameLine(rp); else ImGui::SameLine();
            if (ImGui::Button("Revision Control")) {} ImGui::SameLine();
            if (ImGui::Button("Save All")) {} ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f,0.8f,0.4f,1));
            if (ImGui::Button("All Systems Go")) {}
            ImGui::PopStyleColor();
        }
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    // === TOOLBAR ===
    void EditorLayer::DrawToolbar() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 0));
        if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar)) {
            if (ImGui::Button("Save", ImVec2(50,26))) {} ImGui::SameLine();
            if (ImGui::Button("Add",  ImVec2(42,26))) {} ImGui::SameLine();
            ImGui::TextDisabled("|"); ImGui::SameLine();
            if (ImGui::Button("Select", ImVec2(56,26))) {} ImGui::SameLine();

            float cx = ImGui::GetWindowWidth() * 0.5f - 70;
            if (cx > ImGui::GetCursorPosX()) ImGui::SameLine(cx);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f,0.4f,0.15f,1));
            if (ImGui::Button("Play",  ImVec2(50,26))) {} ImGui::PopStyleColor(); ImGui::SameLine();
            if (ImGui::Button("Pause", ImVec2(50,26))) {} ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f,0.15f,0.15f,1));
            if (ImGui::Button("Stop",  ImVec2(50,26))) {} ImGui::PopStyleColor();

            float rx = ImGui::GetWindowWidth() - 190;
            if (rx > ImGui::GetCursorPosX()) ImGui::SameLine(rx); else ImGui::SameLine();
            if (ImGui::Button("Platforms", ImVec2(72,26))) {} ImGui::SameLine();
            if (ImGui::Button("Settings", ImVec2(68,26))) {} ImGui::SameLine();
            if (ImGui::Button("...", ImVec2(26,26))) {}
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    // === DOCKING LAYOUT ===
    void EditorLayer::SetupDockingLayout() {
        ImGuiID dockId = ImGui::GetID("RvelentDock");
        
        // Only set up if no saved layout exists (imgui.ini)
        if (ImGui::DockBuilderGetNode(dockId) != nullptr) return;

        ImGui::DockBuilderRemoveNode(dockId);
        ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockId, ImGui::GetMainViewport()->WorkSize);

        ImGuiID main = dockId;
        ImGuiID right, bottom;

        ImGui::DockBuilderSplitNode(main, ImGuiDir_Right, 0.22f, &right, &main);
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Down, 0.30f, &bottom, &main);

        ImGuiID rTop, rBot;
        ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.60f, &rBot, &rTop);

        ImGuiID topBar;
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Up, 0.035f, &topBar, &main);
        ImGuiID toolbar;
        ImGui::DockBuilderSplitNode(main, ImGuiDir_Up, 0.04f, &toolbar, &main);

        ImGuiID botBar;
        ImGui::DockBuilderSplitNode(bottom, ImGuiDir_Down, 0.08f, &botBar, &bottom);

        // Dock panels — but NOT to the center! Center stays empty = 3D scene shows through
        ImGui::DockBuilderDockWindow("TabBar", topBar);
        ImGui::DockBuilderDockWindow("Toolbar", toolbar);
        // "main" is the central node — we leave it EMPTY so PassthruCentralNode works
        ImGui::DockBuilderDockWindow("Outliner", rTop);
        ImGui::DockBuilderDockWindow("Details", rBot);
        ImGui::DockBuilderDockWindow("Content Drawer", bottom);
        ImGui::DockBuilderDockWindow("Output Log", bottom);
        ImGui::DockBuilderDockWindow("BottomBar", botBar);

        ImGui::DockBuilderFinish(dockId);
    }

    // ============================
    // UE5-Style Dark Theme
    // ============================
    static void SetupUnrealStyle() {
        ImVec4* c = ImGui::GetStyle().Colors;

        c[ImGuiCol_WindowBg]            = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        c[ImGuiCol_ChildBg]             = ImVec4(0.06f, 0.06f, 0.06f, 0.00f);
        c[ImGuiCol_PopupBg]             = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        c[ImGuiCol_Border]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        c[ImGuiCol_FrameBg]             = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_FrameBgHovered]      = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        c[ImGuiCol_FrameBgActive]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        c[ImGuiCol_TitleBg]             = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        c[ImGuiCol_TitleBgActive]       = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        c[ImGuiCol_TitleBgCollapsed]    = ImVec4(0.04f, 0.04f, 0.04f, 0.75f);
        c[ImGuiCol_MenuBarBg]           = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        c[ImGuiCol_ScrollbarBg]         = ImVec4(0.05f, 0.05f, 0.05f, 0.60f);
        c[ImGuiCol_ScrollbarGrab]       = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered]= ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        c[ImGuiCol_CheckMark]           = ImVec4(0.26f, 0.56f, 0.98f, 1.00f);
        c[ImGuiCol_SliderGrab]          = ImVec4(0.26f, 0.56f, 0.98f, 0.78f);
        c[ImGuiCol_SliderGrabActive]    = ImVec4(0.26f, 0.56f, 0.98f, 1.00f);
        c[ImGuiCol_Button]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        c[ImGuiCol_ButtonHovered]       = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        c[ImGuiCol_ButtonActive]        = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        c[ImGuiCol_Header]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        c[ImGuiCol_HeaderHovered]       = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        c[ImGuiCol_HeaderActive]        = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        c[ImGuiCol_Separator]           = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        c[ImGuiCol_Tab]                 = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        c[ImGuiCol_TabHovered]          = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        c[ImGuiCol_TabActive]           = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        c[ImGuiCol_TabUnfocused]        = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        c[ImGuiCol_TabUnfocusedActive]  = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        c[ImGuiCol_DockingPreview]      = ImVec4(0.26f, 0.56f, 0.98f, 0.50f);
        c[ImGuiCol_DockingEmptyBg]      = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        c[ImGuiCol_Text]                = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        c[ImGuiCol_TextDisabled]        = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        c[ImGuiCol_ResizeGrip]          = ImVec4(0.26f, 0.56f, 0.98f, 0.20f);
        c[ImGuiCol_ResizeGripHovered]   = ImVec4(0.26f, 0.56f, 0.98f, 0.67f);
        c[ImGuiCol_ResizeGripActive]    = ImVec4(0.26f, 0.56f, 0.98f, 1.00f);
    }
}