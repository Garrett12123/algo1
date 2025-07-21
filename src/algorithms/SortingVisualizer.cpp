#include "algorithms/SortingVisualizer.h"
#include "audio/AudioManager.h"
#include "utils/Timer.h"
#include "Application.h"  // For Application class

#include <imgui.h>
#include <implot.h>
#include <algorithm>
#include <random>
#include <thread>
#include <cmath>
#include <fmt/core.h>

namespace AlgorithmVisualizer {

SortingVisualizer::SortingVisualizer(AudioManager* audioManager) 
    : m_randomGenerator(m_randomDevice()), m_audioManager(audioManager) {
    SetArraySize(m_arraySize);
    GenerateRandomArray();
}

void SortingVisualizer::Update() {
    if (m_state == AnimationState::Running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate);
        
        // Update current animation time
        if (m_isTimingActive) {
            m_currentAnimationTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_animationStartTime);
        }
        
        if (elapsed >= m_stepDelay) {
            StepForward();
            m_lastUpdate = now;
        }
    }
}

void SortingVisualizer::Render() {
    ImGui::Columns(2, "SortingColumns", true);
    
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
                                     ImVec4(0.0f, 1.0f, 1.0f, 0.6f));
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
            case SortingAlgorithm::BubbleSort:
                ImGui::TextWrapped("Bubble Sort repeatedly steps through the list, compares adjacent elements and swaps them if wrong order.");
                ImGui::Text("Time: O(n^2), Space: O(1)");
                ImGui::Text("Stable, simple but inefficient");
                break;
            case SortingAlgorithm::SelectionSort:
                ImGui::TextWrapped("Selection Sort finds minimum element and places it at beginning, then repeats for remainder.");
                ImGui::Text("Time: O(n^2), Space: O(1)");
                ImGui::Text("Not stable, minimal swaps");
                break;
            case SortingAlgorithm::InsertionSort:
                ImGui::TextWrapped("Insertion Sort builds sorted portion one element at a time by inserting into correct position.");
                ImGui::Text("Time: O(n^2), Space: O(1)");
                ImGui::Text("Stable, efficient for small arrays");
                break;
            case SortingAlgorithm::MergeSort:
                ImGui::TextWrapped("Merge Sort divides array in half recursively, then merges sorted halves back together.");
                ImGui::Text("Time: O(n log n), Space: O(n)");
                ImGui::Text("Stable, guaranteed O(n log n)");
                break;
            case SortingAlgorithm::QuickSort:
                ImGui::TextWrapped("Quick Sort picks pivot, partitions around it, then recursively sorts partitions.");
                ImGui::Text("Time: O(n log n) avg, O(n^2) worst, Space: O(log n)");
                ImGui::Text("Not stable, in-place, very fast average case");
                break;
            case SortingAlgorithm::HeapSort:
                ImGui::TextWrapped("Heap Sort builds max heap, then repeatedly extracts maximum to build sorted array.");
                ImGui::Text("Time: O(n log n), Space: O(1)");
                ImGui::Text("Not stable, guaranteed O(n log n), in-place");
                break;
            case SortingAlgorithm::TournamentSort:
                ImGui::TextWrapped("Tournament Sort builds a tournament tree to repeatedly find and remove the minimum element.");
                ImGui::Text("Time: O(n log n), Space: O(n)");
                ImGui::Text("Stable, good for external sorting, visualizes tree structure");
                break;
            case SortingAlgorithm::IntroSort:
                ImGui::TextWrapped("Introsort starts with QuickSort, switches to HeapSort when recursion gets too deep.");
                ImGui::Text("Time: O(n log n), Space: O(log n)");
                ImGui::Text("Not stable, hybrid approach, used in many standard libraries");
                break;
            case SortingAlgorithm::PatienceSort:
                ImGui::TextWrapped("Patience Sort uses card game strategy, finds longest increasing subsequence efficiently.");
                ImGui::Text("Time: O(n log n), Space: O(n)");
                ImGui::Text("Not stable, great for partially sorted data, elegant algorithm");
                break;
        }
        
        ImGui::Columns(1);
    }
    ImGui::EndChild();
    
    // Bottom right - Visualization with retro effects
    if (g_application) {
        // Draw glowing panel border for visualization area
        ImVec2 vizPanelPos = ImGui::GetCursorScreenPos();
        ImVec2 vizPanelSize = ImGui::GetContentRegionAvail();
        g_application->DrawNeonBorder(vizPanelPos, ImVec2(vizPanelPos.x + vizPanelSize.x, vizPanelPos.y + vizPanelSize.y), 
                                     ImVec4(1.0f, 0.0f, 1.0f, 0.4f)); // Magenta border
    }
    
    if (ImGui::BeginChild("VisualizationPanel", ImVec2(0, 0), true)) {
        // Add animated dots background
        if (g_application) {
            ImVec2 vizPos = ImGui::GetWindowPos();
            ImVec2 vizSize = ImGui::GetWindowSize();
            g_application->DrawAnimatedDots(vizPos, vizSize, 30);
        }
        RenderVisualization();
    }
    ImGui::EndChild();
    
    ImGui::Columns(1);
}

