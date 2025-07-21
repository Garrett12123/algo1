#include "algorithms/TreeVisualizer.h"
#include "Application.h"  // For Application class
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <fmt/format.h>

namespace AlgorithmVisualizer {

TreeVisualizer::TreeVisualizer(std::shared_ptr<AlgorithmVisualizer::AudioManager> audioManager)
    : m_audioManager(audioManager) {
}

void TreeVisualizer::Render() {
    ImGui::Columns(2, "TreeColumns", true);
    
    // Left column - Controls only
    RenderControls();
    
    ImGui::NextColumn();
    
    // Right column - split into top (statistics/info) and bottom (visualization)
    float rightColumnHeight = ImGui::GetContentRegionAvail().y;
    
    // Top right - Statistics and Algorithm Info with retro styling
    extern Application* g_application;
    if (g_application) {
        // Draw glowing panel border
        ImVec2 panelPos = ImGui::GetCursorScreenPos();
        ImVec2 panelSize = ImVec2(ImGui::GetContentRegionAvail().x, rightColumnHeight * 0.35f);
        g_application->DrawNeonBorder(panelPos, ImVec2(panelPos.x + panelSize.x, panelPos.y + panelSize.y), 
                                     ImVec4(0.6f, 0.0f, 1.0f, 0.6f)); // Purple border for trees
    }
    
    if (ImGui::BeginChild("TopRightPanel", ImVec2(0, rightColumnHeight * 0.35f), true)) {
        // Add retro grid background
        if (g_application) {
            ImVec2 childPos = ImGui::GetWindowPos();
            ImVec2 childSize = ImGui::GetWindowSize();
            g_application->DrawRetroGrid(childPos, childSize, 25.0f, 0.08f);
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
            case TreeAlgorithm::BinarySearchTree:
                ImGui::TextWrapped("BST maintains the property: left subtree < node < right subtree for efficient searching.");
                ImGui::Text("Time: O(log n) avg, O(n) worst, Space: O(n)");
                ImGui::Text("Simple structure, no self-balancing");
                ImGui::Spacing();
                ImGui::Text("Insert/Search/Delete: O(h) where h is height");
                ImGui::Text("Can degenerate to linked list in worst case");
                break;
            case TreeAlgorithm::AVLTree:
                ImGui::TextWrapped("AVL Tree is self-balancing BST where height difference of subtrees is at most 1.");
                ImGui::Text("Time: O(log n), Space: O(n)");
                ImGui::Text("Guaranteed balanced, uses rotations");
                ImGui::Spacing();
                ImGui::Text("Height difference (balance factor) ≤ 1");
                ImGui::Text("Rotations: Left, Right, Left-Right, Right-Left");
                break;
            case TreeAlgorithm::MinHeap:
                ImGui::TextWrapped("Min Heap is complete binary tree where parent ≤ children, root is minimum.");
                ImGui::Text("Time: O(log n) insert/delete, O(1) min, Space: O(n)");
                ImGui::Text("Array implementation, complete tree");
                ImGui::Spacing();
                ImGui::Text("Parent at i, children at 2i+1 and 2i+2");
                ImGui::Text("Heapify operations maintain heap property");
                break;
            case TreeAlgorithm::MaxHeap:
                ImGui::TextWrapped("Max Heap is complete binary tree where parent ≥ children, root is maximum.");
                ImGui::Text("Time: O(log n) insert/delete, O(1) max, Space: O(n)");
                ImGui::Text("Array implementation, complete tree");
                ImGui::Spacing();
                ImGui::Text("Used in heap sort and priority queues");
                ImGui::Text("Efficient for finding maximum element");
                break;
            case TreeAlgorithm::RedBlackTree:
                ImGui::TextWrapped("Red-Black Tree is self-balancing BST with colored nodes following specific rules.");
                ImGui::Text("Time: O(log n), Space: O(n)");
                ImGui::Text("Balanced using colors and rotations");
                ImGui::Spacing();
                ImGui::Text("Rules: Root black, no red-red parent-child");
                ImGui::Text("Used in many standard libraries");
                break;
        }
        
        ImGui::Columns(1);
    }
    ImGui::EndChild();
    
    // Bottom right - Tree Visualization with retro effects
    if (g_application) {
        // Draw glowing panel border for tree visualization area
        ImVec2 treePanelPos = ImGui::GetCursorScreenPos();
        ImVec2 treePanelSize = ImGui::GetContentRegionAvail();
        g_application->DrawNeonBorder(treePanelPos, ImVec2(treePanelPos.x + treePanelSize.x, treePanelPos.y + treePanelSize.y), 
                                     ImVec4(1.0f, 1.0f, 0.0f, 0.4f)); // Yellow border for tree visualization
    }
    
    if (ImGui::BeginChild("TreePanel", ImVec2(0, 0), true)) {
        // Add animated dots background
        if (g_application) {
            ImVec2 treePos = ImGui::GetWindowPos();
            ImVec2 treeSize = ImGui::GetWindowSize();
            g_application->DrawAnimatedDots(treePos, treeSize, 25);
        }
        RenderVisualization();
    }
    ImGui::EndChild();
    
    ImGui::Columns(1);
}

void TreeVisualizer::RenderControls() {
    ImGui::Text("Tree Algorithms");
    ImGui::Separator();
    
    // Algorithm selection
    if (ImGui::Combo("Tree Type", reinterpret_cast<int*>(&m_currentAlgorithm), 
                     s_algorithmNames, static_cast<int>(TreeAlgorithm::RedBlackTree) + 1)) {
        ClearTree();
        ResetVisualization();
    }
    
    // Operation selection
    ImGui::Combo("Operation", reinterpret_cast<int*>(&m_currentOperation), 
                 s_operationNames, static_cast<int>(TreeOperation::Traverse) + 1);
    
    ImGui::Spacing();
    
    // Value input
    ImGui::SliderInt("Value", &m_inputValue, 1, 100);
    
    // Operation buttons
    ImGui::Text("Operations:");
    if (ImGui::Button("Insert") && m_currentOperation == TreeOperation::Insert) {
        InsertValue(m_inputValue);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Delete") && m_currentOperation == TreeOperation::Delete) {
        DeleteValue(m_inputValue);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Search") && m_currentOperation == TreeOperation::Search) {
        SearchValue(m_inputValue);
    }
    
    if (ImGui::Button("Traverse") && m_currentOperation == TreeOperation::Traverse) {
        TraverseTree();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear Tree")) {
        ClearTree();
        ResetVisualization();
    }
    
    ImGui::Spacing();
    
    // Animation controls
    ImGui::Text("Animation:");
    if (ImGui::Button("Step") && !m_steps.empty() && m_currentStep < m_steps.size()) {
        m_currentStep++;
        if (m_currentStep < m_steps.size()) {
            const auto& step = m_steps[m_currentStep];
            PlayStepSound(step.isComparison, step.isInsertion, step.isRotation);
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset Animation")) {
        ResetVisualization();
    }
    
    ImGui::SliderFloat("Speed", &m_animationSpeed, 0.1f, 5.0f);
    
    ImGui::Spacing();
    
    // Settings
    if (m_currentAlgorithm == TreeAlgorithm::AVLTree) {
        ImGui::Checkbox("Auto Balance", &m_autoBalance);
    }
    
    ImGui::Checkbox("Show Traversal", &m_showTraversal);
    
    if (m_showTraversal && !m_traversalResult.empty()) {
        ImGui::Text("Traversal Result:");
        std::string result;
        for (size_t i = 0; i < m_traversalResult.size(); ++i) {
            if (i > 0) result += ", ";
            result += std::to_string(m_traversalResult[i]);
        }
        ImGui::TextWrapped("%s", result.c_str());
    }
    
    ImGui::Spacing();
    
    // Instructions
    ImGui::Text("Instructions:");
    ImGui::BulletText("Select tree type and operation");
    ImGui::BulletText("Set value and click operation button");
    ImGui::BulletText("Use 'Step' to see animation details");
    ImGui::BulletText("Highlighted nodes show current operation");
}

void TreeVisualizer::RenderVisualization() {
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Clear background
    drawList->AddRectFilled(canvasPos, 
                           ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                           IM_COL32(30, 30, 30, 255));
    
    if (m_currentAlgorithm == TreeAlgorithm::MinHeap || m_currentAlgorithm == TreeAlgorithm::MaxHeap) {
        // Render heap as array and tree
        if (!m_heap.empty()) {
            // Draw heap as array
            float arrayY = canvasPos.y + canvasSize.y - 60;
            float elementWidth = std::min(40.0f, (canvasSize.x - 40) / m_heap.size());
            
            for (size_t i = 0; i < m_heap.size(); ++i) {
                float x = canvasPos.x + 20 + i * elementWidth;
                
                // Draw element box
                drawList->AddRectFilled(
                    ImVec2(x, arrayY),
                    ImVec2(x + elementWidth - 2, arrayY + 30),
                    IM_COL32(100, 100, 100, 255)
                );
                
                // Draw value
                std::string valueStr = std::to_string(m_heap[i]);
                ImVec2 textSize = ImGui::CalcTextSize(valueStr.c_str());
                drawList->AddText(
                    ImVec2(x + (elementWidth - textSize.x) / 2, arrayY + (30 - textSize.y) / 2),
                    IM_COL32(255, 255, 255, 255),
                    valueStr.c_str()
                );
                
                // Draw index
                std::string indexStr = std::to_string(i);
                ImVec2 indexSize = ImGui::CalcTextSize(indexStr.c_str());
                drawList->AddText(
                    ImVec2(x + (elementWidth - indexSize.x) / 2, arrayY + 35),
                    IM_COL32(200, 200, 200, 255),
                    indexStr.c_str()
                );
            }
            
            // Draw heap as tree structure above array
            // Simple layout for heap visualization
            float treeHeight = canvasSize.y - 120;
            int levels = static_cast<int>(std::floor(std::log2(m_heap.size()))) + 1;
            float levelHeight = treeHeight / (levels + 1);
            
            for (size_t i = 0; i < m_heap.size(); ++i) {
                int level = static_cast<int>(std::floor(std::log2(i + 1)));
                int posInLevel = i - (std::pow(2, level) - 1);
                int maxInLevel = std::pow(2, level);
                
                float x = canvasPos.x + canvasSize.x * (posInLevel + 0.5f) / maxInLevel;
                float y = canvasPos.y + 20 + level * levelHeight;
                
                // Draw connections to children
                if (2 * i + 1 < m_heap.size()) { // Left child
                    size_t leftChild = 2 * i + 1;
                    int childLevel = static_cast<int>(std::floor(std::log2(leftChild + 1)));
                    int childPosInLevel = leftChild - (std::pow(2, childLevel) - 1);
                    int childMaxInLevel = std::pow(2, childLevel);
                    
                    float childX = canvasPos.x + canvasSize.x * (childPosInLevel + 0.5f) / childMaxInLevel;
                    float childY = canvasPos.y + 20 + childLevel * levelHeight;
                    
                    drawList->AddLine(ImVec2(x, y + 15), ImVec2(childX, childY + 15), 
                                     IM_COL32(150, 150, 150, 255), 2.0f);
                }
                
                if (2 * i + 2 < m_heap.size()) { // Right child
                    size_t rightChild = 2 * i + 2;
                    int childLevel = static_cast<int>(std::floor(std::log2(rightChild + 1)));
                    int childPosInLevel = rightChild - (std::pow(2, childLevel) - 1);
                    int childMaxInLevel = std::pow(2, childLevel);
                    
                    float childX = canvasPos.x + canvasSize.x * (childPosInLevel + 0.5f) / childMaxInLevel;
                    float childY = canvasPos.y + 20 + childLevel * levelHeight;
                    
                    drawList->AddLine(ImVec2(x, y + 15), ImVec2(childX, childY + 15), 
                                     IM_COL32(150, 150, 150, 255), 2.0f);
                }
                
                // Draw node
                drawList->AddCircleFilled(ImVec2(x, y + 15), 15, IM_COL32(70, 70, 200, 255));
                drawList->AddCircle(ImVec2(x, y + 15), 15, IM_COL32(255, 255, 255, 255), 0, 2.0f);
                
                // Draw value
                std::string valueStr = std::to_string(m_heap[i]);
                ImVec2 textSize = ImGui::CalcTextSize(valueStr.c_str());
                drawList->AddText(
                    ImVec2(x - textSize.x / 2, y + 15 - textSize.y / 2),
                    IM_COL32(255, 255, 255, 255),
                    valueStr.c_str()
                );
            }
        }
    } else {
        // Render binary tree
        if (m_root) {
            // Calculate positions
            CalculatePositions(m_root, canvasSize.x / 2, 50, canvasSize.x / 4);
            
            // Draw tree
            DrawNode(m_root, drawList, canvasPos);
        }
    }
    
    // Draw current step description
    if (!m_steps.empty() && m_currentStep < m_steps.size()) {
        const auto& step = m_steps[m_currentStep];
        drawList->AddText(
            ImVec2(canvasPos.x + 10, canvasPos.y + 10),
            IM_COL32(255, 255, 0, 255),
            step.description.c_str()
        );
    }
    
    ImGui::Dummy(canvasSize);
}

void TreeVisualizer::RenderStatistics() {
    // Pulsing statistics header
    extern Application* g_application;
    if (g_application) {
        g_application->PushPulsingTextStyle(0.15f);
        ImGui::Text("Tree Statistics");
        g_application->PopPulsingTextStyle();
        g_application->DrawAnimatedSeparator();
    } else {
        ImGui::Text("Tree Statistics");
        ImGui::Separator();
    }
    
    if (m_currentAlgorithm == TreeAlgorithm::MinHeap || m_currentAlgorithm == TreeAlgorithm::MaxHeap) {
        ImGui::Text("Heap Size: %zu", m_heap.size());
        if (!m_heap.empty()) {
            if (m_currentAlgorithm == TreeAlgorithm::MinHeap) {
                ImGui::Text("Minimum: %d", m_heap[0]);
            } else {
                ImGui::Text("Maximum: %d", m_heap[0]);
            }
        }
    } else {
        ImGui::Text("Node Count: %d", m_nodeCount);
        ImGui::Text("Tree Height: %d", m_treeHeight);
        ImGui::Text("Comparisons: %d", m_comparisons);
        if (m_currentAlgorithm == TreeAlgorithm::AVLTree) {
            ImGui::Text("Rotations: %d", m_rotations);
        }
    }
    
    if (m_operationTime > 0) {
        ImGui::Text("Operation Time: %.3f ms", m_operationTime);
    }
    
    if (!m_steps.empty()) {
        ImGui::Text("Step: %zu/%zu", m_currentStep + 1, m_steps.size());
        
        // Animated progress bar
        float progress = static_cast<float>(m_currentStep) / m_steps.size();
        extern Application* g_application;
        if (g_application) {
            g_application->DrawAnimatedProgressBar(progress, ImVec2(-1, 25), "");
        } else {
            ImGui::ProgressBar(progress, ImVec2(-1, 0));
        }
    }
}

void TreeVisualizer::Update() {
    if (m_isRunning && !m_isComplete) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStepTime);
        
        if (elapsed.count() >= (1000 / m_animationSpeed)) {
            if (m_currentStep < m_steps.size()) {
                m_currentStep++;
                const auto& step = m_steps[m_currentStep - 1];
                PlayStepSound(step.isComparison, step.isInsertion, step.isRotation);
            }
            
            if (m_currentStep >= m_steps.size()) {
                m_isComplete = true;
                m_isRunning = false;
            }
            
            m_lastStepTime = now;
        }
    }
}

