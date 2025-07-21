#include "Application.h"
#include "algorithms/SortingVisualizer.h"
#include "algorithms/PathfindingVisualizer.h"
#include "algorithms/GraphVisualizer.h"
#include "algorithms/SearchVisualizer.h"
#include "algorithms/TreeVisualizer.h"
#include "audio/AudioManager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <cmath>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace AlgorithmVisualizer {

Application::Application() = default;

Application::~Application() = default;

bool Application::Initialize() {
    if (!InitializeGLFW()) {
        return false;
    }
    
    if (!InitializeImGui()) {
        return false;
    }
    
    // Initialize audio manager
    m_audioManager = std::make_shared<AlgorithmVisualizer::AudioManager>();
    if (!m_audioManager->Initialize()) {
        std::cerr << "Warning: Failed to initialize audio manager" << std::endl;
    }
    
    // Initialize visualizers with audio manager and performance callback
    m_sortingVisualizer = std::make_unique<SortingVisualizer>(m_audioManager.get());
    m_pathfindingVisualizer = std::make_unique<PathfindingVisualizer>(m_audioManager.get());
    m_graphVisualizer = std::make_unique<GraphVisualizer>(m_audioManager.get());
    m_searchVisualizer = std::make_unique<SearchVisualizer>(m_audioManager);
    m_treeVisualizer = std::make_unique<TreeVisualizer>(m_audioManager);
    
    // Set up performance reporting callbacks
    m_sortingVisualizer->SetPerformanceCallback(
        [this](const std::string& name, double time, int comparisons, int swaps) {
            this->ReportAlgorithmPerformance(name, time, comparisons, swaps);
        }
    );
    
    m_searchVisualizer->SetPerformanceCallback(
        [this](const std::string& name, double time, int comparisons, int swaps) {
            this->ReportAlgorithmPerformance(name, time, comparisons, swaps);
        }
    );
    
    m_treeVisualizer->SetPerformanceCallback(
        [this](const std::string& name, double time, int comparisons, int swaps) {
            this->ReportAlgorithmPerformance(name, time, comparisons, swaps);
        }
    );
    
    std::cout << "Application initialized successfully" << std::endl;
    return true;
}

void Application::Run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        
        Update();
        Render();
        
        glfwSwapBuffers(m_window);
    }
}

void Application::Shutdown() {
    CleanupImGui();
    CleanupGLFW();
}

void Application::Update() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Update audio manager
    if (m_audioManager) {
        m_audioManager->Update();
    }
    
    // Update current visualizer
    switch (m_currentVisualizer) {
        case VisualizationMode::Sorting:
            m_sortingVisualizer->Update();
            break;
        case VisualizationMode::Pathfinding:
            m_pathfindingVisualizer->Update();
            break;
        case VisualizationMode::Graph:
            m_graphVisualizer->Update();
            break;
        case VisualizationMode::Search:
            m_searchVisualizer->Update();
            break;
        case VisualizationMode::Tree:
            m_treeVisualizer->Update();
            break;
    }
}

void Application::Render() {
    // Update splash screen timing
    if (m_appState == AppState::Splash) {
        // Cap delta time to prevent splash screen from being skipped on first frame
        float deltaTime = std::min(ImGui::GetIO().DeltaTime, 1.0f / 30.0f); // Max 30 FPS delta
        
        // Skip the first few frames to let ImGui stabilize
        static int frameCount = 0;
        frameCount++;
        if (frameCount > 3) {
            m_splashAnimationTime += deltaTime;
        }
        

        
        // Allow skipping only after minimum display time (0.5 seconds)
        bool canSkip = m_splashAnimationTime >= 0.5f && (ImGui::IsKeyPressed(ImGuiKey_Space) || ImGui::IsMouseClicked(0));
        if (m_splashAnimationTime >= m_splashDuration || canSkip) {
            m_appState = AppState::Running;
        }
    }
    
    // Clear the screen with retro background
    if (m_appState == AppState::Splash) {
        glClearColor(0.02f, 0.02f, 0.08f, 1.0f); // Deep retro blue-black
    } else {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render based on current state
    if (m_appState == AppState::Splash) {
        RenderSplashScreen();
    } else {
        RenderUI();
    }
    
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Handle multiple viewports
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void Application::RenderSplashScreen() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    
    // Create a centered window (this was working)
    float windowWidth = 900;
    float windowHeight = 700;
    ImVec2 windowPos = ImVec2((displaySize.x - windowWidth) * 0.5f, (displaySize.y - windowHeight) * 0.5f);
    
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    
    // Retro cyberpunk styling
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.15f, 1.0f)); // Dark blue-black
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan text
    
    if (ImGui::Begin("ALGO1 - Retro Algorithm Visualizer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        
        // Retro loading screen (starting simple and building up)
        
        float progress = m_splashAnimationTime / m_splashDuration;
        float pulse = (sin(m_splashAnimationTime * 4.0f) + 1.0f) * 0.5f; // Pulsing effect
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Clean retro title
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 0.5f + pulse * 0.5f)); // Pulsing cyan
        ImGui::Text("                    A L G O 1");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // Subtitle
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 1.0f, 0.7f + pulse * 0.3f)); // Pulsing magenta
        ImGui::Text("        >>> RETRO ALGORITHM VISUALIZER <<<");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Progress percentage
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f)); // Neon green
        ImGui::Text("        [ SYSTEM LOADING: %.0f%% ]", progress * 100.0f);
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        // ASCII progress bar using basic characters
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan
        std::string progressBar = "        [";
        int barLength = 40;
        int filledLength = static_cast<int>(progress * barLength);
        
        for (int i = 0; i < barLength; i++) {
            if (i < filledLength) {
                progressBar += "#";
            } else if (i == filledLength && pulse > 0.5f) {
                progressBar += "*"; // Blinking cursor
            } else {
                progressBar += ".";
            }
        }
        progressBar += "]";
        ImGui::Text("%s", progressBar.c_str());
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Status messages
        std::vector<std::string> statusMessages = {
            ">>> INITIALIZING QUANTUM PROCESSORS...",
            ">>> LOADING ALGORITHM MATRICES...",
            ">>> CALIBRATING NEON DISPLAYS...", 
            ">>> WARMING UP CATHODE TUBES...",
            ">>> SYSTEM READY FOR VISUALIZATION <<<"
        };
        
        int messageIndex = std::min(static_cast<int>(progress * statusMessages.size()), 
                                  static_cast<int>(statusMessages.size() - 1));
        std::string currentMessage = statusMessages[messageIndex];
        
        // Add blinking cursor
        if (pulse > 0.5f && progress < 0.98f) {
            currentMessage += " |";
        }
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 0.8f + pulse * 0.2f)); // Pulsing yellow
        ImGui::Text("        %s", currentMessage.c_str());
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Skip instruction
        if (m_splashAnimationTime > 0.5f) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 0.5f + pulse * 0.3f)); // Pulsing gray
            ImGui::Text("        [ PRESS SPACE OR CLICK TO SKIP ]");
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Developer credits
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 1.0f, 0.7f)); // Soft blue
        ImGui::Text("    < CREATED BY GARRETT FLOWERS & SETH DELONEY > 2025 >");
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
    ImGui::PopStyleColor(2); // Pop the 2 colors we pushed
}