void SortingVisualizer::RenderControls() {
    ImGui::Text("Sorting Controls");
    
    // Animated separator for retro feel
    extern Application* g_application;
    if (g_application) {
        g_application->DrawAnimatedSeparator();
    } else {
        ImGui::Separator();
    }
    
    // Algorithm selection
    if (ImGui::Combo("Algorithm", &m_selectedAlgorithm, m_algorithmNames, 9)) {
        SetAlgorithm(static_cast<SortingAlgorithm>(m_selectedAlgorithm));
        ResetArray();
    }
    
    // Array size control
    if (ImGui::SliderInt("Array Size", &m_selectedArraySize, 10, 500)) {
        SetArraySize(m_selectedArraySize);
        ResetArray();
    }
    
    // Animation speed control
    if (ImGui::SliderFloat("Speed", &m_selectedSpeed, 0.1f, 10.0f, "%.1fx")) {
        SetAnimationSpeed(m_selectedSpeed);
    }
    
    ImGui::Spacing();
    
    // Audio controls
    ImGui::Text("Audio:");
    if (ImGui::Checkbox("Enable Sound", &m_audioEnabled)) {
        if (m_audioManager) {
            m_audioManager->SetEnabled(m_audioEnabled);
        }
    }
    
    if (m_audioEnabled && ImGui::SliderFloat("Volume", &m_audioVolume, 0.0f, 1.0f, "%.1f")) {
        if (m_audioManager) {
            m_audioManager->SetMasterVolume(m_audioVolume);
        }
    }
    
    ImGui::Spacing();
    
    // Array generation buttons
    ImGui::Text("Array Generation:");
    if (ImGui::Button("Random")) {
        GenerateRandomArray();
        ResetArray();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reversed")) {
        GenerateReversedArray();
        ResetArray();
    }
    ImGui::SameLine();
    if (ImGui::Button("Nearly Sorted")) {
        GenerateNearlySortedArray();
        ResetArray();
    }
    
    ImGui::Spacing();
    
    // Playback controls
    ImGui::Text("Playback Controls:");
    
    if (m_state == AnimationState::Stopped || m_state == AnimationState::Paused) {
        // Glowing start button
        extern Application* g_application;
        if (g_application) {
            g_application->DrawGlowingButton(">> Start", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            if (ImGui::IsItemClicked()) {
                StartSorting();
            }
        } else {
            if (ImGui::Button("Start")) {
                StartSorting();
            }
        }
    } else if (m_state == AnimationState::Running) {
        // Glowing pause button
        extern Application* g_application;
        if (g_application) {
            g_application->DrawGlowingButton("|| Pause", ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            if (ImGui::IsItemClicked()) {
                PauseSorting();
            }
        } else {
            if (ImGui::Button("Pause")) {
                PauseSorting();
            }
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        ResetArray();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Step Back")) {
        StepBackward();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Step Forward")) {
        StepForward();
    }
    
    ImGui::Spacing();
    ImGui::Text("Instructions:");
    ImGui::BulletText("Choose algorithm and array size");
    ImGui::BulletText("Click 'Generate Array' to create random data");
    ImGui::BulletText("Use 'Start Sorting' to begin animation");
    ImGui::BulletText("'Step' to advance one step at a time");
    

    
    // Animated progress bar with retro effects
    if (!m_sortingSteps.empty()) {
        float progress = static_cast<float>(m_currentStepIndex) / static_cast<float>(m_sortingSteps.size());
        
        // Get application instance to access retro UI effects
        extern Application* g_application;
        if (g_application) {
            g_application->DrawAnimatedProgressBar(progress, ImVec2(-1, 25), 
                fmt::format("Step {}/{}", m_currentStepIndex, m_sortingSteps.size()).c_str());
        } else {
            // Fallback to standard progress bar
            ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), 
                              fmt::format("Step {}/{}", m_currentStepIndex, m_sortingSteps.size()).c_str());
        }
    }
}

void SortingVisualizer::RenderStatistics() {
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
    
    ImGui::Text("Comparisons: %d", m_comparisons);
    ImGui::Text("Swaps: %d", m_swaps);
    ImGui::Text("Generation Time: %lld ms", m_algorithmGenerationTime.count());
    
    if (m_isTimingActive) {
        ImGui::Text("Animation Time: %lld ms", m_currentAnimationTime.count());
    } else if (m_state == AnimationState::Completed) {
        ImGui::Text("Animation Time: %lld ms", m_currentAnimationTime.count());
    } else {
        ImGui::Text("Animation Time: 0 ms");
    }
    
    if (m_state == AnimationState::Completed) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Sorting Complete!");
    }
    
    // Current step description
    if (!m_sortingSteps.empty() && m_currentStepIndex < m_sortingSteps.size()) {
        const auto& step = m_sortingSteps[m_currentStepIndex];
        if (!step.description.empty()) {
            ImGui::Spacing();
            ImGui::Text("Current Step:");
            ImGui::TextWrapped("%s", step.description.c_str());
        }
    }
}

