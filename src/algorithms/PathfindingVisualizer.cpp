#include "algorithms/PathfindingVisualizer.h"
#include "audio/AudioManager.h"
#include "Application.h"  // For Application class
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <random>

namespace AlgorithmVisualizer {

PathfindingVisualizer::PathfindingVisualizer(AudioManager* audioManager) 
    : m_audioManager(audioManager) {
    InitializeGrid();
}

void PathfindingVisualizer::Update() {
    if (m_state == AnimationState::Running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate);
        
        if (elapsed >= m_stepDelay) {
            StepForward();
            m_lastUpdate = now;
        }
    }
}

void PathfindingVisualizer::Render() {
    ImGui::Columns(2, "PathfindingColumns", true);
    
    // Left column - Controls only
    RenderControls();
    
    ImGui::NextColumn();
    
    // Right column - split into top (statistics/info) and bottom (grid)
    float rightColumnHeight = ImGui::GetContentRegionAvail().y;
    
    // Top right - Statistics and Algorithm Info with retro styling
    extern Application* g_application;
    if (g_application) {
        // Draw glowing panel border
        ImVec2 panelPos = ImGui::GetCursorScreenPos();
        ImVec2 panelSize = ImVec2(ImGui::GetContentRegionAvail().x, rightColumnHeight * 0.4f);
        g_application->DrawNeonBorder(panelPos, ImVec2(panelPos.x + panelSize.x, panelPos.y + panelSize.y), 
                                     ImVec4(0.0f, 1.0f, 0.0f, 0.6f)); // Green border for pathfinding
    }
    
    if (ImGui::BeginChild("TopRightPanel", ImVec2(0, rightColumnHeight * 0.4f), true)) {
        // Add retro grid background
        if (g_application) {
            ImVec2 childPos = ImGui::GetWindowPos();
            ImVec2 childSize = ImGui::GetWindowSize();
            g_application->DrawRetroGrid(childPos, childSize, 30.0f, 0.08f);
        }
        ImGui::Columns(2, "TopRightColumns", true);
        
        // Statistics
        RenderStatistics();
        
        ImGui::NextColumn();
        
        // Algorithm Information
        extern Application* g_application;
        if (g_application) {
            g_application->PushPulsingTextStyle(0.15f);
            ImGui::Text("Algorithm Details");
            g_application->PopPulsingTextStyle();
            g_application->DrawAnimatedSeparator();
        } else {
            ImGui::Text("Algorithm Details");
            ImGui::Separator();
        }
        switch (m_currentAlgorithm) {
            case Algorithm::AStar:
                ImGui::TextWrapped("A* uses both distance traveled and heuristic to find optimal paths efficiently.");
                ImGui::Text("Time: O(b^d), Space: O(b^d)");
                ImGui::Text("Guaranteed optimal with admissible heuristic");
                ImGui::Spacing();
                ImGui::Text("Heuristic: Manhattan distance");
                ImGui::Text("Best for: Shortest path with obstacles");
                break;
            case Algorithm::Dijkstra:
                ImGui::TextWrapped("Dijkstra's algorithm finds shortest paths by exploring nodes in order of distance.");
                ImGui::Text("Time: O(V^2), Space: O(V)");
                ImGui::Text("Guaranteed optimal for non-negative weights");
                ImGui::Spacing();
                ImGui::Text("Explores uniformly in all directions");
                ImGui::Text("Best for: Weighted shortest paths");
                break;
            case Algorithm::BreadthFirst:
                ImGui::TextWrapped("BFS explores all neighbors before moving to next level. Finds shortest unweighted path.");
                ImGui::Text("Time: O(V + E), Space: O(V)");
                ImGui::Text("Optimal for unweighted graphs");
                ImGui::Spacing();
                ImGui::Text("Explores level by level");
                ImGui::Text("Best for: Unweighted shortest paths");
                break;
            case Algorithm::DepthFirst:
                ImGui::TextWrapped("DFS explores as far as possible before backtracking. Not guaranteed to find optimal path.");
                ImGui::Text("Time: O(V + E), Space: O(V)");
                ImGui::Text("Not optimal, good for maze solving");
                ImGui::Spacing();
                ImGui::Text("Explores deeply first");
                ImGui::Text("Best for: Finding any path quickly");
                break;
        }
        
        ImGui::Columns(1);
    }
    ImGui::EndChild();
    
    // Bottom right - Grid Visualization with retro effects
    if (g_application) {
        // Draw glowing panel border for grid area
        ImVec2 gridPanelPos = ImGui::GetCursorScreenPos();
        ImVec2 gridPanelSize = ImGui::GetContentRegionAvail();
        g_application->DrawNeonBorder(gridPanelPos, ImVec2(gridPanelPos.x + gridPanelSize.x, gridPanelPos.y + gridPanelSize.y), 
                                     ImVec4(1.0f, 0.5f, 0.0f, 0.4f)); // Orange border for grid
    }
    
    if (ImGui::BeginChild("GridPanel", ImVec2(0, 0), true)) {
        // Add animated dots background
        if (g_application) {
            ImVec2 gridPos = ImGui::GetWindowPos();
            ImVec2 gridSize = ImGui::GetWindowSize();
            g_application->DrawAnimatedDots(gridPos, gridSize, 40);
        }
        RenderGrid();
    }
    ImGui::EndChild();
    
    ImGui::Columns(1);
}