void Application::RenderUI() {
    RenderMenuBar();
    
    // Show notification if there's recent performance data AND notifications are enabled
    if (!m_performanceHistory.empty() && m_performanceNotificationsEnabled) {
        const auto& latest = m_performanceHistory.back();
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.3f, 0.1f, 0.9f));
        
        // Position at very bottom left corner
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 window_pos = ImVec2(5, io.DisplaySize.y - 60); // Very bottom left with small margin
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        
        if (ImGui::Begin("Performance Alert", nullptr, 
                        ImGuiWindowFlags_NoTitleBar | 
                        ImGuiWindowFlags_NoResize | 
                        ImGuiWindowFlags_AlwaysAutoResize | 
                        ImGuiWindowFlags_NoMove | 
                        ImGuiWindowFlags_NoCollapse)) {
            
            ImGui::Text("Latest: %s - %.2fms (%d comparisons)", 
                       latest.name.c_str(), latest.executionTime, latest.comparisons);
            
            if (ImGui::Button("View Analysis")) {
                m_showPerformanceAnalysis = true;
                m_performanceNotificationsEnabled = true; // Ensure notifications stay enabled when user wants analysis
            }
            ImGui::SameLine();
            if (ImGui::Button("Export Data")) {
                m_showExportDialog = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Dismiss")) {
                // Clear notification by clearing the performance history
                // In a real app, you'd have a separate notification system
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }
    
    RenderMainContent();
    
    // Render theme selector window
    if (m_showThemeSelector) {
        RenderThemeSelector();
    }
    
    // Render info windows
    if (m_showAlgorithmInfo) {
        RenderAlgorithmInfo();
    }
    
    if (m_showLicenses) {
        RenderLicenses();
    }
    
    if (m_showDevelopers) {
        RenderDevelopers();
    }
    
    if (m_showComparison) {
        RenderComparison();
    }
    
    if (m_showPerformanceAnalysis) {
        RenderPerformanceAnalysis();
    }
    
    if (m_showExportDialog) {
        RenderExportDialog();
    }
}

void Application::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Visualizations")) {
            if (ImGui::MenuItem("Sorting Algorithms", nullptr, m_currentVisualizer == VisualizationMode::Sorting)) {
                m_currentVisualizer = VisualizationMode::Sorting;
            }
            if (ImGui::MenuItem("Pathfinding", nullptr, m_currentVisualizer == VisualizationMode::Pathfinding)) {
                m_currentVisualizer = VisualizationMode::Pathfinding;
            }
            if (ImGui::MenuItem("Graph Algorithms", nullptr, m_currentVisualizer == VisualizationMode::Graph)) {
                m_currentVisualizer = VisualizationMode::Graph;
            }
            if (ImGui::MenuItem("Search Algorithms", nullptr, m_currentVisualizer == VisualizationMode::Search)) {
                m_currentVisualizer = VisualizationMode::Search;
            }
            if (ImGui::MenuItem("Tree Algorithms", nullptr, m_currentVisualizer == VisualizationMode::Tree)) {
                m_currentVisualizer = VisualizationMode::Tree;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Themes")) {
            if (ImGui::MenuItem("Dark", nullptr, m_currentTheme == Theme::Dark)) {
                m_currentTheme = Theme::Dark;
                ApplyTheme(m_currentTheme);
            }
            if (ImGui::MenuItem("Light", nullptr, m_currentTheme == Theme::Light)) {
                m_currentTheme = Theme::Light;
                ApplyTheme(m_currentTheme);
            }
            if (ImGui::MenuItem("Cyber", nullptr, m_currentTheme == Theme::Cyberpunk)) {
                m_currentTheme = Theme::Cyberpunk;
                ApplyTheme(m_currentTheme);
            }
            if (ImGui::MenuItem("Ocean", nullptr, m_currentTheme == Theme::Ocean)) {
                m_currentTheme = Theme::Ocean;
                ApplyTheme(m_currentTheme);
            }
            if (ImGui::MenuItem("Forest", nullptr, m_currentTheme == Theme::Forest)) {
                m_currentTheme = Theme::Forest;
                ApplyTheme(m_currentTheme);
            }
            if (ImGui::MenuItem("Sunset", nullptr, m_currentTheme == Theme::Sunset)) {
                m_currentTheme = Theme::Sunset;
                ApplyTheme(m_currentTheme);
            }
            ImGui::Separator();
            ImGui::MenuItem("More Themes...", nullptr, &m_showThemeSelector);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("About Developers", nullptr, &m_showDevelopers);
            ImGui::Separator();
            if (ImGui::MenuItem("Algorithm Info")) {
                m_showAlgorithmInfo = true;
            }
            if (ImGui::MenuItem("Licenses & Attribution")) {
                m_showLicenses = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Algorithm Comparison", nullptr, &m_showComparison);
            ImGui::Separator();
            ImGui::MenuItem("Performance Analysis", nullptr, &m_performanceNotificationsEnabled);
            ImGui::MenuItem("Export Results", nullptr, &m_showExportDialog);
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
    
    // Render theme selector window
    if (m_showThemeSelector) {
        RenderThemeSelector();
    }
}

void Application::RenderMainContent() {
    // Create a fullscreen window for the main content
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (viewport) {
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + ImGui::GetFrameHeight()));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - ImGui::GetFrameHeight()));
        // ImGui::SetNextWindowViewport(viewport->ID);
    }
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    if (ImGui::Begin("MainContent", nullptr, window_flags)) {
        switch (m_currentVisualizer) {
            case VisualizationMode::Sorting:
                m_sortingVisualizer->Render();
                break;
            case VisualizationMode::Pathfinding:
                m_pathfindingVisualizer->Render();
                break;
            case VisualizationMode::Graph:
                m_graphVisualizer->Render();
                break;
            case VisualizationMode::Search:
                m_searchVisualizer->Render();
                break;
            case VisualizationMode::Tree:
                m_treeVisualizer->Render();
                break;
        }
    }
    ImGui::End();
}