void SortingVisualizer::RenderVisualization() {
    ImGui::Text("Array Visualization");
    ImGui::Separator();
    
    if (m_array.empty()) return;
    
    // Get the current step for highlighting
    int comp1 = -1, comp2 = -1, pivot = -1;
    bool swapped = false;
    
    if (!m_sortingSteps.empty() && m_currentStepIndex < m_sortingSteps.size()) {
        const auto& step = m_sortingSteps[m_currentStepIndex];
        comp1 = step.compareIndex1;
        comp2 = step.compareIndex2;
        pivot = step.pivotIndex;
        swapped = step.swapped;
    }
    
    // Create a bar chart visualization
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    canvas_size.y = std::min(canvas_size.y, 400.0f);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    if (canvas_size.x > 0 && canvas_size.y > 0) {
        // Calculate bar dimensions
        float bar_width = canvas_size.x / static_cast<float>(m_array.size());
        int max_value = *std::max_element(m_array.begin(), m_array.end());
        
        for (size_t i = 0; i < m_array.size(); ++i) {
            float bar_height = (static_cast<float>(m_array[i]) / static_cast<float>(max_value)) * canvas_size.y;
            
            ImVec2 bar_min(canvas_pos.x + static_cast<float>(i) * bar_width, 
                          canvas_pos.y + canvas_size.y - bar_height);
            ImVec2 bar_max(canvas_pos.x + static_cast<float>(i + 1) * bar_width - 1, 
                          canvas_pos.y + canvas_size.y);
            
            // Choose color based on current operation
            ImU32 color = IM_COL32(100, 150, 200, 255); // Default blue
            
            if (static_cast<int>(i) == pivot) {
                color = IM_COL32(255, 165, 0, 255); // Orange for pivot
            } else if (static_cast<int>(i) == comp1 || static_cast<int>(i) == comp2) {
                if (swapped) {
                    color = IM_COL32(255, 0, 0, 255); // Red for elements being swapped
                } else {
                    color = IM_COL32(255, 255, 0, 255); // Yellow for elements being compared
                }
            }
            
            draw_list->AddRectFilled(bar_min, bar_max, color);
            draw_list->AddRect(bar_min, bar_max, IM_COL32(50, 50, 50, 255));
        }
    }
    
    ImGui::Dummy(canvas_size);
    
    // Legend
    ImGui::Spacing();
    ImGui::Text("Legend:");
    ImGui::SameLine(); ImGui::ColorButton("Default", ImVec4(100/255.0f, 150/255.0f, 200/255.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Normal");
    
    ImGui::SameLine(); ImGui::ColorButton("Compare", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Comparing");
    
    ImGui::SameLine(); ImGui::ColorButton("Swap", ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Swapping");
    
    ImGui::SameLine(); ImGui::ColorButton("Pivot", ImVec4(1.0f, 165/255.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip);
    ImGui::SameLine(); ImGui::Text("Pivot");
}

void SortingVisualizer::StartSorting() {
    if (m_state == AnimationState::Stopped) {
        // Record start time for performance tracking
        m_sortStartTime = std::chrono::steady_clock::now();
        m_totalComparisons = 0;
        m_totalSwaps = 0;
        
        ClearSteps();
        m_comparisons = 0;
        m_swaps = 0;
        
        Timer timer;
        timer.Start();
        
        // Execute the selected sorting algorithm
        switch (m_currentAlgorithm) {
            case SortingAlgorithm::BubbleSort: BubbleSort(); break;
            case SortingAlgorithm::SelectionSort: SelectionSort(); break;
            case SortingAlgorithm::InsertionSort: InsertionSort(); break;
            case SortingAlgorithm::QuickSort: QuickSort(); break;
            case SortingAlgorithm::MergeSort: MergeSort(); break;
            case SortingAlgorithm::HeapSort: HeapSort(); break;
            case SortingAlgorithm::TournamentSort: TournamentSort(); break;
            case SortingAlgorithm::IntroSort: IntroSort(); break;
            case SortingAlgorithm::PatienceSort: PatienceSort(); break;
        }
        
        timer.Stop();
        m_algorithmGenerationTime = timer.GetElapsed();
        
        m_currentStepIndex = 0;
    }
    
    m_state = AnimationState::Running;
    m_lastUpdate = std::chrono::steady_clock::now();
    
    // Start timing the animation
    m_animationStartTime = std::chrono::steady_clock::now();
    m_isTimingActive = true;
    m_currentAnimationTime = std::chrono::milliseconds(0);
}

void SortingVisualizer::PauseSorting() {
    if (m_state == AnimationState::Running) {
        m_state = AnimationState::Paused;
        m_isTimingActive = false;
    }
}

void SortingVisualizer::ResetArray() {
    m_state = AnimationState::Stopped;
    m_array = m_originalArray;
    m_currentStepIndex = 0;
    ClearSteps();
    m_comparisons = 0;
    m_swaps = 0;
    m_algorithmGenerationTime = std::chrono::milliseconds(0);
    m_currentAnimationTime = std::chrono::milliseconds(0);
    m_isTimingActive = false;
}

void SortingVisualizer::StepForward() {
    if (!m_sortingSteps.empty() && m_currentStepIndex < m_sortingSteps.size()) {
        ExecuteCurrentStep();
        m_currentStepIndex++;
        
        if (m_currentStepIndex >= m_sortingSteps.size()) {
            m_state = AnimationState::Completed;
            m_isTimingActive = false;
            
            // Play completion sound when sorting animation finishes
            if (m_audioManager && m_audioEnabled) {
                m_audioManager->PlayCompletionSound();
            }
            
            // Report performance when algorithm completes
            if (m_performanceCallback) {
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_sortStartTime);
                
                std::string algorithmName = GetAlgorithmName(m_currentAlgorithm);
                m_performanceCallback(algorithmName, duration.count(), m_comparisons, m_swaps);
            }
            
            return;
        }
    }
}

void SortingVisualizer::StepBackward() {
    if (m_currentStepIndex > 0) {
        m_currentStepIndex--;
        ExecuteCurrentStep();
        
        if (m_state == AnimationState::Completed) {
            m_state = AnimationState::Paused;
        }
    }
}

void SortingVisualizer::SetArraySize(int size) {
    m_arraySize = std::clamp(size, 10, 500);
    m_selectedArraySize = m_arraySize;
    
    // Reset performance counters
    m_totalComparisons = 0;
    m_totalSwaps = 0;
}

void SortingVisualizer::SetAnimationSpeed(float speed) {
    m_animationSpeed = std::clamp(speed, 0.1f, 10.0f);
    m_selectedSpeed = m_animationSpeed;
    m_stepDelay = std::chrono::milliseconds(static_cast<int>(100 / m_animationSpeed));
}

void SortingVisualizer::SetAlgorithm(SortingAlgorithm algorithm) {
    m_currentAlgorithm = algorithm;
    m_selectedAlgorithm = static_cast<int>(algorithm);
}

void SortingVisualizer::GenerateRandomArray() {
    m_array.clear();
    m_array.reserve(m_arraySize);
    
    std::uniform_int_distribution<int> dist(1, m_arraySize);
    for (int i = 0; i < m_arraySize; ++i) {
        m_array.push_back(dist(m_randomGenerator));
    }
    
    m_originalArray = m_array;
}

void SortingVisualizer::GenerateReversedArray() {
    m_array.clear();
    m_array.reserve(m_arraySize);
    
    for (int i = m_arraySize; i >= 1; --i) {
        m_array.push_back(i);
    }
    
    m_originalArray = m_array;
}

void SortingVisualizer::GenerateNearlySortedArray() {
    m_array.clear();
    m_array.reserve(m_arraySize);
    
    for (int i = 1; i <= m_arraySize; ++i) {
        m_array.push_back(i);
    }
    
    // Shuffle a few elements
    std::uniform_int_distribution<int> dist(0, m_arraySize - 1);
    for (int i = 0; i < m_arraySize / 10; ++i) {
        int idx1 = dist(m_randomGenerator);
        int idx2 = dist(m_randomGenerator);
        std::swap(m_array[idx1], m_array[idx2]);
    }
    
    m_originalArray = m_array;
}

// Sorting algorithm implementations
void SortingVisualizer::BubbleSort() {
    auto arr = m_array;
    int n = static_cast<int>(arr.size());
    
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            RecordStep(arr, j, j + 1, -1, false, 
                      fmt::format("Comparing elements at positions {} and {}", j, j + 1));
            m_comparisons++;
            
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                m_swaps++;
                RecordStep(arr, j, j + 1, -1, true, 
                          fmt::format("Swapping elements at positions {} and {}", j, j + 1));
            }
        }
    }
    
    RecordStep(arr, -1, -1, -1, false, "Bubble sort completed!");
}

void SortingVisualizer::SelectionSort() {
    auto arr = m_array;
    int n = static_cast<int>(arr.size());
    
    for (int i = 0; i < n - 1; ++i) {
        int min_idx = i;
        RecordStep(arr, i, min_idx, -1, false, 
                  fmt::format("Finding minimum element from position {}", i));
        
        for (int j = i + 1; j < n; ++j) {
            RecordStep(arr, j, min_idx, -1, false, 
                      fmt::format("Comparing element at {} with current minimum at {}", j, min_idx));
            m_comparisons++;
            
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
                RecordStep(arr, j, min_idx, -1, false, 
                          fmt::format("New minimum found at position {}", min_idx));
            }
        }
        
        if (min_idx != i) {
            std::swap(arr[i], arr[min_idx]);
            m_swaps++;
            RecordStep(arr, i, min_idx, -1, true, 
                      fmt::format("Swapping minimum element to position {}", i));
        }
    }
    
    RecordStep(arr, -1, -1, -1, false, "Selection sort completed!");
}

