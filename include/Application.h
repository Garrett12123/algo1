#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;

namespace AlgorithmVisualizer {

class AudioManager;
class SortingVisualizer;
class PathfindingVisualizer;
class GraphVisualizer;
class SearchVisualizer;
class TreeVisualizer;

class Application {
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

    // Performance tracking - called by visualizers
    void ReportAlgorithmPerformance(const std::string& algorithmName, double time, int comparisons, int swaps = 0);

private:
    void Update();
    void Render();
    void RenderUI();
    void RenderSplashScreen();
    void RenderMenuBar();
    void RenderMainContent();
    
    // Window management
    GLFWwindow* m_window = nullptr;
    int m_windowWidth = 1920;
    int m_windowHeight = 1080;
    std::string m_windowTitle = "Algo1 - Retro Algorithm Visualizer";
    
    // Application state
    enum class AppState {
        Splash,
        Running
    };
    
    enum class VisualizationMode {
        Sorting,
        Pathfinding,
        Graph,
        Search,
        Tree
    };
    
    enum class Theme {
        Dark,
        Light,
        Cyberpunk,
        Ocean,
        Forest,
        Sunset
    };
    
    VisualizationMode m_currentMode = VisualizationMode::Sorting;
    bool m_showDemoWindow = false;
    bool m_showAbout = false;
    
    // Core components
    std::shared_ptr<AudioManager> m_audioManager;
    
    // Visualizers
    std::unique_ptr<SortingVisualizer> m_sortingVisualizer;
    std::unique_ptr<PathfindingVisualizer> m_pathfindingVisualizer;
    std::unique_ptr<GraphVisualizer> m_graphVisualizer;
    std::unique_ptr<SearchVisualizer> m_searchVisualizer;
    std::unique_ptr<TreeVisualizer> m_treeVisualizer;
    
    // Initialization helpers
    bool InitializeGLFW();
    bool InitializeImGui();
    void SetupImGuiStyle();
    void ApplyTheme(Theme theme);
    void RenderThemeSelector();
    void RenderAlgorithmInfo();
    void RenderLicenses();
    void RenderDevelopers();
    void RenderComparison();
    void RenderPerformanceAnalysis();
    void RenderExportDialog();
    
    // Cleanup helpers
    void CleanupImGui();
    void CleanupGLFW();

    // State  
    AppState m_appState = AppState::Splash;
    VisualizationMode m_currentVisualizer = VisualizationMode::Sorting;
    Theme m_currentTheme = Theme::Dark;
    bool m_showThemeSelector = false;
    
    // Splash screen state
    float m_splashStartTime = 0.0f;
    float m_splashDuration = 5.0f;  // Longer duration to enjoy the retro effects
    float m_splashAnimationTime = 0.0f;
    bool m_showAlgorithmInfo = false;
    bool m_showLicenses = false;
    bool m_showDevelopers = false;
    bool m_showComparison = false;
    bool m_showPerformanceAnalysis = false;
    bool m_showExportDialog = false;
    bool m_performanceNotificationsEnabled = true; // Controls whether notifications appear after algorithm runs
    
    // Performance tracking
    struct AlgorithmPerformance {
        std::string name;
        double executionTime = 0.0;
        int comparisons = 0;
        int swaps = 0;
        int memoryUsage = 0;
        std::string timestamp;
    };
    
    std::vector<AlgorithmPerformance> m_performanceHistory;
    
    static constexpr const char* s_visualizerNames[] = {
        "Sorting Algorithms",
        "Pathfinding Algorithms", 
        "Graph Algorithms",
        "Search Algorithms",
        "Tree Algorithms"
    };

    void RecordAlgorithmPerformance(const std::string& algorithmName, double time, int comparisons, int swaps = 0);
    std::string SaveExportFile(const std::string& content, const std::string& filename);
};

} // namespace AlgorithmVisualizer 