bool Application::InitializeGLFW() {
    if (!glfwInit()) {
        fmt::print("Failed to initialize GLFW\n");
        return false;
    }
    
    // GLFW configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), nullptr, nullptr);
    if (!m_window) {
        fmt::print("Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync
    
    return true;
}

bool Application::InitializeImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Increase font scale for better readability
    io.FontGlobalScale = 1.2f;

    SetupImGuiStyle();
    ApplyTheme(m_currentTheme);
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(m_window, true)) {
        fmt::print("Failed to initialize ImGui GLFW backend\n");
        return false;
    }
    
    const char* glsl_version = "#version 330";
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        fmt::print("Failed to initialize ImGui OpenGL3 backend\n");
        return false;
    }
    
    return true;
}

void Application::SetupImGuiStyle() {
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    
    // Apply additional styling
    style.WindowRounding = 5.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;
    
    // Modern spacing and padding
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.CellPadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
    
    // Modern borders and thickness
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    
    // Enhanced visual hierarchy
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    
    // Modern alpha values for better depth
    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6f;
}

void Application::CleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void Application::CleanupGLFW() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Application::ApplyTheme(Theme theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    switch (theme) {
        case Theme::Dark:
            ImGui::StyleColorsDark();
            break;
            
        case Theme::Light:
            ImGui::StyleColorsLight();
            break;
            
        case Theme::Cyberpunk:
            // Cyberpunk neon theme
            colors[ImGuiCol_Text]                   = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
            colors[ImGuiCol_Border]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.50f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
            colors[ImGuiCol_Button]                 = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
            colors[ImGuiCol_Header]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
            colors[ImGuiCol_Separator]              = ImVec4(0.00f, 0.50f, 0.50f, 0.33f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.00f, 0.50f, 0.50f, 0.47f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(0.00f, 0.70f, 0.67f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            break;
            
        case Theme::Ocean:
            // Ocean blue theme
            colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
            colors[ImGuiCol_Border]                 = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
            colors[ImGuiCol_CheckMark]              = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
            colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
            break;
            
        case Theme::Forest:
            // Forest green theme
            colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.12f, 0.06f, 0.94f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.08f, 0.16f, 0.08f, 1.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.05f, 0.10f, 0.05f, 0.94f);
            colors[ImGuiCol_Border]                 = ImVec4(0.25f, 0.50f, 0.25f, 0.50f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.15f, 0.30f, 0.15f, 0.54f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.20f, 0.40f, 0.20f, 0.40f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.25f, 0.50f, 0.25f, 0.67f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.20f, 0.10f, 1.00f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.15f, 0.30f, 0.15f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.05f, 0.10f, 0.05f, 1.00f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.10f, 0.20f, 0.10f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.10f, 0.05f, 0.53f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.25f, 0.50f, 0.25f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.30f, 0.60f, 0.30f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.35f, 0.70f, 0.35f, 1.00f);
            colors[ImGuiCol_CheckMark]              = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.30f, 0.60f, 0.30f, 1.00f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
            colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.40f, 0.20f, 0.40f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.25f, 0.50f, 0.25f, 1.00f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.30f, 0.60f, 0.30f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.25f, 0.50f, 0.25f, 0.31f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.30f, 0.60f, 0.30f, 0.80f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(0.35f, 0.70f, 0.35f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.25f, 0.50f, 0.25f, 0.50f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.30f, 0.60f, 0.30f, 0.78f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(0.35f, 0.70f, 0.35f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(0.25f, 0.50f, 0.25f, 0.25f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.30f, 0.60f, 0.30f, 0.67f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.35f, 0.70f, 0.35f, 0.95f);
            break;
            
        case Theme::Sunset:
            // Sunset orange/purple theme
            colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.90f, 0.85f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.45f, 0.40f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.06f, 0.12f, 0.94f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.16f, 0.08f, 0.16f, 1.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.10f, 0.05f, 0.10f, 0.94f);
            colors[ImGuiCol_Border]                 = ImVec4(0.80f, 0.40f, 0.20f, 0.50f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.30f, 0.15f, 0.30f, 0.54f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.20f, 0.40f, 0.40f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.50f, 0.25f, 0.50f, 0.67f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.20f, 0.10f, 0.20f, 1.00f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.30f, 0.15f, 0.30f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.10f, 0.05f, 0.10f, 1.00f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.20f, 0.10f, 0.20f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.10f, 0.05f, 0.10f, 0.53f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.80f, 0.40f, 0.20f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.90f, 0.50f, 0.30f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(1.00f, 0.60f, 0.40f, 1.00f);
            colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 0.70f, 0.30f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.80f, 0.40f, 0.20f, 1.00f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.00f, 0.60f, 0.40f, 1.00f);
            colors[ImGuiCol_Button]                 = ImVec4(0.40f, 0.20f, 0.40f, 0.40f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.80f, 0.40f, 0.20f, 1.00f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(1.00f, 0.60f, 0.40f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.80f, 0.40f, 0.20f, 0.31f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.90f, 0.50f, 0.30f, 0.80f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 0.60f, 0.40f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.80f, 0.40f, 0.20f, 0.50f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.90f, 0.50f, 0.30f, 0.78f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(1.00f, 0.60f, 0.40f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.40f, 0.20f, 0.25f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.90f, 0.50f, 0.30f, 0.67f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 0.60f, 0.40f, 0.95f);
            break;
    }
    
    // Apply additional styling
    style.WindowRounding = 5.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;
    
    // Modern spacing and padding
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.CellPadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
    
    // Modern borders and thickness
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    
    // Enhanced visual hierarchy
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    
    // Modern alpha values for better depth
    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6f;
}

void Application::RenderThemeSelector() {
    if (ImGui::Begin("Theme Selector", &m_showThemeSelector)) {
        ImGui::Text("Choose a theme for the visualizer:");
        ImGui::Separator();
        
        const char* themeNames[] = { "Dark", "Light", "Cyber", "Ocean", "Forest", "Sunset" };
        int currentThemeIndex = static_cast<int>(m_currentTheme);
        
        if (ImGui::Combo("Theme", &currentThemeIndex, themeNames, IM_ARRAYSIZE(themeNames))) {
            m_currentTheme = static_cast<Theme>(currentThemeIndex);
            ApplyTheme(m_currentTheme);
        }
        
        ImGui::Spacing();
        
        // Theme previews with colored rectangles
        ImGui::Text("Theme Previews:");
        ImVec2 colorSize(50, 30);
        
        if (ImGui::Button("Dark##preview", colorSize)) {
            m_currentTheme = Theme::Dark;
            ApplyTheme(m_currentTheme);
        }
        ImGui::SameLine();
        
        if (ImGui::Button("Light##preview", colorSize)) {
            m_currentTheme = Theme::Light;
            ApplyTheme(m_currentTheme);
        }
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 1.0f, 0.4f));
        if (ImGui::Button("Cyber##preview", colorSize)) {
            m_currentTheme = Theme::Cyberpunk;
            ApplyTheme(m_currentTheme);
        }
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.56f, 1.0f, 0.4f));
        if (ImGui::Button("Ocean##preview", colorSize)) {
            m_currentTheme = Theme::Ocean;
            ApplyTheme(m_currentTheme);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.50f, 0.25f, 0.4f));
        if (ImGui::Button("Forest##preview", colorSize)) {
            m_currentTheme = Theme::Forest;
            ApplyTheme(m_currentTheme);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.40f, 0.20f, 0.4f));
        if (ImGui::Button("Sunset##preview", colorSize)) {
            m_currentTheme = Theme::Sunset;
            ApplyTheme(m_currentTheme);
        }
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        
        if (ImGui::Button("Close")) {
            m_showThemeSelector = false;
        }
    }
    ImGui::End();
}