void SortingVisualizer::InsertionSort() {
    auto arr = m_array;
    int n = static_cast<int>(arr.size());
    
    for (int i = 1; i < n; ++i) {
        int key = arr[i];
        int j = i - 1;
        
        RecordStep(arr, i, -1, -1, false, 
                  fmt::format("Inserting element {} into sorted portion", key));
        
        while (j >= 0 && arr[j] > key) {
            RecordStep(arr, j, j + 1, -1, false, 
                      fmt::format("Comparing {} with {}", arr[j], key));
            m_comparisons++;
            
            arr[j + 1] = arr[j];
            RecordStep(arr, j, j + 1, -1, true, 
                      fmt::format("Shifting element {} to the right", arr[j]));
            j--;
        }
        arr[j + 1] = key;
        RecordStep(arr, j + 1, -1, -1, false, 
                  fmt::format("Placed {} at position {}", key, j + 1));
    }
    
    RecordStep(arr, -1, -1, -1, false, "Insertion sort completed!");
}

void SortingVisualizer::QuickSort() {
    auto arr = m_array;
    QuickSortHelper(arr, 0, static_cast<int>(arr.size()) - 1);
    RecordStep(arr, -1, -1, -1, false, "Quick sort completed!");
}

void SortingVisualizer::QuickSortHelper(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = QuickSortPartition(arr, low, high);
        
        QuickSortHelper(arr, low, pi - 1);
        QuickSortHelper(arr, pi + 1, high);
    }
}