// Simplified implementations for basic functionality
void TreeVisualizer::InsertValue(int value) {
    m_startTime = std::chrono::high_resolution_clock::now();
    
    if (m_currentAlgorithm == TreeAlgorithm::MinHeap || m_currentAlgorithm == TreeAlgorithm::MaxHeap) {
        HeapInsert(value);
    } else {
        m_root = BSTInsert(m_root, value);
        m_nodeCount++;
    }
    
    m_endTime = std::chrono::high_resolution_clock::now();
    m_operationTime = std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count();
}

void TreeVisualizer::DeleteValue(int value) {
    m_startTime = std::chrono::high_resolution_clock::now();
    
    if (m_currentAlgorithm == TreeAlgorithm::MinHeap || m_currentAlgorithm == TreeAlgorithm::MaxHeap) {
        HeapExtract();
    } else {
        m_root = BSTDelete(m_root, value);
        if (m_nodeCount > 0) m_nodeCount--;
    }
    
    m_endTime = std::chrono::high_resolution_clock::now();
    m_operationTime = std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count();
}

void TreeVisualizer::SearchValue(int value) {
    m_startTime = std::chrono::high_resolution_clock::now();
    m_comparisons = 0;
    
    ResetVisualization();
    RecordStep(fmt::format("Searching for value {}", value));
    
    std::shared_ptr<TreeNode> result = BSTSearch(m_root, value);
    
    if (result) {
        RecordStep(fmt::format("Found value {} in tree!", value));
    } else {
        RecordStep(fmt::format("Value {} not found in tree", value));
    }
    
    m_endTime = std::chrono::high_resolution_clock::now();
    m_operationTime = std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count();
}