void PathfindingVisualizer::RenderControls() {
    ImGui::Text("Pathfinding Controls");
    
    // Animated separator for retro feel
    extern Application* g_application;
    if (g_application) {
        g_application->DrawAnimatedSeparator();
    } else {
        ImGui::Separator();
    }
    
    // Algorithm selection
    if (ImGui::Combo("Algorithm", &m_selectedAlgorithm, m_algorithmNames, 4)) {
        m_currentAlgorithm = static_cast<Algorithm>(m_selectedAlgorithm);
        ResetGrid();
    }
    
    // Speed control
    if (ImGui::SliderFloat("Speed", &m_selectedSpeed, 0.1f, 10.0f, "%.1fx")) {
        m_animationSpeed = m_selectedSpeed;
        m_stepDelay = std::chrono::milliseconds(static_cast<int>(100 / m_animationSpeed));
    }
    
    ImGui::Spacing();
    
    // Grid manipulation
    ImGui::Text("Grid Tools:");
    if (ImGui::Button("Generate Maze")) {
        GenerateMaze();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Walls")) {
        ClearWalls();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Path")) {
        ClearPath();
    }
    
    ImGui::Spacing();
    
    // Playback controls
    ImGui::Text("Playback:");
    if (m_state == AnimationState::Stopped || m_state == AnimationState::Paused) {
        // Glowing start button
        extern Application* g_application;
        if (g_application) {
            g_application->DrawGlowingButton(">> Start Search", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            if (ImGui::IsItemClicked()) {
                StartPathfinding();
            }
        } else {
            if (ImGui::Button("Start Search")) {
                StartPathfinding();
            }
        }
    } else if (m_state == AnimationState::Running) {
        // Glowing pause button
        extern Application* g_application;
        if (g_application) {
            g_application->DrawGlowingButton("|| Pause", ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            if (ImGui::IsItemClicked()) {
                PausePathfinding();
            }
        } else {
            if (ImGui::Button("Pause")) {
                PausePathfinding();
            }
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        ResetGrid();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        StepForward();
    }
    
    ImGui::Spacing();
    
    // Audio controls
    ImGui::Text("Audio:");
    ImGui::Checkbox("Enable Sound", &m_audioEnabled);
    
    ImGui::Spacing();
    ImGui::Text("Instructions:");
    ImGui::BulletText("Left click: Place walls");
    ImGui::BulletText("Right click: Place start (green)");
    ImGui::BulletText("Middle click: Place end (red)");
    

}

void PathfindingVisualizer::RenderStatistics() {
    // Pulsing statistics header
    extern Application* g_application;
    if (g_application) {
        g_application->PushPulsingTextStyle(0.15f);
        ImGui::Text("Statistics");
        g_application->PopPulsingTextStyle();
        g_application->DrawAnimatedSeparator();
    } else {
        ImGui::Text("Statistics");
        ImGui::Separator();
    }
    
    ImGui::Text("Cells Explored: %d", m_cellsExplored);
    ImGui::Text("Path Length: %d", m_pathLength);
    ImGui::Text("Generation Time: %lld ms", m_algorithmGenerationTime.count());
    
    if (m_isSearchTimingActive) {
        ImGui::Text("Search Time: %lld ms", m_currentSearchTime.count());
    } else if (m_state == AnimationState::Completed) {
        ImGui::Text("Search Time: %lld ms", m_currentSearchTime.count());
    } else {
        ImGui::Text("Search Time: 0 ms");
    }
    
    if (m_state == AnimationState::Completed) {
        if (m_finalPath.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Path Found");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Path Found!");
        }
    }
}

void PathfindingVisualizer::RenderGrid() {
    ImGui::Text("Grid Visualization");
    ImGui::Separator();
    
    HandleMouseInput();
    
    // Get available space
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    canvas_size.y = std::min(canvas_size.y, 500.0f);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    if (canvas_size.x > 0 && canvas_size.y > 0) {
        float cell_width = canvas_size.x / GRID_WIDTH;
        float cell_height = canvas_size.y / GRID_HEIGHT;
        
        // Draw grid cells
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                const auto& cell = m_grid[y][x];
                
                ImVec2 cell_min(canvas_pos.x + x * cell_width, canvas_pos.y + y * cell_height);
                ImVec2 cell_max(cell_min.x + cell_width - 1, cell_min.y + cell_height - 1);
                
                // Choose color based on cell type
                ImU32 color = IM_COL32(255, 255, 255, 255); // Default white
                
                switch (cell.type) {
                    case GridCell::Type::Empty:
                        color = IM_COL32(255, 255, 255, 255); // White
                        break;
                    case GridCell::Type::Wall:
                        color = IM_COL32(50, 50, 50, 255); // Dark gray
                        break;
                    case GridCell::Type::Start:
                        color = IM_COL32(0, 255, 0, 255); // Green
                        break;
                    case GridCell::Type::End:
                        color = IM_COL32(255, 0, 0, 255); // Red
                        break;
                    case GridCell::Type::Visited:
                        color = IM_COL32(173, 216, 230, 255); // Light blue
                        break;
                    case GridCell::Type::Frontier:
                        color = IM_COL32(255, 255, 0, 255); // Yellow
                        break;
                    case GridCell::Type::Path:
                        color = IM_COL32(255, 165, 0, 255); // Orange
                        break;
                }
                
                draw_list->AddRectFilled(cell_min, cell_max, color);
                draw_list->AddRect(cell_min, cell_max, IM_COL32(128, 128, 128, 255)); // Grid lines
            }
        }
    }
    
    ImGui::Dummy(canvas_size);
    
    // Legend
    ImGui::Spacing();
    ImGui::Text("Legend:");
    ImGui::SameLine(); ImGui::ColorButton("Start", ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Start");
    ImGui::SameLine(); ImGui::ColorButton("End", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("End");
    ImGui::SameLine(); ImGui::ColorButton("Wall", ImVec4(0.2f, 0.2f, 0.2f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Wall");
    ImGui::SameLine(); ImGui::ColorButton("Visited", ImVec4(0.68f, 0.85f, 0.90f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Visited");
    ImGui::SameLine(); ImGui::ColorButton("Path", ImVec4(1.0f, 0.65f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Path");
}

void PathfindingVisualizer::StartPathfinding() {
    if (!m_startCell || !m_endCell) {
        return; // Need both start and end
    }
    
    ResetGridForSearch();
    m_state = AnimationState::Running;
    m_lastUpdate = std::chrono::steady_clock::now();
    
    // Start timing the search animation
    m_searchStartTime = std::chrono::steady_clock::now();
    m_isSearchTimingActive = true;
    m_currentSearchTime = std::chrono::milliseconds(0);
    
    auto generationStartTime = std::chrono::steady_clock::now();
    
    // Execute the selected algorithm
    switch (m_currentAlgorithm) {
        case Algorithm::AStar: ExecuteAStar(); break;
        case Algorithm::Dijkstra: ExecuteDijkstra(); break;
        case Algorithm::BreadthFirst: ExecuteBFS(); break;
        case Algorithm::DepthFirst: ExecuteDFS(); break;
    }
    
    auto generationEndTime = std::chrono::steady_clock::now();
    m_algorithmGenerationTime = std::chrono::duration_cast<std::chrono::milliseconds>(generationEndTime - generationStartTime);
    
    m_currentStepIndex = 0;
}

void PathfindingVisualizer::PausePathfinding() {
    if (m_state == AnimationState::Running) {
        m_state = AnimationState::Paused;
        m_isSearchTimingActive = false;
    }
}

void PathfindingVisualizer::ResetGrid() {
    m_state = AnimationState::Stopped;
    m_cellsExplored = 0;
    m_pathLength = 0;
    m_algorithmGenerationTime = std::chrono::milliseconds(0);
    m_currentSearchTime = std::chrono::milliseconds(0);
    m_isSearchTimingActive = false;
    m_animationSteps.clear();
    m_currentStepIndex = 0;
    m_finalPath.clear();
    
    // Reset all cells except walls, start, and end
    for (auto& row : m_grid) {
        for (auto& cell : row) {
            if (cell.type == GridCell::Type::Visited || 
                cell.type == GridCell::Type::Frontier || 
                cell.type == GridCell::Type::Path) {
                cell.type = GridCell::Type::Empty;
            }
            cell.gCost = 0.0f;
            cell.hCost = 0.0f;
            cell.fCost = 0.0f;
            cell.parent = nullptr;
        }
    }
}

void PathfindingVisualizer::ClearPath() {
    for (auto& row : m_grid) {
        for (auto& cell : row) {
            if (cell.type == GridCell::Type::Visited || 
                cell.type == GridCell::Type::Frontier || 
                cell.type == GridCell::Type::Path) {
                cell.type = GridCell::Type::Empty;
            }
        }
    }
    m_finalPath.clear();
    m_pathLength = 0;
}

void PathfindingVisualizer::StepForward() {
    if (m_currentStepIndex < m_animationSteps.size()) {
        GridCell* cell = m_animationSteps[m_currentStepIndex];
        [[maybe_unused]] GridCell::Type oldType = cell->type;
        
        ExecuteCurrentStep();
        
        // Play appropriate sound based on what happened
        PlayStepSound(cell, cell->type);
        
        m_currentStepIndex++;
    } else if (!m_finalPath.empty()) {
        // Animation complete, show final path
        for (auto* cell : m_finalPath) {
            if (cell->type != GridCell::Type::Start && cell->type != GridCell::Type::End) {
                cell->type = GridCell::Type::Path;
            }
        }
        m_state = AnimationState::Completed;
        
                 if (m_audioManager && m_audioEnabled) {
             m_audioManager->PlayPathFoundSound();
         }
             } else {
        m_state = AnimationState::Completed;
        m_isSearchTimingActive = false;
        if (m_audioManager && m_audioEnabled) {
            m_audioManager->PlayNoPathSound();
        }
    }
}

void PathfindingVisualizer::InitializeGrid() {
    m_grid.resize(GRID_HEIGHT);
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        m_grid[y].resize(GRID_WIDTH);
        for (int x = 0; x < GRID_WIDTH; ++x) {
            m_grid[y][x].x = x;
            m_grid[y][x].y = y;
            m_grid[y][x].type = GridCell::Type::Empty;
        }
    }
    
    // Set default start and end positions
    m_startCell = &m_grid[GRID_HEIGHT/2][5];
    m_startCell->type = GridCell::Type::Start;
    
    m_endCell = &m_grid[GRID_HEIGHT/2][GRID_WIDTH-6];
    m_endCell->type = GridCell::Type::End;
}

void PathfindingVisualizer::ResetGridForSearch() {
    m_openSet.clear();
    m_closedSet.clear();
    m_animationSteps.clear();
    m_finalPath.clear();
    m_cellsExplored = 0;
    m_pathLength = 0;
    
    // Reset pathfinding data
    for (auto& row : m_grid) {
        for (auto& cell : row) {
            cell.gCost = 0.0f;
            cell.hCost = 0.0f;
            cell.fCost = 0.0f;
            cell.parent = nullptr;
            
            if (cell.type == GridCell::Type::Visited || 
                cell.type == GridCell::Type::Frontier || 
                cell.type == GridCell::Type::Path) {
                cell.type = GridCell::Type::Empty;
            }
        }
    }
}

void PathfindingVisualizer::ExecuteAStar() {
    m_openSet.clear();
    m_closedSet.clear();
    
    m_startCell->gCost = 0.0f;
    m_startCell->hCost = CalculateHeuristic(*m_startCell, *m_endCell);
    m_startCell->fCost = m_startCell->gCost + m_startCell->hCost;
    
    m_openSet.push_back(m_startCell);
    
    while (!m_openSet.empty()) {
        // Find cell with lowest fCost
        GridCell* current = *std::min_element(m_openSet.begin(), m_openSet.end(),
            [](GridCell* a, GridCell* b) { return a->fCost < b->fCost; });
        
        // Remove current from open set
        m_openSet.erase(std::find(m_openSet.begin(), m_openSet.end(), current));
        m_closedSet.insert(current);
        
        RecordStep(current, GridCell::Type::Visited);
        m_cellsExplored++;
        
        if (current == m_endCell) {
            ReconstructPath(current);
            return;
        }
        
        for (GridCell* neighbor : GetNeighbors(current)) {
            if (m_closedSet.find(neighbor) != m_closedSet.end() || 
                neighbor->type == GridCell::Type::Wall) {
                continue;
            }
            
            float tentativeGCost = current->gCost + CalculateDistance(*current, *neighbor);
            
            bool inOpenSet = std::find(m_openSet.begin(), m_openSet.end(), neighbor) != m_openSet.end();
            
            if (!inOpenSet || tentativeGCost < neighbor->gCost) {
                neighbor->parent = current;
                neighbor->gCost = tentativeGCost;
                neighbor->hCost = CalculateHeuristic(*neighbor, *m_endCell);
                neighbor->fCost = neighbor->gCost + neighbor->hCost;
                
                if (!inOpenSet) {
                    m_openSet.push_back(neighbor);
                    RecordStep(neighbor, GridCell::Type::Frontier);
                }
            }
        }
    }
}

void PathfindingVisualizer::ExecuteDijkstra() {
    // Similar to A* but without heuristic
    m_openSet.clear();
    m_closedSet.clear();
    
    m_startCell->gCost = 0.0f;
    m_openSet.push_back(m_startCell);
    
    while (!m_openSet.empty()) {
        GridCell* current = *std::min_element(m_openSet.begin(), m_openSet.end(),
            [](GridCell* a, GridCell* b) { return a->gCost < b->gCost; });
        
        m_openSet.erase(std::find(m_openSet.begin(), m_openSet.end(), current));
        m_closedSet.insert(current);
        
        RecordStep(current, GridCell::Type::Visited);
        m_cellsExplored++;
        
        if (current == m_endCell) {
            ReconstructPath(current);
            return;
        }
        
        for (GridCell* neighbor : GetNeighbors(current)) {
            if (m_closedSet.find(neighbor) != m_closedSet.end() || 
                neighbor->type == GridCell::Type::Wall) {
                continue;
            }
            
            float tentativeGCost = current->gCost + CalculateDistance(*current, *neighbor);
            
            bool inOpenSet = std::find(m_openSet.begin(), m_openSet.end(), neighbor) != m_openSet.end();
            
            if (!inOpenSet || tentativeGCost < neighbor->gCost) {
                neighbor->parent = current;
                neighbor->gCost = tentativeGCost;
                
                if (!inOpenSet) {
                    m_openSet.push_back(neighbor);
                    RecordStep(neighbor, GridCell::Type::Frontier);
                }
            }
        }
    }
}

void PathfindingVisualizer::ExecuteBFS() {
    std::queue<GridCell*> queue;
    std::unordered_set<GridCell*> visited;
    
    queue.push(m_startCell);
    visited.insert(m_startCell);
    
    while (!queue.empty()) {
        GridCell* current = queue.front();
        queue.pop();
        
        RecordStep(current, GridCell::Type::Visited);
        m_cellsExplored++;
        
        if (current == m_endCell) {
            ReconstructPath(current);
            return;
        }
        
        for (GridCell* neighbor : GetNeighbors(current)) {
            if (visited.find(neighbor) != visited.end() || 
                neighbor->type == GridCell::Type::Wall) {
                continue;
            }
            
            neighbor->parent = current;
            queue.push(neighbor);
            visited.insert(neighbor);
            RecordStep(neighbor, GridCell::Type::Frontier);
        }
    }
}

void PathfindingVisualizer::ExecuteDFS() {
    std::stack<GridCell*> stack;
    std::unordered_set<GridCell*> visited;
    
    stack.push(m_startCell);
    
    while (!stack.empty()) {
        GridCell* current = stack.top();
        stack.pop();
        
        if (visited.find(current) != visited.end()) {
            continue;
        }
        
        visited.insert(current);
        RecordStep(current, GridCell::Type::Visited);
        m_cellsExplored++;
        
        if (current == m_endCell) {
            ReconstructPath(current);
            return;
        }
        
        for (GridCell* neighbor : GetNeighbors(current)) {
            if (visited.find(neighbor) != visited.end() || 
                neighbor->type == GridCell::Type::Wall) {
                continue;
            }
            
            neighbor->parent = current;
            stack.push(neighbor);
            RecordStep(neighbor, GridCell::Type::Frontier);
        }
    }
}

void PathfindingVisualizer::ReconstructPath(GridCell* endCell) {
    m_finalPath.clear();
    GridCell* current = endCell;
    
    while (current != nullptr) {
        m_finalPath.push_back(current);
        current = current->parent;
    }
    
    std::reverse(m_finalPath.begin(), m_finalPath.end());
    m_pathLength = static_cast<int>(m_finalPath.size()) - 1; // Don't count start cell
}

float PathfindingVisualizer::CalculateHeuristic(const GridCell& a, const GridCell& b) {
    // Manhattan distance
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

float PathfindingVisualizer::CalculateDistance(const GridCell& a, const GridCell& b) {
    // Euclidean distance
    float dx = static_cast<float>(a.x - b.x);
    float dy = static_cast<float>(a.y - b.y);
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<GridCell*> PathfindingVisualizer::GetNeighbors(GridCell* cell) {
    std::vector<GridCell*> neighbors;
    
    // Check 4-directional neighbors
    int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    
    for (auto& dir : directions) {
        int newX = cell->x + dir[0];
        int newY = cell->y + dir[1];
        
        if (IsValidPosition(newX, newY)) {
            neighbors.push_back(&m_grid[newY][newX]);
        }
    }
    
    return neighbors;
}

void PathfindingVisualizer::RecordStep(GridCell* cell, [[maybe_unused]] GridCell::Type visualType) {
    m_animationSteps.push_back(cell);
}

void PathfindingVisualizer::ExecuteCurrentStep() {
    if (m_currentStepIndex < m_animationSteps.size()) {
        GridCell* cell = m_animationSteps[m_currentStepIndex];
        if (cell->type != GridCell::Type::Start && cell->type != GridCell::Type::End) {
            cell->type = GridCell::Type::Visited;
        }
    }
}

void PathfindingVisualizer::PlayStepSound(GridCell* cell, [[maybe_unused]] GridCell::Type stepType) {
    if (!m_audioManager || !m_audioEnabled) return;
    
    // Calculate pitch based on position for spatial audio feel
    float normalizedX = static_cast<float>(cell->x) / static_cast<float>(GRID_WIDTH);
    float normalizedY = static_cast<float>(cell->y) / static_cast<float>(GRID_HEIGHT);
    float basePitch = 0.8f + (normalizedX * 0.4f) + (normalizedY * 0.2f);
    
    // Different sounds for different algorithm types
    switch (m_currentAlgorithm) {
        case Algorithm::AStar:
            // A* uses both distance and heuristic - higher pitch for better cells
            if (cell->fCost > 0) {
                float heuristicFactor = cell->hCost / (cell->fCost + 1.0f);
                m_audioManager->PlayExploreSound(basePitch + heuristicFactor * 0.3f);
            } else {
                m_audioManager->PlayExploreSound(basePitch);
            }
            break;
            
        case Algorithm::Dijkstra:
            // Dijkstra explores by distance - pitch based on distance
            if (cell->gCost > 0) {
                float distanceFactor = std::min(cell->gCost / 20.0f, 1.0f);
                m_audioManager->PlayVisitedSound(basePitch + distanceFactor * 0.2f);
            } else {
                m_audioManager->PlayVisitedSound(basePitch);
            }
            break;
            
        case Algorithm::BreadthFirst:
            // BFS explores level by level - consistent pitch per level
            m_audioManager->PlayFrontierSound(basePitch);
            break;
            
        case Algorithm::DepthFirst:
            // DFS goes deep - pitch increases with depth
            float depthPitch = basePitch + (normalizedY * 0.5f);
            m_audioManager->PlayExploreSound(depthPitch);
            break;
    }
}

GridCell* PathfindingVisualizer::GetCell(int x, int y) {
    if (IsValidPosition(x, y)) {
        return &m_grid[y][x];
    }
    return nullptr;
}

bool PathfindingVisualizer::IsValidPosition(int x, int y) {
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

void PathfindingVisualizer::HandleMouseInput() {
    // Basic mouse input handling - simplified for now
    // In a real implementation, you'd handle mouse clicks to place walls, start, end
}

void PathfindingVisualizer::SetCellType(int x, int y, GridCell::Type type) {
    if (IsValidPosition(x, y)) {
        m_grid[y][x].type = type;
    }
}

void PathfindingVisualizer::GenerateMaze() {
    // Simple maze generation - random walls
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (auto& row : m_grid) {
        for (auto& cell : row) {
            if (cell.type != GridCell::Type::Start && cell.type != GridCell::Type::End) {
                cell.type = dis(gen) < 0.3 ? GridCell::Type::Wall : GridCell::Type::Empty;
            }
        }
    }
}

void PathfindingVisualizer::ClearWalls() {
    for (auto& row : m_grid) {
        for (auto& cell : row) {
            if (cell.type == GridCell::Type::Wall) {
                cell.type = GridCell::Type::Empty;
            }
        }
    }
}

} // namespace AlgorithmVisualizer 