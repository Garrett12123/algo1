#pragma once

#include <vector>
#include <queue>
#include <stack>
#include <unordered_set>
#include <functional>
#include <chrono>

namespace AlgorithmVisualizer {

class AudioManager;

struct GridCell {
    int x, y;
    enum class Type {
        Empty,
        Wall,
        Start,
        End,
        Path,
        Visited,
        Frontier
    } type = Type::Empty;
    
    float gCost = 0.0f;  // Distance from start
    float hCost = 0.0f;  // Heuristic distance to end
    float fCost = 0.0f;  // Total cost (g + h)
    GridCell* parent = nullptr;
    
    bool operator==(const GridCell& other) const {
        return x == other.x && y == other.y;
    }
};

struct GridCellHash {
    std::size_t operator()(const GridCell& cell) const {
        return std::hash<int>()(cell.x) ^ (std::hash<int>()(cell.y) << 1);
    }
};

class PathfindingVisualizer {
public:
    enum class Algorithm {
        AStar,
        Dijkstra,
        BreadthFirst,
        DepthFirst
    };
    
    enum class AnimationState {
        Stopped,
        Running,
        Paused,
        Completed
    };

public:
    PathfindingVisualizer(AudioManager* audioManager = nullptr);
    ~PathfindingVisualizer() = default;
    
    void Update();
    void Render();
    void RenderControls();
    void RenderGrid();
    void RenderStatistics();
    
    // Control methods
    void StartPathfinding();
    void PausePathfinding();
    void ResetGrid();
    void ClearPath();
    void StepForward();
    
    // Grid manipulation
    void SetCellType(int x, int y, GridCell::Type type);
    void GenerateMaze();
    void ClearWalls();
    
    // Getters
    [[nodiscard]] AnimationState GetState() const { return m_state; }
    [[nodiscard]] Algorithm GetAlgorithm() const { return m_currentAlgorithm; }

private:
    // Grid properties
    static constexpr int GRID_WIDTH = 40;
    static constexpr int GRID_HEIGHT = 25;
    std::vector<std::vector<GridCell>> m_grid;
    
    // Start and end positions
    GridCell* m_startCell = nullptr;
    GridCell* m_endCell = nullptr;
    
    // Algorithm state
    Algorithm m_currentAlgorithm = Algorithm::AStar;
    AnimationState m_state = AnimationState::Stopped;
    
    // Animation data
    std::vector<GridCell*> m_animationSteps;
    size_t m_currentStepIndex = 0;
    float m_animationSpeed = 1.0f;
    std::chrono::steady_clock::time_point m_lastUpdate;
    std::chrono::milliseconds m_stepDelay{50};
    
    // Pathfinding data
    std::vector<GridCell*> m_openSet;
    std::unordered_set<GridCell*> m_closedSet;
    std::vector<GridCell*> m_finalPath;
    
    // Statistics and timing
    int m_cellsExplored = 0;
    int m_pathLength = 0;
    std::chrono::milliseconds m_algorithmGenerationTime{0};
    std::chrono::milliseconds m_currentSearchTime{0};
    std::chrono::steady_clock::time_point m_searchStartTime;
    bool m_isSearchTimingActive = false;
    
    // UI state
    int m_selectedAlgorithm = 0;
    float m_selectedSpeed = 1.0f;
    [[maybe_unused]] bool m_isDragging = false;
    [[maybe_unused]] GridCell::Type m_dragType = GridCell::Type::Wall;
    
    const char* m_algorithmNames[4] = {
        "A* Algorithm", "Dijkstra's Algorithm", 
        "Breadth-First Search", "Depth-First Search"
    };
    
    // Audio
    AudioManager* m_audioManager = nullptr;
    bool m_audioEnabled = true;
    
    // Algorithm implementations
    void ExecuteAStar();
    void ExecuteDijkstra();
    void ExecuteBFS();
    void ExecuteDFS();
    
    // Helper methods
    void InitializeGrid();
    void ResetGridForSearch();
    void ReconstructPath(GridCell* endCell);
    float CalculateHeuristic(const GridCell& a, const GridCell& b);
    float CalculateDistance(const GridCell& a, const GridCell& b);
    std::vector<GridCell*> GetNeighbors(GridCell* cell);
    void RecordStep(GridCell* cell, GridCell::Type visualType);
    void ExecuteCurrentStep();
    void PlayStepSound(GridCell* cell, GridCell::Type stepType);
    
    // Grid utilities
    GridCell* GetCell(int x, int y);
    bool IsValidPosition(int x, int y);
    void HandleMouseInput();
};

} // namespace AlgorithmVisualizer 