void TreeVisualizer::TraverseTree() {
    m_traversalResult.clear();
    ResetVisualization();
    
    if (m_root) {
        RecordStep("Starting in-order traversal");
        InorderTraversal(m_root, m_traversalResult);
        RecordStep("Traversal completed");
    }
}

// Basic BST implementations
std::shared_ptr<TreeNode> TreeVisualizer::BSTInsert(std::shared_ptr<TreeNode> root, int value) {
    if (!root) {
        RecordStep(fmt::format("Creating new node with value {}", value));
        auto newNode = std::make_shared<TreeNode>(value);
        newNode->isNew = true;
        return newNode;
    }
    
    RecordStep(fmt::format("Comparing {} with {}", value, root->value), root);
    m_comparisons++;
    
    if (value < root->value) {
        RecordStep(fmt::format("{} < {}, going left", value, root->value));
        root->left = BSTInsert(root->left, value);
    } else if (value > root->value) {
        RecordStep(fmt::format("{} > {}, going right", value, root->value));
        root->right = BSTInsert(root->right, value);
    }
    
    return root;
}

std::shared_ptr<TreeNode> TreeVisualizer::BSTDelete(std::shared_ptr<TreeNode> root, int value) {
    if (!root) {
        RecordStep(fmt::format("Value {} not found", value));
        return root;
    }
    
    RecordStep(fmt::format("Comparing {} with {}", value, root->value), root);
    m_comparisons++;
    
    if (value < root->value) {
        root->left = BSTDelete(root->left, value);
    } else if (value > root->value) {
        root->right = BSTDelete(root->right, value);
    } else {
        // Node to be deleted found
        RecordStep(fmt::format("Found node to delete: {}", value), root);
        root->isDeleted = true;
        
        if (!root->left) {
            return root->right;
        } else if (!root->right) {
            return root->left;
        }
        
        // Node with two children - find inorder successor
        std::shared_ptr<TreeNode> temp = root->right;
        while (temp->left) {
            temp = temp->left;
        }
        
        root->value = temp->value;
        root->right = BSTDelete(root->right, temp->value);
    }
    
    return root;
}