void Application::RenderAlgorithmInfo() {
    if (ImGui::Begin("Algorithm Information", &m_showAlgorithmInfo, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Algorithm Visualizer - Educational Tool");
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("Sorting Algorithms", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("Bubble Sort - O(n²) - Simple comparison-based algorithm");
            ImGui::BulletText("Selection Sort - O(n²) - Finds minimum and swaps");
            ImGui::BulletText("Insertion Sort - O(n²) - Builds sorted array one item at a time");
            ImGui::BulletText("Merge Sort - O(n log n) - Divide and conquer approach");
            ImGui::BulletText("Quick Sort - O(n log n) average - Partition-based sorting");
            ImGui::BulletText("Heap Sort - O(n log n) - Uses binary heap data structure");
            ImGui::BulletText("Tournament Sort - O(n log n) - Tournament tree approach");
            ImGui::BulletText("Intro Sort - O(n log n) - Hybrid of quicksort and heapsort");
            ImGui::BulletText("Patience Sort - O(n log n) - Based on patience card game");
        }
        
        if (ImGui::CollapsingHeader("Search Algorithms")) {
            ImGui::BulletText("Linear Search - O(n) - Sequential search through array");
            ImGui::BulletText("Binary Search - O(log n) - Search in sorted array by halving");
            ImGui::BulletText("Interpolation Search - O(log log n) - Estimates position");
            ImGui::BulletText("Exponential Search - O(log n) - Exponential jumps then binary");
            ImGui::BulletText("Jump Search - O(√n) - Fixed-size jumps through array");
        }
        
        if (ImGui::CollapsingHeader("Pathfinding Algorithms")) {
            ImGui::BulletText("A* - Optimal pathfinding with heuristic");
            ImGui::BulletText("Dijkstra - Shortest path, guaranteed optimal");
            ImGui::BulletText("Breadth-First Search - Unweighted shortest path");
            ImGui::BulletText("Depth-First Search - Explores as far as possible");
        }
        
        if (ImGui::CollapsingHeader("Graph Algorithms")) {
            ImGui::BulletText("Kruskal's MST - O(E log E) - Minimum spanning tree using union-find");
            ImGui::BulletText("Prim's MST - O(V²) or O(E log V) - Greedy minimum spanning tree");
            ImGui::BulletText("Topological Sort - O(V + E) - Linear ordering of vertices");
            ImGui::BulletText("Strongly Connected Components - O(V + E) - Find SCCs using DFS");
        }
        
        if (ImGui::CollapsingHeader("Tree Algorithms")) {
            ImGui::BulletText("Binary Search Tree - Ordered binary tree structure");
            ImGui::BulletText("AVL Tree - Self-balancing binary search tree");
            ImGui::BulletText("Red-Black Tree - Balanced binary search tree");
            ImGui::BulletText("Min/Max Heap - Complete binary tree with heap property");
        }
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                          "Use this tool to understand algorithm behavior, complexity, and performance.");
    }
    ImGui::End();
}