int SortingVisualizer::QuickSortPartition(std::vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    
    RecordStep(arr, -1, -1, high, false, 
              fmt::format("Using {} as pivot", pivot));
    
    for (int j = low; j <= high - 1; ++j) {
        RecordStep(arr, j, -1, high, false, 
                  fmt::format("Comparing {} with pivot {}", arr[j], pivot));
        m_comparisons++;
        
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
            m_swaps++;
            RecordStep(arr, i, j, high, true, 
                      fmt::format("Moving {} to left partition", arr[i]));
        }
    }
    
    std::swap(arr[i + 1], arr[high]);
    m_swaps++;
    RecordStep(arr, i + 1, high, i + 1, true, 
              "Placing pivot in final position");
    
    return i + 1;
}

void SortingVisualizer::MergeSort() {
    auto arr = m_array;
    MergeSortHelper(arr, 0, static_cast<int>(arr.size()) - 1);
    RecordStep(arr, -1, -1, -1, false, "Merge sort completed!");
}

void SortingVisualizer::MergeSortHelper(std::vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        
        RecordStep(arr, left, right, mid, false, 
                  fmt::format("Dividing array from {} to {} at {}", left, right, mid));
        
        MergeSortHelper(arr, left, mid);
        MergeSortHelper(arr, mid + 1, right);
        Merge(arr, left, mid, right);
    }
}