std::shared_ptr<TreeNode> TreeVisualizer::BSTSearch(std::shared_ptr<TreeNode> root, int value) {
    if (!root || root->value == value) {
        if (root) {
            RecordStep(fmt::format("Found value {}", value), root);
            root->isHighlighted = true;
        }
        return root;
    }
    
    RecordStep(fmt::format("Comparing {} with {}", value, root->value), root);
    m_comparisons++;
    
    if (value < root->value) {
        RecordStep(fmt::format("{} < {}, searching left", value, root->value));
        return BSTSearch(root->left, value);
    } else {
        RecordStep(fmt::format("{} > {}, searching right", value, root->value));
        return BSTSearch(root->right, value);
    }
}

// Basic heap implementations
void TreeVisualizer::HeapInsert(int value) {
    RecordStep(fmt::format("Inserting {} into heap", value));
    m_heap.push_back(value);
    HeapifyUp(m_heap.size() - 1);
    RecordStep(fmt::format("Inserted {} successfully", value));
}

void TreeVisualizer::HeapExtract() {
    if (m_heap.empty()) {
        RecordStep("Heap is empty, cannot extract");
        return;
    }
    
    int extracted = m_heap[0];
    RecordStep(fmt::format("Extracting root element: {}", extracted));
    
    m_heap[0] = m_heap.back();
    m_heap.pop_back();
    
    if (!m_heap.empty()) {
        HeapifyDown(0);
    }
    
    RecordStep(fmt::format("Extracted {} successfully", extracted));
}