void Application::RenderLicenses() {
    if (ImGui::Begin("Licenses & Attribution", &m_showLicenses)) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Third-Party Libraries & Attribution");
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("Dear ImGui", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("MIT License");
            ImGui::Text("Copyright (c) 2014-2024 Omar Cornut");
            ImGui::Spacing();
            ImGui::TextWrapped("Permission is hereby granted, free of charge, to any person obtaining a copy "
                              "of this software and associated documentation files (the \"Software\"), to deal "
                              "in the Software without restriction, including without limitation the rights "
                              "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                              "copies of the Software, and to permit persons to whom the Software is "
                              "furnished to do so, subject to the following conditions.");
            ImGui::Spacing();
            ImGui::Text("Features: docking-experimental, glfw-binding, opengl3-binding");
            ImGui::Text("Repository: https://github.com/ocornut/imgui");
        }
        
        if (ImGui::CollapsingHeader("GLFW")) {
            ImGui::Text("zlib/libpng License");
            ImGui::Text("Copyright (c) 2002-2006 Marcus Geelnard");
            ImGui::Text("Copyright (c) 2006-2019 Camilla Löwy");
            ImGui::Spacing();
            ImGui::TextWrapped("This software is provided 'as-is', without any express or implied warranty. "
                              "In no event will the authors be held liable for any damages arising from the "
                              "use of this software. Permission is granted to anyone to use this software "
                              "for any purpose, including commercial applications, and to alter it and "
                              "redistribute it freely.");
            ImGui::Spacing();
            ImGui::Text("Website: https://www.glfw.org/");
            ImGui::Text("Repository: https://github.com/glfw/glfw");
        }
        
        if (ImGui::CollapsingHeader("OpenGL")) {
            ImGui::Text("Khronos Group OpenGL API");
            ImGui::TextWrapped("OpenGL and the oval logo are trademarks or registered trademarks of "
                              "Silicon Graphics, Inc. in the United States and/or other countries worldwide.");
            ImGui::Spacing();
            ImGui::Text("Website: https://www.opengl.org/");
        }
        
        if (ImGui::CollapsingHeader("OpenAL-Soft")) {
            ImGui::Text("LGPL v2.1 License");
            ImGui::Text("Copyright (c) 1999-2000 Loki Software");
            ImGui::Text("Copyright (c) 2005+ OpenAL-Soft contributors");
            ImGui::Spacing();
            ImGui::TextWrapped("OpenAL-Soft is free software: you can redistribute it and/or modify it "
                              "under the terms of the GNU Lesser General Public License as published by "
                              "the Free Software Foundation, either version 2.1 of the License, or "
                              "(at your option) any later version.");
            ImGui::Spacing();
            ImGui::Text("Website: https://openal-soft.org/");
            ImGui::Text("Repository: https://github.com/kcat/openal-soft");
        }
        
        if (ImGui::CollapsingHeader("ImPlot")) {
            ImGui::Text("MIT License");
            ImGui::Text("Copyright (c) 2020 Evan Pezent");
            ImGui::Spacing();
            ImGui::TextWrapped("Permission is hereby granted, free of charge, to any person obtaining a copy "
                              "of this software and associated documentation files (the \"Software\"), to deal "
                              "in the Software without restriction, including without limitation the rights "
                              "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                              "copies of the Software, and to permit persons to whom the Software is "
                              "furnished to do so, subject to the following conditions.");
            ImGui::Spacing();
            ImGui::Text("Repository: https://github.com/epezent/implot");
        }
        
        if (ImGui::CollapsingHeader("fmt Library")) {
            ImGui::Text("MIT License");
            ImGui::Text("Copyright (c) 2012 - present, Victor Zverovich");
            ImGui::Spacing();
            ImGui::TextWrapped("Permission is hereby granted, free of charge, to any person obtaining a copy "
                              "of this software and associated documentation files (the \"Software\"), to deal "
                              "in the Software without restriction, including without limitation the rights "
                              "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                              "copies of the Software, and to permit persons to whom the Software is "
                              "furnished to do so, subject to the following conditions.");
            ImGui::Spacing();
            ImGui::Text("Website: https://fmt.dev/");
            ImGui::Text("Repository: https://github.com/fmtlib/fmt");
        }
        
        if (ImGui::CollapsingHeader("vcpkg Package Manager")) {
            ImGui::Text("MIT License");
            ImGui::Text("Copyright (c) Microsoft Corporation");
            ImGui::Spacing();
            ImGui::TextWrapped("C++ package manager for acquiring and managing libraries. Used to manage "
                              "dependencies: fmt, imgui, glfw3, implot, and openal-soft.");
            ImGui::Spacing();
            ImGui::Text("Website: https://vcpkg.io/");
            ImGui::Text("Repository: https://github.com/microsoft/vcpkg");
        }
        
        if (ImGui::CollapsingHeader("C++ Standard Library")) {
            ImGui::Text("Implementation-specific License");
            ImGui::Spacing();
            ImGui::TextWrapped("This application uses C++20 standard library features including: "
                              "<memory>, <string>, <vector>, <functional>, <chrono>, <fstream>, "
                              "<iostream>, <cmath>, <algorithm>, and <cstdlib>.");
            ImGui::Spacing();
            ImGui::TextWrapped("Standard library implementation provided by the system compiler "
                              "(Clang/LLVM on macOS, GCC on Linux, MSVC on Windows).");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Legal Notice");
        ImGui::TextWrapped("This application is built using the above open-source libraries and frameworks. "
                          "All third-party components retain their original licenses and copyrights. "
                          "This software is provided for educational purposes.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                          "For complete license texts, please refer to the respective project repositories.");
    }
    ImGui::End();
}

void Application::RenderDevelopers() {
    if (ImGui::Begin("About Developers", &m_showDevelopers, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Algo1 - Retro Algorithm Visualizer");
        ImGui::Text("Version 2.0 - Educational Algorithm Visualization Tool");
        ImGui::Separator();
        
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Developed by:");
        ImGui::Spacing();
        
        ImGui::BulletText("Garrett Flowers");
        ImGui::Indent();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Lead Developer & Algorithm Implementation");
        ImGui::Unindent();
        
        ImGui::BulletText("Seth Deloney");
        ImGui::Indent();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Co-Developer & System Architecture");
        ImGui::Unindent();
        
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.3f, 1.0f), "Features:");
        ImGui::BulletText("Interactive algorithm visualization");
        ImGui::BulletText("Real-time performance analysis");
        ImGui::BulletText("Audio feedback system");
        ImGui::BulletText("Multiple beautiful themes");
        ImGui::BulletText("Educational step-by-step execution");
        
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Built with Modern C++20");
        ImGui::Text("Graphics: OpenGL + Dear ImGui");
        ImGui::Text("Audio: OpenAL");
        ImGui::Text("Build System: CMake + vcpkg");
        
        ImGui::Spacing();
        
        if (ImGui::Button("Close")) {
            m_showDevelopers = false;
        }
    }
    ImGui::End();
}

