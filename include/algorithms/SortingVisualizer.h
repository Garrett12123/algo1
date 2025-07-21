#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <memory>
#include <random>

namespace AlgorithmVisualizer {

class AudioManager;

class SortingVisualizer {
public:
    enum class SortingAlgorithm {
        BubbleSort,
        SelectionSort,
        InsertionSort,
        QuickSort,
        MergeSort,
        HeapSort,
        TournamentSort,
        IntroSort,
        PatienceSort
    };
    
    enum class AnimationState {
        Stopped,
        Running,
        Paused,
        Completed
    };
    
    struct SortingStep {
        std::vector<int> array;
        int compareIndex1 = -1;
        int compareIndex2 = -1;
        int pivotIndex = -1;
        bool swapped = false;
        std::string description;
    };

    using PerformanceCallback = std::function<void(const std::string&, double, int, int)>;

public:
    SortingVisualizer(AudioManager* audioManager = nullptr);
    ~SortingVisualizer() = default;
    
    void Update();
    void Render();
    void RenderControls();
    void RenderVisualization();
    void RenderStatistics();
    
    // Control methods
    void StartSorting();
    void PauseSorting();
    void ResetArray();
    void StepForward();
    void StepBackward();
    
    // Configuration
    void SetArraySize(int size);
    void SetAnimationSpeed(float speed);
    void SetAlgorithm(SortingAlgorithm algorithm);
    void GenerateRandomArray();
    void GenerateReversedArray();
    void GenerateNearlySortedArray();
    
    // Getters
    [[nodiscard]] AnimationState GetState() const { return m_state; }
    [[nodiscard]] SortingAlgorithm GetAlgorithm() const { return m_currentAlgorithm; }
    [[nodiscard]] const std::vector<int>& GetArray() const { return m_array; }
    [[nodiscard]] size_t GetCurrentStep() const { return m_currentStepIndex; }
    [[nodiscard]] size_t GetTotalSteps() const { return m_sortingSteps.size(); }

    void SetPerformanceCallback(PerformanceCallback callback) { m_performanceCallback = callback; }

    void ResetColors();
    
    std::string GetAlgorithmName(SortingAlgorithm algorithm) const;

private:
    // Sorting algorithms
    void BubbleSort();
    void SelectionSort();
    void InsertionSort();
    void QuickSort();
    void MergeSort();
    void HeapSort();
    void TournamentSort();
    void IntroSort();
    void PatienceSort();
    
    // Helper methods for specific algorithms
    void QuickSortHelper(std::vector<int>& arr, int low, int high);
    int QuickSortPartition(std::vector<int>& arr, int low, int high);
    void MergeSortHelper(std::vector<int>& arr, int left, int right);
    void Merge(std::vector<int>& arr, int left, int mid, int right);
    void HeapifyDown(std::vector<int>& arr, int n, int i);
    void IntroSortUtil(std::vector<int>& arr, int low, int high, int depthLimit);
    void IntroSortHeapify(std::vector<int>& arr, int low, int high, int i);
    int IntroSortPartition(std::vector<int>& arr, int low, int high);
    
    // Animation and recording
    void RecordStep(const std::vector<int>& array, int comp1 = -1, int comp2 = -1, 
                   int pivot = -1, bool swapped = false, const std::string& desc = "");
    void ClearSteps();
    void ExecuteCurrentStep();
    
    // Data members
    std::vector<int> m_array;
    std::vector<int> m_originalArray;
    std::vector<SortingStep> m_sortingSteps;
    
    SortingAlgorithm m_currentAlgorithm = SortingAlgorithm::BubbleSort;
    AnimationState m_state = AnimationState::Stopped;
    
    size_t m_currentStepIndex = 0;
    int m_arraySize = 50;
    float m_animationSpeed = 1.0f;
    
    // Timing
    std::chrono::steady_clock::time_point m_lastUpdate;
    std::chrono::milliseconds m_stepDelay{100};
    std::chrono::steady_clock::time_point m_animationStartTime;
    bool m_isTimingActive = false;
    
    // Statistics
    int m_comparisons = 0;
    int m_swaps = 0;
    std::chrono::milliseconds m_algorithmGenerationTime{0};
    std::chrono::milliseconds m_currentAnimationTime{0};
    
    // Random number generation
    std::random_device m_randomDevice;
    std::mt19937 m_randomGenerator;
    
    // UI state
    int m_selectedArraySize = 50;
    float m_selectedSpeed = 1.0f;
    int m_selectedAlgorithm = 0;
    const char* m_algorithmNames[9] = {
        "Bubble Sort", "Selection Sort", "Insertion Sort",
        "Quick Sort", "Merge Sort", "Heap Sort",
        "Tournament Sort", "Introsort", "Patience Sort"
    };
    
    // Audio
    AudioManager* m_audioManager;
    bool m_audioEnabled = true;
    float m_audioVolume = 0.5f;

    // Performance tracking
    PerformanceCallback m_performanceCallback;
    std::chrono::steady_clock::time_point m_sortStartTime;
    int m_totalComparisons = 0;
    int m_totalSwaps = 0;
};

} // namespace AlgorithmVisualizer 