void TreeVisualizer::HeapifyUp(int index) {
    if (index == 0) return;
    
    int parent = (index - 1) / 2;
    bool shouldSwap = false;
    
    if (m_currentAlgorithm == TreeAlgorithm::MinHeap) {
        shouldSwap = m_heap[index] < m_heap[parent];
    } else {
        shouldSwap = m_heap[index] > m_heap[parent];
    }
    
    if (shouldSwap) {
        RecordStep(fmt::format("Swapping {} with parent {}", m_heap[index], m_heap[parent]));
        std::swap(m_heap[index], m_heap[parent]);
        HeapifyUp(parent);
    }
}

void TreeVisualizer::HeapifyDown(int index) {
    int size = m_heap.size();
    int target = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    
    if (m_currentAlgorithm == TreeAlgorithm::MinHeap) {
        if (left < size && m_heap[left] < m_heap[target]) {
            target = left;
        }
        if (right < size && m_heap[right] < m_heap[target]) {
            target = right;
        }
    } else {
        if (left < size && m_heap[left] > m_heap[target]) {
            target = left;
        }
        if (right < size && m_heap[right] > m_heap[target]) {
            target = right;
        }
    }
    
    if (target != index) {
        RecordStep(fmt::format("Swapping {} with {}", m_heap[index], m_heap[target]));
        std::swap(m_heap[index], m_heap[target]);
        HeapifyDown(target);
    }
}