void Application::RenderComparison() {
    if (ImGui::Begin("Algorithm Comparison & Analysis", &m_showComparison)) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Compare Algorithm Performance");
        ImGui::Separator();
        
        // Modern tabs for different comparison types
        if (ImGui::BeginTabBar("ComparisonTabs", ImGuiTabBarFlags_Reorderable)) {
            
            if (ImGui::BeginTabItem("Sorting Comparison")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Time Complexity Analysis");
                
                // Modern table with alternating row colors
                if (ImGui::BeginTable("SortingTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupColumn("Algorithm", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                    ImGui::TableSetupColumn("Best Case", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("Average Case", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("Worst Case", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableHeadersRow();
                    
                    // Bubble Sort
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Bubble Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "O(n²)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "O(n²)");
                    
                    // Quick Sort
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Quick Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "O(n²)");
                    
                    // Merge Sort
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Merge Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    
                    // Heap Sort
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Heap Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(n log n)");
                    
                    ImGui::EndTable();
                }
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Space Complexity & Stability");
                
                if (ImGui::BeginTable("SortingProperties", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Algorithm");
                    ImGui::TableSetupColumn("Space");
                    ImGui::TableSetupColumn("Stable");
                    ImGui::TableHeadersRow();
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Bubble Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(1)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Yes");
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Quick Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "O(log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No");
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Merge Sort");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "O(n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Yes");
                    
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Search Comparison")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Search Algorithm Analysis");
                
                if (ImGui::BeginTable("SearchTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Algorithm");
                    ImGui::TableSetupColumn("Time Complexity");
                    ImGui::TableSetupColumn("Space");
                    ImGui::TableSetupColumn("Requirement");
                    ImGui::TableHeadersRow();
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Linear Search");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "O(n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(1)");
                    ImGui::TableNextColumn(); ImGui::Text("None");
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Binary Search");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(log n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(1)");
                    ImGui::TableNextColumn(); ImGui::Text("Sorted Array");
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("Jump Search");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "O(√n)");
                    ImGui::TableNextColumn(); ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "O(1)");
                    ImGui::TableNextColumn(); ImGui::Text("Sorted Array");
                    
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Performance Metrics")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Real-time Performance");
                
                if (m_performanceHistory.empty()) {
                    ImGui::Text("No performance data available yet.");
                    ImGui::Text("Run some algorithms to see real performance metrics!");
                } else {
                    ImGui::Text("Recent Algorithm Performance:");
                    ImGui::Separator();
                    
                    // Show real performance data from recent runs
                    for (size_t i = std::max(0, (int)m_performanceHistory.size() - 5); i < m_performanceHistory.size(); i++) {
                        const auto& perf = m_performanceHistory[i];
                        
                        ImGui::Text("%s", perf.name.c_str());
                        
                        // Calculate performance rating based on execution time
                        float performanceRating = 0.0f;
                        if (perf.executionTime < 50.0) {
                            performanceRating = 0.9f; // Excellent
                        } else if (perf.executionTime < 500.0) {
                            performanceRating = 0.6f; // Good
                        } else if (perf.executionTime < 2000.0) {
                            performanceRating = 0.4f; // Fair
                        } else {
                            performanceRating = 0.2f; // Slow
                        }
                        
                        ImGui::ProgressBar(performanceRating, ImVec2(-1, 0), 
                                         fmt::format("{:.1f}ms, {} ops", perf.executionTime, perf.comparisons).c_str());
                    }
                    
                    ImGui::Spacing();
                    ImGui::Text("Average Performance:");
                    
                    // Calculate averages from real data
                    double avgTime = 0.0;
                    int avgComparisons = 0;
                    int avgMemory = 0;
                    
                    for (const auto& perf : m_performanceHistory) {
                        avgTime += perf.executionTime;
                        avgComparisons += perf.comparisons;
                        avgMemory += perf.memoryUsage;
                    }
                    
                    size_t count = m_performanceHistory.size();
                    avgTime /= count;
                    avgComparisons /= count;
                    avgMemory /= count;
                    
                    ImGui::Text("Average Time: %.1fms", avgTime);
                    ImGui::Text("Average Comparisons: %d", avgComparisons);
                    ImGui::Text("Average Memory: %dKB", avgMemory);
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                          "Use this comparison tool to understand algorithm trade-offs and choose the best algorithm for your use case.");
    }
    ImGui::End();
}

