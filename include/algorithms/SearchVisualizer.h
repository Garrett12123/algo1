#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include "audio/AudioManager.h"
#include <random>
#include <functional>

namespace AlgorithmVisualizer {

enum class SearchAlgorithm {
    LinearSearch,
    BinarySearch,
    InterpolationSearch,
    ExponentialSearch,
    JumpSearch
};

using PerformanceCallback = std::function<void(const std::string&, double, int, int)>;

struct SearchStep {
    std::string description;
    int currentIndex = -1;
    int lowIndex = -1;
    int highIndex = -1;
    int foundIndex = -1;
    bool isComparison = false;
    bool isFound = false;
    std::vector<int> searchRange;
};

class SearchVisualizer {
public:
    SearchVisualizer(std::shared_ptr<AlgorithmVisualizer::AudioManager> audioManager);
    ~SearchVisualizer() = default;

    void Render();
    void Update();

    void SetPerformanceCallback(PerformanceCallback callback) { m_performanceCallback = callback; }
    std::string GetAlgorithmName(SearchAlgorithm algorithm) const;

private:
    void RenderControls();
    void RenderVisualization();
    void RenderStatistics();
    
    // Algorithm implementations
    void LinearSearch(int target);
    void BinarySearch(int target);
    void InterpolationSearch(int target);
    void ExponentialSearch(int target);
    void JumpSearch(int target);
    
    // Utility functions
    void GenerateArray();
    void SortArray();
    void StartSearch();
    void StepSearch();
    void ResetSearch();
    void RecordStep(const std::string& description, int currentIndex = -1, 
                   int low = -1, int high = -1, bool isComparison = false);
    void PlayStepSound(bool isComparison, bool isFound);
    
    // Data
    std::vector<int> m_array;
    std::vector<SearchStep> m_steps;
    std::shared_ptr<AlgorithmVisualizer::AudioManager> m_audioManager;
    
    // Algorithm state
    SearchAlgorithm m_currentAlgorithm = SearchAlgorithm::BinarySearch;
    int m_arraySize = 20;
    int m_targetValue = 50;
    size_t m_currentStep = 0;
    bool m_isRunning = false;
    bool m_isComplete = false;
    bool m_targetFound = false;
    int m_foundIndex = -1;
    
    // Timing
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_endTime;
    double m_searchTime = 0.0;
    
    // Statistics
    int m_comparisons = 0;
    int m_maxComparisons = 0;
    
    // Animation
    float m_animationSpeed = 1.0f;
    std::chrono::high_resolution_clock::time_point m_lastStepTime;
    
    // UI state
    bool m_showCode = false;
    bool m_autoGenerate = true;
    
    // Performance tracking
    PerformanceCallback m_performanceCallback;
    std::chrono::steady_clock::time_point m_searchStartTime;
    int m_totalComparisons = 0;
    
    static constexpr const char* s_algorithmNames[] = {
        "Linear Search",
        "Binary Search", 
        "Interpolation Search",
        "Exponential Search",
        "Jump Search"
    };

    void CompleteSearch(int foundIndex);
};

} // namespace AlgorithmVisualizer 