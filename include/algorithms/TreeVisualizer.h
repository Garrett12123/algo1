#pragma once

#include <vector>
#include <string>
#include <memory>
#include <queue>
#include <chrono>
#include <functional>
#include "audio/AudioManager.h"

// Forward declarations
struct ImDrawList;
struct ImVec2;

namespace AlgorithmVisualizer {

class AudioManager;

enum class TreeAlgorithm {
    BinarySearchTree,
    AVLTree,
    MinHeap,
    MaxHeap,
    RedBlackTree
};

enum class TreeOperation {
    Insert,
    Delete,
    Search,
    Traverse
};

using PerformanceCallback = std::function<void(const std::string&, double, int, int)>;

struct TreeNode {
    int value;
    std::shared_ptr<TreeNode> left;
    std::shared_ptr<TreeNode> right;
    int height = 1; // For AVL trees
    enum Color { RED, BLACK } color = RED; // For Red-Black trees
    float x = 0, y = 0; // For visualization positioning
    bool isHighlighted = false;
    bool isNew = false;
    bool isDeleted = false;
    
    TreeNode(int val) : value(val) {}
};

struct TreeStep {
    std::string description;
    std::shared_ptr<TreeNode> highlightedNode;
    std::vector<std::shared_ptr<TreeNode>> visitedNodes;
    bool isRotation = false;
    bool isInsertion = false;
    bool isDeletion = false;
    bool isComparison = false;
};

class TreeVisualizer {
public:
    TreeVisualizer(std::shared_ptr<AlgorithmVisualizer::AudioManager> audioManager);
    ~TreeVisualizer() = default;

    void Render();
    void Update();

    void SetPerformanceCallback(PerformanceCallback callback) { m_performanceCallback = callback; }
    std::string GetAlgorithmName(TreeAlgorithm algorithm) const;

private:
    void RenderControls();
    void RenderVisualization();
    void RenderStatistics();
    
    // Tree operations
    void InsertValue(int value);
    void DeleteValue(int value);
    void SearchValue(int value);
    void TraverseTree();
    
    // BST operations
    std::shared_ptr<TreeNode> BSTInsert(std::shared_ptr<TreeNode> root, int value);
    std::shared_ptr<TreeNode> BSTDelete(std::shared_ptr<TreeNode> root, int value);
    std::shared_ptr<TreeNode> BSTSearch(std::shared_ptr<TreeNode> root, int value);
    
    // AVL operations
    std::shared_ptr<TreeNode> AVLInsert(std::shared_ptr<TreeNode> root, int value);
    std::shared_ptr<TreeNode> AVLDelete(std::shared_ptr<TreeNode> root, int value);
    std::shared_ptr<TreeNode> RotateLeft(std::shared_ptr<TreeNode> x);
    std::shared_ptr<TreeNode> RotateRight(std::shared_ptr<TreeNode> y);
    int GetHeight(std::shared_ptr<TreeNode> node);
    int GetBalance(std::shared_ptr<TreeNode> node);
    void UpdateHeight(std::shared_ptr<TreeNode> node);
    
    // Heap operations
    void HeapInsert(int value);
    void HeapExtract();
    void HeapifyUp(int index);
    void HeapifyDown(int index);
    
    // Utility functions
    void CalculatePositions(std::shared_ptr<TreeNode> node, float x, float y, float spacing);
    void DrawNode(std::shared_ptr<TreeNode> node, ImDrawList* drawList, ImVec2 offset);
    void DrawEdge(std::shared_ptr<TreeNode> parent, std::shared_ptr<TreeNode> child, 
                  ImDrawList* drawList, ImVec2 offset);
    void ClearTree();
    void ResetVisualization();
    void RecordStep(const std::string& description, std::shared_ptr<TreeNode> highlighted = nullptr);
    void PlayStepSound(bool isComparison, bool isInsertion, bool isRotation);
    
    // Tree traversal
    void InorderTraversal(std::shared_ptr<TreeNode> node, std::vector<int>& result);
    void PreorderTraversal(std::shared_ptr<TreeNode> node, std::vector<int>& result);
    void PostorderTraversal(std::shared_ptr<TreeNode> node, std::vector<int>& result);
    void LevelOrderTraversal();
    
    // Data
    std::shared_ptr<TreeNode> m_root;
    std::vector<int> m_heap; // For heap visualization
    std::vector<TreeStep> m_steps;
    std::shared_ptr<AlgorithmVisualizer::AudioManager> m_audioManager;
    
    // Algorithm state
    TreeAlgorithm m_currentAlgorithm = TreeAlgorithm::BinarySearchTree;
    TreeOperation m_currentOperation = TreeOperation::Insert;
    int m_inputValue = 50;
    size_t m_currentStep = 0;
    bool m_isRunning = false;
    bool m_isComplete = false;
    
    // Timing
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_endTime;
    double m_operationTime = 0.0;
    
    // Statistics
    int m_nodeCount = 0;
    int m_treeHeight = 0;
    int m_comparisons = 0;
    int m_rotations = 0;
    
    // Animation
    float m_animationSpeed = 1.0f;
    std::chrono::high_resolution_clock::time_point m_lastStepTime;
    
    // UI state
    bool m_showTraversal = false;
    bool m_autoBalance = true;
    std::vector<int> m_traversalResult;
    
    // Performance tracking
    PerformanceCallback m_performanceCallback;
    std::chrono::steady_clock::time_point m_operationStartTime;
    int m_totalComparisons = 0;
    int m_totalOperations = 0;
    
    static constexpr const char* s_algorithmNames[] = {
        "Binary Search Tree",
        "AVL Tree",
        "Min Heap",
        "Max Heap",
        "Red-Black Tree"
    };
    
    static constexpr const char* s_operationNames[] = {
        "Insert",
        "Delete", 
        "Search",
        "Traverse"
    };
};

} // namespace AlgorithmVisualizer 