// Utility functions
void TreeVisualizer::CalculatePositions(std::shared_ptr<TreeNode> node, float x, float y, float spacing) {
    if (!node) return;
    
    node->x = x;
    node->y = y;
    
    if (node->left) {
        CalculatePositions(node->left, x - spacing, y + 60, spacing / 2);
    }
    if (node->right) {
        CalculatePositions(node->right, x + spacing, y + 60, spacing / 2);
    }
}

void TreeVisualizer::DrawNode(std::shared_ptr<TreeNode> node, ImDrawList* drawList, ImVec2 offset) {
    if (!node) return;
    
    ImVec2 pos = ImVec2(offset.x + node->x, offset.y + node->y);
    
    // Draw edges first
    if (node->left) {
        ImVec2 childPos = ImVec2(offset.x + node->left->x, offset.y + node->left->y);
        drawList->AddLine(pos, childPos, IM_COL32(150, 150, 150, 255), 2.0f);
        DrawNode(node->left, drawList, offset);
    }
    if (node->right) {
        ImVec2 childPos = ImVec2(offset.x + node->right->x, offset.y + node->right->y);
        drawList->AddLine(pos, childPos, IM_COL32(150, 150, 150, 255), 2.0f);
        DrawNode(node->right, drawList, offset);
    }
    
    // Draw node
    ImU32 nodeColor = IM_COL32(70, 70, 200, 255);
    if (node->isHighlighted) {
        nodeColor = IM_COL32(255, 255, 0, 255);
    } else if (node->isNew) {
        nodeColor = IM_COL32(0, 255, 0, 255);
    } else if (node->isDeleted) {
        nodeColor = IM_COL32(255, 0, 0, 255);
    }
    
    drawList->AddCircleFilled(pos, 20, nodeColor);
    drawList->AddCircle(pos, 20, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    
    // Draw value
    std::string valueStr = std::to_string(node->value);
    ImVec2 textSize = ImGui::CalcTextSize(valueStr.c_str());
    drawList->AddText(
        ImVec2(pos.x - textSize.x / 2, pos.y - textSize.y / 2),
        IM_COL32(255, 255, 255, 255),
        valueStr.c_str()
    );
}

void TreeVisualizer::InorderTraversal(std::shared_ptr<TreeNode> node, std::vector<int>& result) {
    if (!node) return;
    
    InorderTraversal(node->left, result);
    result.push_back(node->value);
    RecordStep(fmt::format("Visiting node {}", node->value), node);
    InorderTraversal(node->right, result);
}

void TreeVisualizer::ClearTree() {
    m_root = nullptr;
    m_heap.clear();
    m_nodeCount = 0;
    m_treeHeight = 0;
    m_comparisons = 0;
    m_rotations = 0;
    m_traversalResult.clear();
}

void TreeVisualizer::ResetVisualization() {
    m_steps.clear();
    m_currentStep = 0;
    m_isRunning = false;
    m_isComplete = false;
    m_operationTime = 0.0;
}

void TreeVisualizer::RecordStep(const std::string& description, std::shared_ptr<TreeNode> highlighted) {
    TreeStep step;
    step.description = description;
    step.highlightedNode = highlighted;
    m_steps.push_back(step);
}

void TreeVisualizer::PlayStepSound(bool isComparison, bool isInsertion, bool isRotation) {
    if (m_audioManager) {
        if (isRotation) {
            m_audioManager->PlayTreeRotation();
        } else if (isInsertion) {
            m_audioManager->PlayTreeInsert();
        } else if (isComparison) {
            m_audioManager->PlayTreeComparison();
        } else {
            m_audioManager->PlayTreeTraversal();
        }
    }
}

// Placeholder implementations for advanced features
std::shared_ptr<TreeNode> TreeVisualizer::AVLInsert(std::shared_ptr<TreeNode> root, int value) {
    // Simplified - would need full AVL implementation
    return BSTInsert(root, value);
}

std::shared_ptr<TreeNode> TreeVisualizer::AVLDelete(std::shared_ptr<TreeNode> root, int value) {
    return BSTDelete(root, value);
}

std::shared_ptr<TreeNode> TreeVisualizer::RotateLeft(std::shared_ptr<TreeNode> x) {
    RecordStep("Performing left rotation", x);
    m_rotations++;
    // Implementation would go here
    return x;
}

std::shared_ptr<TreeNode> TreeVisualizer::RotateRight(std::shared_ptr<TreeNode> y) {
    RecordStep("Performing right rotation", y);
    m_rotations++;
    // Implementation would go here
    return y;
}

int TreeVisualizer::GetHeight(std::shared_ptr<TreeNode> node) {
    return node ? node->height : 0;
}

int TreeVisualizer::GetBalance(std::shared_ptr<TreeNode> node) {
    return node ? GetHeight(node->left) - GetHeight(node->right) : 0;
}

void TreeVisualizer::UpdateHeight(std::shared_ptr<TreeNode> node) {
    if (node) {
        node->height = 1 + std::max(GetHeight(node->left), GetHeight(node->right));
    }
}

void TreeVisualizer::PreorderTraversal(std::shared_ptr<TreeNode> node, std::vector<int>& result) {
    if (!node) return;
    
    result.push_back(node->value);
    PreorderTraversal(node->left, result);
    PreorderTraversal(node->right, result);
}

void TreeVisualizer::PostorderTraversal(std::shared_ptr<TreeNode> node, std::vector<int>& result) {
    if (!node) return;
    
    PostorderTraversal(node->left, result);
    PostorderTraversal(node->right, result);
    result.push_back(node->value);
}

void TreeVisualizer::LevelOrderTraversal() {
    if (!m_root) return;
    
    std::queue<std::shared_ptr<TreeNode>> queue;
    queue.push(m_root);
    
    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();
        
        m_traversalResult.push_back(node->value);
        RecordStep(fmt::format("Visiting node {}", node->value), node);
        
        if (node->left) queue.push(node->left);
                 if (node->right) queue.push(node->right);
     }
 }

std::string TreeVisualizer::GetAlgorithmName(TreeAlgorithm algorithm) const {
    switch (algorithm) {
        case TreeAlgorithm::BinarySearchTree: return "Binary Search Tree";
        case TreeAlgorithm::AVLTree: return "AVL Tree";
        case TreeAlgorithm::MinHeap: return "Min Heap";
        case TreeAlgorithm::MaxHeap: return "Max Heap";
        case TreeAlgorithm::RedBlackTree: return "Red-Black Tree";
        default: return "Unknown Tree";
    }
}

} // namespace AlgorithmVisualizer 