void SortingVisualizer::Merge(std::vector<int>& arr, int left, int mid, int right) {
    RecordStep(arr, left, right, mid, false, 
              fmt::format("Merging subarrays [{}, {}] and [{}, {}]", left, mid, mid + 1, right));
    
    std::vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;
    
    while (i <= mid && j <= right) {
        m_comparisons++;
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }
    
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    
    for (int i = 0; i < k; ++i) {
        arr[left + i] = temp[i];
    }
    
    RecordStep(arr, left, right, -1, false, 
              fmt::format("Merged subarray from {} to {}", left, right));
}

void SortingVisualizer::HeapSort() {
    auto arr = m_array;
    int n = static_cast<int>(arr.size());
    
    // Build heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        HeapifyDown(arr, n, i);
    }
    
    RecordStep(arr, -1, -1, -1, false, "Max heap built");
    
    // Extract elements one by one
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        m_swaps++;
        RecordStep(arr, 0, i, -1, true, 
                  fmt::format("Moving max element {} to sorted portion", arr[i]));
        
        HeapifyDown(arr, i, 0);
    }
    
    RecordStep(arr, -1, -1, -1, false, "Heap sort completed!");
}

void SortingVisualizer::HeapifyDown(std::vector<int>& arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n) {
        m_comparisons++;
        if (arr[left] > arr[largest]) {
            largest = left;
        }
    }
    
    if (right < n) {
        m_comparisons++;
        if (arr[right] > arr[largest]) {
            largest = right;
        }
    }
    
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        m_swaps++;
        RecordStep(arr, i, largest, -1, true, 
                  fmt::format("Maintaining heap property: swapping {} and {}", i, largest));
        HeapifyDown(arr, n, largest);
    }
}

void SortingVisualizer::RecordStep(const std::vector<int>& array, int comp1, int comp2, 
                                  int pivot, bool swapped, const std::string& desc) {
    SortingStep step;
    step.array = array;
    step.compareIndex1 = comp1;
    step.compareIndex2 = comp2;
    step.pivotIndex = pivot;
    step.swapped = swapped;
    step.description = desc;
    
    m_sortingSteps.push_back(step);
}

void SortingVisualizer::ClearSteps() {
    m_sortingSteps.clear();
    m_currentStepIndex = 0;
}

