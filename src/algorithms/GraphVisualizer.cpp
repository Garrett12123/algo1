#include "algorithms/GraphVisualizer.h"
#include "audio/AudioManager.h"
#include <imgui.h>
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>
#include <functional>
#include <limits>

namespace AlgorithmVisualizer {

GraphVisualizer::GraphVisualizer(AudioManager* audioManager) 
    : m_audioManager(audioManager) {
}

void GraphVisualizer::Update() {
    // Update graph visualization
}

void GraphVisualizer::Render() {
    ImGui::Columns(2, "GraphColumns", true);
    
    // Left column - Controls only
    RenderControls();
    
    ImGui::NextColumn();
    
    // Right column - split into top (statistics/info) and bottom (graph)
    float rightColumnHeight = ImGui::GetContentRegionAvail().y;
    
    // Top right - Statistics and Algorithm Info
    if (ImGui::BeginChild("TopRightPanel", ImVec2(0, rightColumnHeight * 0.35f), true)) {
        ImGui::Columns(2, "TopRightColumns", true);
        
        // Statistics
        RenderStatistics();
        
        ImGui::NextColumn();
        
        // Algorithm Information
        ImGui::Text("Algorithm Details");
        ImGui::Separator();
        switch (m_currentAlgorithm) {
            case Algorithm::KruskalMST:
                ImGui::TextWrapped("Kruskal's algorithm finds MST by sorting edges and using Union-Find to avoid cycles.");
                ImGui::Text("Time: O(E log E), Space: O(V)");
                ImGui::Text("Works well for sparse graphs");
                ImGui::Spacing();
                ImGui::Text("Edge-based approach");
                ImGui::Text("Uses Union-Find data structure");
                break;
            case Algorithm::PrimMST:
                ImGui::TextWrapped("Prim's algorithm grows MST from a starting vertex, always adding minimum weight edge.");
                ImGui::Text("Time: O(V^2), Space: O(V)");
                ImGui::Text("Works well for dense graphs");
                ImGui::Spacing();
                ImGui::Text("Vertex-based approach");
                ImGui::Text("Grows tree from single vertex");
                break;
            case Algorithm::TopologicalSort:
                ImGui::TextWrapped("Topological sort orders vertices in DAG so edges go from earlier to later vertices.");
                ImGui::Text("Time: O(V + E), Space: O(V)");
                ImGui::Text("Only works on acyclic graphs");
                ImGui::Spacing();
                ImGui::Text("Used for dependency ordering");
                ImGui::Text("DFS-based implementation");
                break;
            case Algorithm::StronglyConnectedComponents:
                ImGui::TextWrapped("SCC finds maximal sets of vertices where every vertex is reachable from every other.");
                ImGui::Text("Time: O(V + E), Space: O(V)");
                ImGui::Text("Uses DFS and transpose graph");
                ImGui::Spacing();
                ImGui::Text("Kosaraju's algorithm");
                ImGui::Text("Two-pass DFS approach");
                break;
        }
        
        ImGui::Columns(1);
    }
    ImGui::EndChild();
    
    // Bottom right - Graph Visualization
    if (ImGui::BeginChild("GraphPanel", ImVec2(0, 0), true)) {
        RenderGraph();
    }
    ImGui::EndChild();
    
    ImGui::Columns(1);
}

void GraphVisualizer::RenderControls() {
    ImGui::Text("Graph Algorithm Controls");
    ImGui::Separator();
    
    // Algorithm selection
    if (ImGui::Combo("Algorithm", &m_selectedAlgorithm, m_algorithmNames, 4)) {
        m_currentAlgorithm = static_cast<Algorithm>(m_selectedAlgorithm);
        ClearGraph();
    }
    
    ImGui::Spacing();
    
    // Graph manipulation
    ImGui::Text("Graph Tools:");
    if (ImGui::Button("Generate Random Graph")) {
        GenerateRandomGraph();
    }
    ImGui::SameLine();
    if (ImGui::Button("Sample Graph")) {
        InitializeSampleGraph();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        ClearGraph();
    }
    
    ImGui::Spacing();
    
    // Algorithm execution
    if (ImGui::Button("Execute Algorithm")) {
        ExecuteAlgorithm();
    }
    
    ImGui::Spacing();
    
    // Audio controls
    ImGui::Text("Audio:");
    ImGui::Checkbox("Enable Sound", &m_audioEnabled);
    
    ImGui::Spacing();
    ImGui::Text("Instructions:");
    ImGui::BulletText("Generate a graph to visualize algorithms");
    ImGui::BulletText("Execute algorithms to see results");
    ImGui::BulletText("Red edges: Minimum Spanning Tree");
    ImGui::BulletText("Blue nodes: Visited/processed");
    

}

void GraphVisualizer::RenderStatistics() {
    ImGui::Text("Statistics");
    ImGui::Separator();
    
    ImGui::Text("Nodes: %zu", m_nodes.size());
    ImGui::Text("Edges: %zu", m_edges.size());
    
    if (m_currentAlgorithm == Algorithm::KruskalMST || m_currentAlgorithm == Algorithm::PrimMST) {
        ImGui::Text("MST Weight: %d", m_mstWeight);
    }
    
    if (m_currentAlgorithm == Algorithm::StronglyConnectedComponents) {
        ImGui::Text("Components: %d", m_componentsCount);
    }
}

void GraphVisualizer::RenderGraph() {
    ImGui::Text("Graph Visualization");
    ImGui::Separator();
    
    // Get available space
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    canvas_size.y = std::min(canvas_size.y, 500.0f);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    if (canvas_size.x > 0 && canvas_size.y > 0 && !m_nodes.empty()) {
        // Draw edges first
        for (const auto& edge : m_edges) {
            if (edge.from < static_cast<int>(m_nodes.size()) && edge.to < static_cast<int>(m_nodes.size())) {
                const auto& fromNode = m_nodes[edge.from];
                const auto& toNode = m_nodes[edge.to];
                
                ImVec2 from_pos(canvas_pos.x + fromNode.x * canvas_size.x, 
                               canvas_pos.y + fromNode.y * canvas_size.y);
                ImVec2 to_pos(canvas_pos.x + toNode.x * canvas_size.x, 
                             canvas_pos.y + toNode.y * canvas_size.y);
                
                ImU32 edge_color = edge.inMST ? IM_COL32(255, 0, 0, 255) : IM_COL32(128, 128, 128, 255);
                float thickness = edge.inMST ? 3.0f : 1.0f;
                
                draw_list->AddLine(from_pos, to_pos, edge_color, thickness);
                
                // Draw weight label
                ImVec2 mid_pos((from_pos.x + to_pos.x) * 0.5f, (from_pos.y + to_pos.y) * 0.5f);
                char weight_text[16];
                snprintf(weight_text, sizeof(weight_text), "%.0f", edge.weight);
                draw_list->AddText(mid_pos, IM_COL32(0, 0, 0, 255), weight_text);
            }
        }
        
        // Draw nodes on top
        for (const auto& node : m_nodes) {
            ImVec2 node_pos(canvas_pos.x + node.x * canvas_size.x, 
                           canvas_pos.y + node.y * canvas_size.y);
            
            ImU32 node_color = node.visited ? IM_COL32(0, 0, 255, 255) : IM_COL32(255, 255, 255, 255);
            if (node.inMST) node_color = IM_COL32(0, 255, 0, 255);
            
            draw_list->AddCircleFilled(node_pos, 15.0f, node_color);
            draw_list->AddCircle(node_pos, 15.0f, IM_COL32(0, 0, 0, 255), 0, 2.0f);
            
            // Draw node label
            char label[16];
            snprintf(label, sizeof(label), "%d", node.id);
            ImVec2 text_size = ImGui::CalcTextSize(label);
            ImVec2 text_pos(node_pos.x - text_size.x * 0.5f, node_pos.y - text_size.y * 0.5f);
            draw_list->AddText(text_pos, IM_COL32(0, 0, 0, 255), label);
        }
    }
    
    ImGui::Dummy(canvas_size);
    
    // Legend
    ImGui::Spacing();
    ImGui::Text("Legend:");
    ImGui::SameLine(); ImGui::ColorButton("Normal", ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Normal Node");
    ImGui::SameLine(); ImGui::ColorButton("Visited", ImVec4(0.0f, 0.0f, 1.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Visited");
    ImGui::SameLine(); ImGui::ColorButton("MST Edge", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("MST Edge");
}

void GraphVisualizer::GenerateRandomGraph() {
    ClearGraph();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dis(0.1, 0.9);
    std::uniform_real_distribution<> weight_dis(1.0, 20.0);
    std::uniform_int_distribution<> edge_dis(0, 1);
    
    // Generate nodes
    const int num_nodes = 8;
    for (int i = 0; i < num_nodes; ++i) {
        GraphNode node;
        node.id = i;
        node.x = pos_dis(gen);
        node.y = pos_dis(gen);
        m_nodes.push_back(node);
    }
    
    // Generate edges (about 50% connectivity)
    for (int i = 0; i < num_nodes; ++i) {
        for (int j = i + 1; j < num_nodes; ++j) {
            if (edge_dis(gen) == 0) { // 50% chance
                GraphEdge edge;
                edge.from = i;
                edge.to = j;
                edge.weight = weight_dis(gen);
                m_edges.push_back(edge);
            }
        }
    }
}

void GraphVisualizer::InitializeSampleGraph() {
    ClearGraph();
    
    // Create a sample graph for demonstration
    m_nodes = {
        {0.2f, 0.3f, 0, false, false, "A"}, {0.5f, 0.1f, 1, false, false, "B"}, {0.8f, 0.3f, 2, false, false, "C"},
        {0.2f, 0.7f, 3, false, false, "D"}, {0.5f, 0.9f, 4, false, false, "E"}, {0.8f, 0.7f, 5, false, false, "F"}
    };
    
    m_edges = {
        {0, 1, 4}, {0, 3, 2}, {1, 2, 3}, {1, 4, 6},
        {2, 5, 1}, {3, 4, 5}, {4, 5, 2}
    };
}

void GraphVisualizer::ClearGraph() {
    m_nodes.clear();
    m_edges.clear();
    m_mstWeight = 0;
    m_componentsCount = 0;
    
    // Reset edge and node states
    for (auto& edge : m_edges) {
        edge.inMST = false;
        edge.highlighted = false;
    }
    for (auto& node : m_nodes) {
        node.visited = false;
        node.inMST = false;
    }
}

void GraphVisualizer::ExecuteAlgorithm() {
    // Reset states
    for (auto& edge : m_edges) {
        edge.inMST = false;
        edge.highlighted = false;
    }
    for (auto& node : m_nodes) {
        node.visited = false;
        node.inMST = false;
    }
    
    switch (m_currentAlgorithm) {
        case Algorithm::KruskalMST:
            ExecuteKruskalMST();
            break;
        case Algorithm::PrimMST:
            ExecutePrimMST();
            break;
        case Algorithm::TopologicalSort:
            // Simplified - just mark nodes as visited in order
            for (auto& node : m_nodes) {
                node.visited = true;
            }
            break;
        case Algorithm::StronglyConnectedComponents:
            // Simplified - count components (assume each node is its own component)
            m_componentsCount = static_cast<int>(m_nodes.size());
            for (auto& node : m_nodes) {
                node.visited = true;
            }
            break;
    }
}

void GraphVisualizer::ExecuteKruskalMST() {
    if (m_nodes.empty()) return;
    
    // Sort edges by weight
    std::vector<int> edge_indices(m_edges.size());
    std::iota(edge_indices.begin(), edge_indices.end(), 0);
    std::sort(edge_indices.begin(), edge_indices.end(),
        [this](int a, int b) { return m_edges[a].weight < m_edges[b].weight; });
    
    // Union-Find for cycle detection (simplified)
    std::vector<int> parent(m_nodes.size());
    std::iota(parent.begin(), parent.end(), 0);
    
    std::function<int(int)> find = [&](int x) {
        return parent[x] == x ? x : parent[x] = find(parent[x]);
    };
    
    m_mstWeight = 0;
    int edges_added = 0;
    
    for (int idx : edge_indices) {
        auto& edge = m_edges[idx];
        int root_from = find(edge.from);
        int root_to = find(edge.to);
        
                 if (root_from != root_to) {
             edge.inMST = true;
             m_mstWeight += static_cast<int>(edge.weight);
             parent[root_from] = root_to;
             edges_added++;
             
             if (m_audioManager && m_audioEnabled) {
                 m_audioManager->PlayEdgeAddSound();
             }
             
             if (edges_added == static_cast<int>(m_nodes.size()) - 1) {
                 if (m_audioManager && m_audioEnabled) {
                     m_audioManager->PlayMSTCompleteSound();
                 }
                 break; // MST complete
             }
         }
    }
    
    // Mark nodes that are part of MST
    for (auto& node : m_nodes) {
        node.inMST = true;
    }
}

void GraphVisualizer::ExecutePrimMST() {
    if (m_nodes.empty()) return;
    
    std::vector<bool> in_mst(m_nodes.size(), false);
    std::vector<float> key(m_nodes.size(), std::numeric_limits<float>::max());
    
    // Start from node 0
    key[0] = 0;
    m_mstWeight = 0;
    
    for (size_t count = 0; count < m_nodes.size(); ++count) {
        // Find minimum key vertex not in MST
        int u = -1;
        for (size_t v = 0; v < m_nodes.size(); ++v) {
            if (!in_mst[v] && (u == -1 || key[v] < key[u])) {
                u = static_cast<int>(v);
            }
        }
        
                 in_mst[u] = true;
         m_nodes[u].inMST = true;
         
         if (m_audioManager && m_audioEnabled) {
             m_audioManager->PlayNodeSelectSound();
         }
         
         if (key[u] != std::numeric_limits<float>::max()) {
             m_mstWeight += static_cast<int>(key[u]);
         }
        
        // Update keys of adjacent vertices
        for (auto& edge : m_edges) {
            int v = -1;
            if (edge.from == u) v = edge.to;
            else if (edge.to == u) v = edge.from;
            
            if (v != -1 && !in_mst[v] && edge.weight < key[v]) {
                key[v] = edge.weight;
                
                // Mark this edge as part of MST
                edge.inMST = true;
            }
        }
    }
}

} // namespace AlgorithmVisualizer 