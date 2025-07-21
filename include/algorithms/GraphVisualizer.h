#pragma once

#include <vector>
#include <string>

namespace AlgorithmVisualizer {

class AudioManager;

struct GraphNode {
    float x, y;
    int id;
    bool visited = false;
    bool inMST = false;
    std::string label;
};

struct GraphEdge {
    int from, to;
    float weight;
    bool highlighted = false;
    bool inMST = false;
};

class GraphVisualizer {
public:
    enum class Algorithm {
        KruskalMST,
        PrimMST,
        TopologicalSort,
        StronglyConnectedComponents
    };

public:
    GraphVisualizer(AudioManager* audioManager = nullptr);
    ~GraphVisualizer() = default;
    
    void Update();
    void Render();
    void RenderControls();
    void RenderGraph();
    void RenderStatistics();
    
    // Graph manipulation
    void GenerateRandomGraph();
    void ClearGraph();
    void ExecuteAlgorithm();

private:
    std::vector<GraphNode> m_nodes;
    std::vector<GraphEdge> m_edges;
    
    Algorithm m_currentAlgorithm = Algorithm::KruskalMST;
    int m_selectedAlgorithm = 0;
    
    // Statistics
    int m_mstWeight = 0;
    int m_componentsCount = 0;
    
    // Audio
    AudioManager* m_audioManager = nullptr;
    bool m_audioEnabled = true;
    
    const char* m_algorithmNames[4] = {
        "Kruskal's MST", "Prim's MST", 
        "Topological Sort", "Strongly Connected Components"
    };
    
    void InitializeSampleGraph();
    void ExecuteKruskalMST();
    void ExecutePrimMST();
};

} // namespace AlgorithmVisualizer 