void SortingVisualizer::ExecuteCurrentStep() {
    if (!m_sortingSteps.empty() && m_currentStepIndex < m_sortingSteps.size()) {
        const auto& step = m_sortingSteps[m_currentStepIndex];
        m_array = step.array;
        
        // Play live audio feedback
        if (m_audioManager && m_audioEnabled) {
            if (step.swapped) {
                m_audioManager->PlaySwapSound();
            } else if (step.compareIndex1 >= 0 && step.compareIndex2 >= 0) {
                int maxVal = *std::max_element(step.array.begin(), step.array.end());
                float avgValue = (step.array[step.compareIndex1] + step.array[step.compareIndex2]) / 2.0f;
                float pitch = 0.5f + (avgValue / maxVal) * 1.0f;
                m_audioManager->PlayComparisonSound(pitch);
            }
        }
    }
}

// Tournament Sort Implementation
void SortingVisualizer::TournamentSort() {
    auto arr = m_array;
    int n = arr.size();
    
    RecordStep(arr, -1, -1, -1, false, "Starting Tournament Sort");
    
    // Build tournament tree (we'll simulate with a simple approach)
    std::vector<int> sorted;
    sorted.reserve(n);
    
    while (!arr.empty()) {
        // Find minimum element (tournament winner)
        auto minIt = std::min_element(arr.begin(), arr.end());
        int minIndex = std::distance(arr.begin(), minIt);
        int minValue = *minIt;
        
        // Record the comparison phase - use size variable to avoid changing size during loop
        int arrSize = static_cast<int>(arr.size());
        for (int i = 0; i < arrSize; ++i) {
            if (i != minIndex && i < static_cast<int>(arr.size())) {
                RecordStep(arr, minIndex, i, -1, false, 
                    fmt::format("Tournament: {} vs {} (winner: {})", minValue, arr[i], minValue));
                m_comparisons++;
            }
        }
        
        // Remove winner and add to sorted array
        sorted.push_back(minValue);
        arr.erase(minIt);
        
        RecordStep(arr, -1, -1, -1, false, 
            fmt::format("Tournament winner {} removed, {} elements remaining", minValue, arr.size()));
    }
    
    // Reconstruct the final sorted array
    RecordStep(sorted, -1, -1, -1, false, "Tournament Sort complete - all elements sorted");
}

// Introsort Implementation  
void SortingVisualizer::IntroSort() {
    auto arr = m_array;
    int n = arr.size();
    int depthLimit = 2 * (int)std::log2(n);
    
    RecordStep(arr, -1, -1, -1, false, 
        fmt::format("Starting Introsort with depth limit: {}", depthLimit));
    
    IntroSortUtil(arr, 0, n - 1, depthLimit);
    
    RecordStep(arr, -1, -1, -1, false, "Introsort complete");
}

void SortingVisualizer::IntroSortUtil(std::vector<int>& arr, int low, int high, int depthLimit) {
    if (low < high) {
        if (depthLimit == 0) {
            // Switch to HeapSort when recursion is too deep
            RecordStep(arr, low, high, -1, false, 
                fmt::format("Depth limit reached, switching to HeapSort for range [{}, {}]", low, high));
            
            // Heapify the subarray
            for (int i = (high - low + 1) / 2 - 1; i >= 0; i--) {
                IntroSortHeapify(arr, low, high, low + i);
            }
            
            // Extract elements from heap
            for (int i = high; i > low; i--) {
                std::swap(arr[low], arr[i]);
                RecordStep(arr, low, i, -1, true, 
                    fmt::format("HeapSort: Move max {} to position {}", arr[i], i));
                m_swaps++;
                IntroSortHeapify(arr, low, i - 1, low);
            }
        } else {
            // Use QuickSort
            int pi = IntroSortPartition(arr, low, high);
            IntroSortUtil(arr, low, pi - 1, depthLimit - 1);
            IntroSortUtil(arr, pi + 1, high, depthLimit - 1);
        }
    }
}

void SortingVisualizer::IntroSortHeapify(std::vector<int>& arr, int low, int high, int i) {
    int largest = i;
    int left = 2 * (i - low) + 1 + low;
    int right = 2 * (i - low) + 2 + low;
    
    if (left <= high && arr[left] > arr[largest]) {
        largest = left;
        m_comparisons++;
    }
    
    if (right <= high && arr[right] > arr[largest]) {
        largest = right;
        m_comparisons++;
    }
    
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        RecordStep(arr, i, largest, -1, true, 
            fmt::format("HeapSort heapify: swap {} and {}", arr[largest], arr[i]));
        m_swaps++;
        IntroSortHeapify(arr, low, high, largest);
    }
}