void Application::RenderPerformanceAnalysis() {
    if (ImGui::Begin("Performance Analysis", &m_showPerformanceAnalysis, ImGuiWindowFlags_None)) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Real-time Algorithm Performance Monitor");
        ImGui::Separator();
        
        if (ImGui::BeginTabBar("PerformanceTabs##perf_analysis")) {
            
            if (ImGui::BeginTabItem("Live Metrics##perf_live")) {
                ImGui::Spacing();
                
                // System Performance
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "System Performance");
                ImGui::Separator();
                
                // Frame rate info
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                           1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                
                float framerate = ImGui::GetIO().Framerate;
                ImGui::Text("Frame Rate: ");
                ImGui::SameLine();
                if (framerate > 45.0f) {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%.1f FPS (Good)", framerate);
                } else if (framerate > 30.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "%.1f FPS (Fair)", framerate);
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%.1f FPS (Poor)", framerate);
                }
                
                ImGui::Spacing();
                
                // Memory usage (calculated from current algorithms)
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Memory Usage");
                ImGui::Separator();
                
                // Calculate total memory from performance history
                int totalMemoryKB = 0;
                for (const auto& perf : m_performanceHistory) {
                    totalMemoryKB += perf.memoryUsage;
                }
                float memoryUsagePercent = std::min(1.0f, totalMemoryKB / 1000.0f); // Scale to percentage
                
                ImGui::Text("Algorithm Data: ");
                ImGui::ProgressBar(memoryUsagePercent, ImVec2(-1, 0), fmt::format("{}KB", totalMemoryKB).c_str());
                
                ImGui::Text("Graphics Memory: ");
                // Estimate graphics memory based on UI elements and arrays
                int graphicsMemoryKB = 500 + (m_performanceHistory.size() * 10); // Base + history
                float graphicsPercent = std::min(1.0f, graphicsMemoryKB / 10000.0f);
                ImGui::PushID("graphics_memory_bar");
                ImGui::ProgressBar(graphicsPercent, ImVec2(-1, 0), fmt::format("{}KB", graphicsMemoryKB).c_str());
                ImGui::PopID();
                
                ImGui::Spacing();
                
                // Current algorithm stats
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Current Algorithm Status");
                ImGui::Separator();
                
                const char* currentViz = "None";
                std::string algorithmName = "None";
                std::string arraySize = "N/A";
                std::string animationSpeed = "N/A";
                
                switch (m_currentVisualizer) {
                    case VisualizationMode::Sorting:
                        currentViz = "Sorting Algorithms";
                        if (m_sortingVisualizer) {
                            algorithmName = m_sortingVisualizer->GetAlgorithmName(m_sortingVisualizer->GetAlgorithm());
                            arraySize = std::to_string(m_sortingVisualizer->GetArray().size());
                            animationSpeed = (m_sortingVisualizer->GetState() == SortingVisualizer::AnimationState::Running) ? "Running" : "Stopped";
                        }
                        break;
                    case VisualizationMode::Search:
                        currentViz = "Search Algorithms";
                        if (m_searchVisualizer) {
                            // Get search algorithm info here
                            arraySize = "1000"; // Default for now
                            animationSpeed = "Normal";
                        }
                        break;
                    case VisualizationMode::Pathfinding:
                        currentViz = "Pathfinding";
                        arraySize = "Grid";
                        animationSpeed = "Normal";
                        break;
                    case VisualizationMode::Graph:
                        currentViz = "Graph Algorithms";
                        arraySize = "Nodes/Edges";
                        animationSpeed = "Normal";
                        break;
                    case VisualizationMode::Tree:
                        currentViz = "Tree Algorithms";
                        if (m_treeVisualizer) {
                            arraySize = "Dynamic";
                            animationSpeed = "Normal";
                        }
                        break;
                }
                
                ImGui::BulletText("Active Visualizer: %s", currentViz);
                ImGui::BulletText("Current Algorithm: %s", algorithmName.c_str());
                ImGui::BulletText("Data Size: %s elements", arraySize.c_str());
                ImGui::BulletText("Status: %s", animationSpeed.c_str());
                ImGui::BulletText("Audio Feedback: Enabled");
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Algorithm Benchmarks##perf_benchmarks")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Recent Algorithm Execution Results");
                ImGui::Separator();
                
                if (m_performanceHistory.empty()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No algorithm runs recorded yet.");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Run some algorithms to see performance data here!");
                } else {
                    if (ImGui::BeginTable("BenchmarkTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable)) {
                        ImGui::TableSetupColumn("Algorithm", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                        ImGui::TableSetupColumn("Time (ms)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableSetupColumn("Comparisons", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                        ImGui::TableSetupColumn("Swaps", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableSetupColumn("Memory (KB)", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                        ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableHeadersRow();
                        
                        // Show last 10 performance records
                        size_t startIdx = std::max(0, (int)m_performanceHistory.size() - 10);
                        for (size_t i = startIdx; i < m_performanceHistory.size(); i++) {
                            const auto& perf = m_performanceHistory[i];
                            
                            ImGui::PushID((int)i); // Fix ID conflicts by giving each row a unique ID
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn(); 
                            ImGui::Text("%s", perf.name.c_str());
                            
                            ImGui::TableNextColumn(); 
                            if (perf.executionTime < 5.0) {
                                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%.2f", perf.executionTime);
                            } else if (perf.executionTime < 50.0) {
                                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "%.2f", perf.executionTime);
                            } else {
                                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%.2f", perf.executionTime);
                            }
                            
                            ImGui::TableNextColumn(); 
                            ImGui::Text("%d", perf.comparisons);
                            
                            ImGui::TableNextColumn(); 
                            ImGui::Text("%d", perf.swaps);
                            
                            ImGui::TableNextColumn(); 
                            ImGui::Text("%d", perf.memoryUsage);
                            
                            ImGui::TableNextColumn(); 
                            ImGui::Text("%s", perf.timestamp.c_str());
                            ImGui::PopID(); // End unique ID scope
                        }
                        
                        ImGui::EndTable();
                    }
                    
                    ImGui::Spacing();
                    if (ImGui::Button("Clear History##perf_clear")) {
                        m_performanceHistory.clear();
                    }
                }
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                                  "Performance data collected from actual algorithm executions in this session.");
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Optimization Tips##perf_tips")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Performance Optimization Recommendations");
                ImGui::Separator();
                
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "General Tips:");
                ImGui::BulletText("Use Quick Sort or Merge Sort for large datasets");
                ImGui::BulletText("Binary Search requires sorted data but is extremely fast");
                ImGui::BulletText("Consider data characteristics when choosing algorithms");
                ImGui::BulletText("Tree structures excel at dynamic insertion/deletion");
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Memory Considerations:");
                ImGui::BulletText("Merge Sort uses O(n) extra space");
                ImGui::BulletText("Quick Sort is in-place but can be unstable");
                ImGui::BulletText("Heap Sort provides guaranteed O(n log n) time");
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "When to Use Each:");
                ImGui::BulletText("Small datasets (< 50): Insertion Sort");
                ImGui::BulletText("General purpose: Quick Sort or Intro Sort");
                ImGui::BulletText("Stability required: Merge Sort");
                ImGui::BulletText("Guaranteed performance: Heap Sort");
                ImGui::BulletText("Nearly sorted data: Insertion Sort");
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void Application::RenderExportDialog() {
    if (ImGui::Begin("Export Results", &m_showExportDialog, ImGuiWindowFlags_None)) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Export Algorithm Data & Results");
        ImGui::Separator();
        
        static int exportFormat = 0;
        static bool includeTimings = true;
        static bool includeComparisons = true;
        static bool includeMemoryStats = false;
        static bool includeSteps = false;
        static char filename[256] = "algorithm_results";
        
        ImGui::Text("Export Format:");
        ImGui::RadioButton("CSV (Comma Separated)##export_csv", &exportFormat, 0); ImGui::SameLine();
        ImGui::RadioButton("JSON##export_json", &exportFormat, 1); ImGui::SameLine();
        ImGui::RadioButton("XML##export_xml", &exportFormat, 2);
        
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Data to Include:");
        ImGui::Checkbox("Timing Information##export_timings", &includeTimings);
        ImGui::Checkbox("Comparison Counts##export_comparisons", &includeComparisons);
        ImGui::Checkbox("Memory Usage Stats##export_memory", &includeMemoryStats);
        ImGui::Checkbox("Step-by-step Details##export_steps", &includeSteps);
        
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Filename:");
        ImGui::InputText("##export_filename", filename, sizeof(filename));
        
        ImGui::Spacing();
        
        // Export preview
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Export Preview:");
        ImGui::BeginChild("ExportPreview##export_child", ImVec2(400, 150), true);
        
        if (exportFormat == 0) { // CSV
            ImGui::Text("algorithm,time_ms,comparisons,swaps,memory_kb,timestamp");
            if (includeTimings) ImGui::Text("quick_sort,2.3,9863,124,124,2024-01-15T10:30:00Z");
            if (includeComparisons) ImGui::Text("merge_sort,3.1,8123,2048,2048,2024-01-15T10:30:00Z");
            ImGui::Text("bubble_sort,127.8,498501,124,124,2024-01-15T10:30:00Z");
        } else if (exportFormat == 1) { // JSON
            ImGui::Text("{");
            ImGui::Text("  \"export_timestamp\": \"2024-01-15T10:30:00Z\",");
            ImGui::Text("  \"algorithms\": [");
            ImGui::Text("    {");
            ImGui::Text("      \"name\": \"quick_sort\",");
            if (includeTimings) ImGui::Text("      \"time_ms\": 2.3,");
            if (includeComparisons) ImGui::Text("      \"comparisons\": 9863");
            ImGui::Text("    }");
            ImGui::Text("  ]");
            ImGui::Text("}");
        } else { // XML
            ImGui::Text("<?xml version=\"1.0\"?>");
            ImGui::Text("<algorithm_results>");
            ImGui::Text("  <algorithm name=\"quick_sort\">");
            if (includeTimings) ImGui::Text("    <time_ms>2.3</time_ms>");
            if (includeComparisons) ImGui::Text("    <comparisons>9863</comparisons>");
            ImGui::Text("  </algorithm>");
            ImGui::Text("</algorithm_results>");
        }
        
        ImGui::EndChild();
        
        ImGui::Spacing();
        
        // Export buttons
        const char* extensions[] = {".csv", ".json", ".xml"};
        std::string fullFilename = std::string(filename) + extensions[exportFormat];
        
        if (ImGui::Button("Export Data##export_btn", ImVec2(120, 0))) {
            // Generate export content
            std::string content;
            
            if (exportFormat == 0) { // CSV
                content = "algorithm,time_ms,comparisons,swaps,memory_kb,timestamp\n";
                for (const auto& perf : m_performanceHistory) {
                    content += fmt::format("{},{:.2f},{},{},{},{}\n",
                                         perf.name, perf.executionTime, perf.comparisons, 
                                         perf.swaps, perf.memoryUsage, perf.timestamp);
                }
            } else if (exportFormat == 1) { // JSON
                content = "{\n  \"export_timestamp\": \"" + std::string(__DATE__) + " " + std::string(__TIME__) + "\",\n";
                content += "  \"total_algorithms\": " + std::to_string(m_performanceHistory.size()) + ",\n";
                content += "  \"algorithms\": [\n";
                for (size_t i = 0; i < m_performanceHistory.size(); i++) {
                    const auto& perf = m_performanceHistory[i];
                    content += "    {\n";
                    content += "      \"name\": \"" + perf.name + "\",\n";
                    if (includeTimings) content += fmt::format("      \"time_ms\": {:.2f},\n", perf.executionTime);
                    if (includeComparisons) content += fmt::format("      \"comparisons\": {},\n", perf.comparisons);
                    if (includeMemoryStats) content += fmt::format("      \"memory_kb\": {},\n", perf.memoryUsage);
                    content += "      \"timestamp\": \"" + perf.timestamp + "\"\n";
                    content += "    }";
                    if (i < m_performanceHistory.size() - 1) content += ",";
                    content += "\n";
                }
                content += "  ]\n}";
            } else { // XML
                content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
                content += "<algorithm_results>\n";
                content += "  <export_info>\n";
                content += "    <timestamp>" + std::string(__DATE__) + " " + std::string(__TIME__) + "</timestamp>\n";
                content += "    <total_algorithms>" + std::to_string(m_performanceHistory.size()) + "</total_algorithms>\n";
                content += "  </export_info>\n";
                for (const auto& perf : m_performanceHistory) {
                    content += "  <algorithm name=\"" + perf.name + "\">\n";
                    if (includeTimings) content += fmt::format("    <time_ms>{:.2f}</time_ms>\n", perf.executionTime);
                    if (includeComparisons) content += fmt::format("    <comparisons>{}</comparisons>\n", perf.comparisons);
                    if (includeMemoryStats) content += fmt::format("    <memory_kb>{}</memory_kb>\n", perf.memoryUsage);
                    content += "    <timestamp>" + perf.timestamp + "</timestamp>\n";
                    content += "  </algorithm>\n";
                }
                content += "</algorithm_results>";
            }
            
            // Save file
            std::string savedPath = SaveExportFile(content, fullFilename);
            if (!savedPath.empty()) {
                ImGui::OpenPopup("Export Complete");
            } else {
                ImGui::OpenPopup("Export Failed");
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel##export_cancel", ImVec2(80, 0))) {
            m_showExportDialog = false;
        }
        
        // Export complete popup
        if (ImGui::BeginPopupModal("Export Complete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Data exported successfully!");
            ImGui::Text("Records exported: %zu", m_performanceHistory.size());
            ImGui::Text("File saved to Desktop");
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                m_showExportDialog = false;
            }
            ImGui::EndPopup();
        }
        
        // Export failed popup
        if (ImGui::BeginPopupModal("Export Failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Failed to export data!");
            ImGui::Text("Please check file permissions.");
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void Application::RecordAlgorithmPerformance(const std::string& algorithmName, double time, int comparisons, int swaps) {
    AlgorithmPerformance perf;
    perf.name = algorithmName;
    perf.executionTime = time;
    perf.comparisons = comparisons;
    perf.swaps = swaps;
    
    // Calculate real memory usage based on current algorithm
    int memoryUsage = 0;
    switch (m_currentVisualizer) {
        case VisualizationMode::Sorting:
            if (m_sortingVisualizer) {
                // Array size * sizeof(int) + overhead for steps and colors
                int arraySize = static_cast<int>(m_sortingVisualizer->GetArray().size());
                memoryUsage = (arraySize * sizeof(int)) / 1024; // Convert to KB
                memoryUsage += arraySize * 2; // Additional overhead for step recording
            }
            break;
        case VisualizationMode::Search:
            memoryUsage = (1000 * sizeof(int)) / 1024; // Default search array size
            break;
        case VisualizationMode::Tree:
            memoryUsage = 50; // Tree nodes are dynamic, estimate based on typical usage
            break;
        default:
            memoryUsage = 10; // Minimal overhead for other algorithms
            break;
    }
    perf.memoryUsage = memoryUsage;
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    perf.timestamp = fmt::format("{:02d}:{:02d}:{:02d}", tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    m_performanceHistory.push_back(perf);
    
    // Keep only last 50 records to avoid memory bloat
    if (m_performanceHistory.size() > 50) {
        m_performanceHistory.erase(m_performanceHistory.begin());
    }
    
    std::cout << fmt::format("Performance recorded: {} completed in {:.2f}ms with {} comparisons, {}KB memory", 
                            algorithmName, time, comparisons, memoryUsage) << std::endl;
}

std::string Application::SaveExportFile(const std::string& content, const std::string& filename) {
    try {
        // Save to Desktop for easy access
        std::string homePath = std::getenv("HOME");
        std::string desktopPath = homePath + "/Desktop/";
        std::string fullPath = desktopPath + filename;
        
        // Write file
        std::ofstream file(fullPath);
        if (file.is_open()) {
            file << content;
            file.close();
            return fullPath;
        }
    } catch (const std::exception& e) {
        // If Desktop fails, try current directory
        try {
            std::ofstream file(filename);
            if (file.is_open()) {
                file << content;
                file.close();
                return filename;
            }
        } catch (...) {
            return "";
        }
    }
    return "";
}

void Application::ReportAlgorithmPerformance(const std::string& algorithmName, double time, int comparisons, int swaps) {
    RecordAlgorithmPerformance(algorithmName, time, comparisons, swaps);
    
    // Don't automatically show performance analysis - only show notification
    // User can click the button in notification to view analysis
}

} // namespace AlgorithmVisualizer 