int SortingVisualizer::IntroSortPartition(std::vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    
    RecordStep(arr, -1, -1, high, false, fmt::format("QuickSort partition with pivot {}", pivot));
    
    for (int j = low; j < high; j++) {
        RecordStep(arr, j, high, high, false, 
            fmt::format("Compare {} with pivot {}", arr[j], pivot));
        m_comparisons++;
        
        if (arr[j] <= pivot) {
            i++;
            if (i != j) {
                std::swap(arr[i], arr[j]);
                RecordStep(arr, i, j, high, true, 
                    fmt::format("Swap {} and {}", arr[j], arr[i]));
                m_swaps++;
            }
        }
    }
    
    std::swap(arr[i + 1], arr[high]);
    RecordStep(arr, i + 1, high, i + 1, true, 
        fmt::format("Place pivot {} in final position", arr[i + 1]));
    m_swaps++;
    
    return i + 1;
}

// Patience Sort Implementation
void SortingVisualizer::PatienceSort() {
    auto arr = m_array;
    int n = arr.size();
    
    RecordStep(arr, -1, -1, -1, false, "Starting Patience Sort - like card game solitaire");
    
    // Create piles (stacks)
    std::vector<std::vector<int>> piles;
    
    // Place each element in the leftmost pile where it can go
    for (int i = 0; i < n; i++) {
        int element = arr[i];
        bool placed = false;
        
        // Find the leftmost pile where element can be placed
        for (size_t j = 0; j < piles.size(); j++) {
            if (!piles[j].empty() && piles[j].back() >= element) {
                piles[j].push_back(element);
                RecordStep(arr, i, -1, -1, true, 
                    fmt::format("Place {} on pile {} (top was {})", element, j, 
                               piles[j].size() >= 2 ? piles[j][piles[j].size()-2] : element));
                
                placed = true;
                break;
            }
            m_comparisons++;
        }
        
        // If no suitable pile found, create new pile
        if (!placed) {
            piles.push_back({element});
            RecordStep(arr, i, -1, -1, false, 
                fmt::format("Create new pile {} for element {}", piles.size()-1, element));
        }
    }
    
    RecordStep(arr, -1, -1, -1, false, 
        fmt::format("Patience phase complete - created {} piles", piles.size()));
    
    // Merge piles back into sorted order (using priority queue simulation)
    std::vector<int> result;
    
    while (!piles.empty()) {
        // Find pile with minimum top element
        size_t minPile = 0;
        for (size_t i = 1; i < piles.size(); i++) {
            if (!piles[i].empty() && !piles[minPile].empty() && 
                piles[i].back() < piles[minPile].back()) {
                minPile = i;
            }
            m_comparisons++;
        }
        
        // Take element from minimum pile
        if (!piles[minPile].empty()) {
            int element = piles[minPile].back();
            piles[minPile].pop_back();
            result.push_back(element);
            
            RecordStep(result, result.size()-1, -1, -1, true, 
                fmt::format("Extract minimum {} from pile {}", element, minPile));
            
            // Remove empty piles
            if (piles[minPile].empty()) {
                piles.erase(piles.begin() + minPile);
            }
        } else {
            break; // Safety check
        }
    }
    
    RecordStep(result, -1, -1, -1, false, "Patience Sort complete - final merge finished");
}

std::string SortingVisualizer::GetAlgorithmName(SortingAlgorithm algorithm) const {
    switch (algorithm) {
        case SortingAlgorithm::BubbleSort: return "Bubble Sort";
        case SortingAlgorithm::SelectionSort: return "Selection Sort";
        case SortingAlgorithm::InsertionSort: return "Insertion Sort";
        case SortingAlgorithm::QuickSort: return "Quick Sort";
        case SortingAlgorithm::MergeSort: return "Merge Sort";
        case SortingAlgorithm::HeapSort: return "Heap Sort";
        case SortingAlgorithm::TournamentSort: return "Tournament Sort";
        case SortingAlgorithm::IntroSort: return "Intro Sort";
        case SortingAlgorithm::PatienceSort: return "Patience Sort";
        default: return "Unknown";
    }
}

} // namespace AlgorithmVisualizer 