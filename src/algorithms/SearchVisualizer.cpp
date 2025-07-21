#include "algorithms/SearchVisualizer.h"
#include "Application.h"  // For Application class
#include <imgui.h>
#include <algorithm>
#include <random>
#include <cmath>
#include <fmt/format.h>

namespace AlgorithmVisualizer {

SearchVisualizer::SearchVisualizer(std::shared_ptr<AlgorithmVisualizer::AudioManager> audioManager)
    : m_audioManager(audioManager) {
    GenerateArray();
    SortArray(); // Most search algorithms require sorted arrays
}

void SearchVisualizer::Render() {
    ImGui::Columns(2, "SearchColumns", true);
    
    // Left column - Controls only
    RenderControls();
    
    ImGui::NextColumn();
    
    // Right column - split into top (statistics/info) and bottom (visualization)
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
            case SearchAlgorithm::LinearSearch:
                ImGui::TextWrapped("Linear Search checks each element sequentially until target is found or array ends.");
                ImGui::Text("Time: O(n), Space: O(1)");
                ImGui::Text("Works on unsorted arrays");
                ImGui::Spacing();
                ImGui::Text("Best case: O(1) - target at start");
                ImGui::Text("Worst case: O(n) - target at end");
                break;
            case SearchAlgorithm::BinarySearch:
                ImGui::TextWrapped("Binary Search repeatedly divides sorted array in half, comparing target with middle element.");
                ImGui::Text("Time: O(log n), Space: O(1)");
                ImGui::Text("Requires sorted array");
                ImGui::Spacing();
                ImGui::Text("Eliminates half the search space each step");
                ImGui::Text("Most efficient for large sorted datasets");
                break;
            case SearchAlgorithm::InterpolationSearch:
                ImGui::TextWrapped("Interpolation Search estimates target position based on value distribution in sorted array.");
                ImGui::Text("Time: O(log log n) avg, O(n) worst, Space: O(1)");
                ImGui::Text("Best for uniformly distributed data");
                ImGui::Spacing();
                ImGui::Text("Uses interpolation formula");
                ImGui::Text("Can be faster than binary search");
                break;
            case SearchAlgorithm::ExponentialSearch:
                ImGui::TextWrapped("Exponential Search finds range containing target by doubling index, then uses binary search.");
                ImGui::Text("Time: O(log n), Space: O(1)");
                ImGui::Text("Good for unbounded arrays");
                ImGui::Spacing();
                ImGui::Text("Two-phase algorithm");
                ImGui::Text("Combines exponential growth with binary search");
                break;
            case SearchAlgorithm::JumpSearch:
                ImGui::TextWrapped("Jump Search skips elements by fixed step size, then linear searches in identified block.");
                ImGui::Text("Time: O(√n), Space: O(1)");
                ImGui::Text("Balance between linear and binary");
                ImGui::Spacing();
                ImGui::Text("Optimal jump size: √n");
                ImGui::Text("Good compromise algorithm");
                break;
        }
        
        ImGui::Columns(1);
    }
    ImGui::EndChild();
    
    // Bottom right - Visualization with retro effects
    extern Application* g_application;
    if (g_application) {
        // Draw glowing panel border for visualization area
        ImVec2 vizPanelPos = ImGui::GetCursorScreenPos();
        ImVec2 vizPanelSize = ImGui::GetContentRegionAvail();
        g_application->DrawNeonBorder(vizPanelPos, ImVec2(vizPanelPos.x + vizPanelSize.x, vizPanelPos.y + vizPanelSize.y), 
                                     ImVec4(0.0f, 0.8f, 1.0f, 0.4f)); // Cyan border for search visualization
    }
    
    if (ImGui::BeginChild("VisualizationPanel", ImVec2(0, 0), true)) {
        // Add animated dots background
        if (g_application) {
            ImVec2 vizPos = ImGui::GetWindowPos();
            ImVec2 vizSize = ImGui::GetWindowSize();
            g_application->DrawAnimatedDots(vizPos, vizSize, 35);
        }
        RenderVisualization();
    }
    ImGui::EndChild();
    
    ImGui::Columns(1);
}

void SearchVisualizer::RenderControls() {
    ImGui::Text("Search Algorithms");
    ImGui::Separator();
    
    // Algorithm selection
    if (ImGui::Combo("Algorithm", reinterpret_cast<int*>(&m_currentAlgorithm), 
                     s_algorithmNames, static_cast<int>(SearchAlgorithm::JumpSearch) + 1)) {
        ResetSearch();
    }
    
    ImGui::Spacing();
    
    // Array controls
    if (ImGui::SliderInt("Array Size", &m_arraySize, 5, 100)) {
        if (m_autoGenerate) {
            GenerateArray();
            SortArray();
            ResetSearch();
        }
    }
    
    if (ImGui::SliderInt("Target Value", &m_targetValue, 1, 100)) {
        ResetSearch();
    }
    
    ImGui::Checkbox("Auto-generate array", &m_autoGenerate);
    
    if (ImGui::Button("Generate New Array")) {
        GenerateArray();
        SortArray();
        ResetSearch();
    }
    
    ImGui::Spacing();
    
    // Search controls
    ImGui::Text("Search Controls:");
    if (ImGui::Button("Start Search") && m_targetValue != -1) {
        // Record start time for performance tracking
        m_searchStartTime = std::chrono::steady_clock::now();
        m_totalComparisons = 0;
        
        ResetSearch();
        StartSearch();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Step") && !m_isRunning && !m_isComplete && !m_steps.empty()) {
        StepSearch();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        ResetSearch();
    }
    
    ImGui::SliderFloat("Animation Speed", &m_animationSpeed, 0.1f, 5.0f);
    
    ImGui::Spacing();
    
    // Instructions
    ImGui::Text("Instructions:");
    ImGui::BulletText("Select algorithm and set target value");
    ImGui::BulletText("Click 'Start Search' to begin animation");
    ImGui::BulletText("Use 'Step' for manual progression");
    ImGui::BulletText("Red: Current position, Green: Search range");
    
    ImGui::Spacing();
    ImGui::Checkbox("Show Pseudocode", &m_showCode);
    
    if (m_showCode) {
        ImGui::Spacing();
        ImGui::Text("Pseudocode:");
        ImGui::Separator();
        switch (m_currentAlgorithm) {
            case SearchAlgorithm::BinarySearch:
                ImGui::TextWrapped(
                    "1. Set low = 0, high = n-1\n"
                    "2. While low <= high:\n"
                    "   a. mid = (low + high) / 2\n"
                    "   b. If arr[mid] == target: return mid\n"
                    "   c. If arr[mid] < target: low = mid + 1\n"
                    "   d. Else: high = mid - 1\n"
                    "3. Return -1 (not found)"
                );
                break;
            case SearchAlgorithm::LinearSearch:
                ImGui::TextWrapped(
                    "1. For i = 0 to n-1:\n"
                    "   a. If arr[i] == target: return i\n"
                    "2. Return -1 (not found)"
                );
                break;
            case SearchAlgorithm::InterpolationSearch:
                ImGui::TextWrapped(
                    "1. Set low = 0, high = n-1\n"
                    "2. While low <= high and target in range:\n"
                    "   a. pos = low + ((target - arr[low]) * \n"
                    "       (high - low)) / (arr[high] - arr[low])\n"
                    "   b. If arr[pos] == target: return pos\n"
                    "   c. If arr[pos] < target: low = pos + 1\n"
                    "   d. Else: high = pos - 1\n"
                    "3. Return -1 (not found)"
                );
                break;
            default:
                ImGui::TextWrapped("Select algorithm to see pseudocode");
                break;
        }
    }
}

void SearchVisualizer::RenderVisualization() {
    if (m_array.empty()) return;
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Calculate bar dimensions
    float barWidth = (canvasSize.x - 40) / m_array.size();
    float maxHeight = canvasSize.y - 80;
    int maxValue = *std::max_element(m_array.begin(), m_array.end());
    
    // Get current step info
    SearchStep currentStep;
    if (m_currentStep < m_steps.size()) {
        currentStep = m_steps[m_currentStep];
    }
    
    // Draw array bars
    for (size_t i = 0; i < m_array.size(); ++i) {
        float x = canvasPos.x + 20 + i * barWidth;
        float barHeight = (static_cast<float>(m_array[i]) / maxValue) * maxHeight;
        float y = canvasPos.y + canvasSize.y - barHeight - 20;
        
        // Determine bar color
        ImU32 color = IM_COL32(100, 100, 100, 255); // Default gray
        
        if (currentStep.foundIndex == static_cast<int>(i)) {
            color = IM_COL32(0, 255, 0, 255); // Green for found
        } else if (currentStep.currentIndex == static_cast<int>(i)) {
            color = IM_COL32(255, 0, 0, 255); // Red for current
        } else if (static_cast<int>(i) >= currentStep.lowIndex && 
                   static_cast<int>(i) <= currentStep.highIndex && 
                   currentStep.lowIndex != -1) {
            color = IM_COL32(100, 200, 100, 200); // Light green for search range
        } else if (std::find(currentStep.searchRange.begin(), currentStep.searchRange.end(), i) != currentStep.searchRange.end()) {
            color = IM_COL32(255, 255, 0, 200); // Yellow for special range
        }
        
        // Draw bar
        drawList->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + barWidth - 2, canvasPos.y + canvasSize.y - 20),
            color
        );
        
        // Draw value
        std::string valueStr = std::to_string(m_array[i]);
        ImVec2 textSize = ImGui::CalcTextSize(valueStr.c_str());
        if (barWidth > textSize.x) {
            drawList->AddText(
                ImVec2(x + (barWidth - textSize.x) / 2, y - textSize.y - 5),
                IM_COL32(255, 255, 255, 255),
                valueStr.c_str()
            );
        }
        
        // Draw index
        std::string indexStr = std::to_string(i);
        ImVec2 indexSize = ImGui::CalcTextSize(indexStr.c_str());
        drawList->AddText(
            ImVec2(x + (barWidth - indexSize.x) / 2, canvasPos.y + canvasSize.y - 15),
            IM_COL32(200, 200, 200, 255),
            indexStr.c_str()
        );
    }
    
    // Draw target indicator
    std::string targetStr = fmt::format("Target: {}", m_targetValue);
    drawList->AddText(
        ImVec2(canvasPos.x + 10, canvasPos.y + 10),
        IM_COL32(255, 255, 255, 255),
        targetStr.c_str()
    );
    
    // Draw current step description
    if (m_currentStep < m_steps.size()) {
        ImVec2 textPos = ImVec2(canvasPos.x + 10, canvasPos.y + 35);
        drawList->AddText(textPos, IM_COL32(255, 255, 0, 255), 
                         m_steps[m_currentStep].description.c_str());
    }
    
    ImGui::Dummy(canvasSize);
}

void SearchVisualizer::RenderStatistics() {
    // Pulsing statistics header
    extern Application* g_application;
    if (g_application) {
        g_application->PushPulsingTextStyle(0.15f);
        ImGui::Text("Search Statistics");
        g_application->PopPulsingTextStyle();
        g_application->DrawAnimatedSeparator();
    } else {
        ImGui::Text("Search Statistics");
        ImGui::Separator();
    }
    
    ImGui::Text("Array Size: %d", static_cast<int>(m_array.size()));
    ImGui::Text("Target: %d", m_targetValue);
    ImGui::Text("Comparisons: %d", m_comparisons);
    ImGui::Text("Max Comparisons: %d", m_maxComparisons);
    
    if (m_searchTime > 0) {
        ImGui::Text("Search Time: %.3f ms", m_searchTime);
    }
    
    if (m_isComplete) {
        if (m_targetFound) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Target FOUND at index %d!", m_foundIndex);
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Target NOT FOUND");
        }
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

void SearchVisualizer::Update() {
    if (m_isRunning && !m_isComplete) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStepTime);
        
        if (elapsed.count() >= (1000 / m_animationSpeed)) {
            StepSearch();
            m_lastStepTime = now;
        }
    }
}

void SearchVisualizer::GenerateArray() {
    m_array.clear();
    m_array.reserve(m_arraySize);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (int i = 0; i < m_arraySize; ++i) {
        m_array.push_back(dis(gen));
    }
}

void SearchVisualizer::SortArray() {
    // Most search algorithms require sorted array
    if (m_currentAlgorithm != SearchAlgorithm::LinearSearch) {
        std::sort(m_array.begin(), m_array.end());
    }
}

void SearchVisualizer::StartSearch() {
    ResetSearch();
    
    m_startTime = std::chrono::high_resolution_clock::now();
    m_lastStepTime = m_startTime;
    
    // Calculate theoretical maximum comparisons
    switch (m_currentAlgorithm) {
        case SearchAlgorithm::LinearSearch:
            m_maxComparisons = m_arraySize;
            LinearSearch(m_targetValue);
            break;
        case SearchAlgorithm::BinarySearch:
            m_maxComparisons = static_cast<int>(std::ceil(std::log2(m_arraySize)));
            BinarySearch(m_targetValue);
            break;
        case SearchAlgorithm::InterpolationSearch:
            m_maxComparisons = static_cast<int>(std::ceil(std::log2(std::log2(m_arraySize))));
            InterpolationSearch(m_targetValue);
            break;
        case SearchAlgorithm::ExponentialSearch:
            m_maxComparisons = static_cast<int>(std::ceil(std::log2(m_arraySize)));
            ExponentialSearch(m_targetValue);
            break;
        case SearchAlgorithm::JumpSearch:
            m_maxComparisons = static_cast<int>(std::sqrt(m_arraySize)) + m_arraySize / static_cast<int>(std::sqrt(m_arraySize));
            JumpSearch(m_targetValue);
            break;
    }
    
    if (!m_steps.empty()) {
        m_isRunning = true;
    }
}

void SearchVisualizer::StepSearch() {
    if (m_currentStep < m_steps.size()) {
        const auto& step = m_steps[m_currentStep];
        
        if (step.isComparison) {
            m_comparisons++;
        }
        
        if (step.isFound) {
            m_targetFound = true;
            m_foundIndex = step.foundIndex;
            m_isComplete = true;
            m_isRunning = false;
            m_endTime = std::chrono::high_resolution_clock::now();
            m_searchTime = std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count();
        }
        
        PlayStepSound(step.isComparison, step.isFound);
        
        m_currentStep++;
        
        if (m_currentStep >= m_steps.size()) {
            m_isComplete = true;
            m_isRunning = false;
            if (!m_targetFound) {
                m_endTime = std::chrono::high_resolution_clock::now();
                m_searchTime = std::chrono::duration<double, std::milli>(m_endTime - m_startTime).count();
            }
        }
    }
}

void SearchVisualizer::ResetSearch() {
    m_steps.clear();
    m_currentStep = 0;
    m_isRunning = false;
    m_isComplete = false;
    m_targetFound = false;
    m_foundIndex = -1;
    m_comparisons = 0;
    m_searchTime = 0.0;
}

void SearchVisualizer::LinearSearch(int target) {
    for (size_t i = 0; i < m_array.size(); i++) {
        SearchStep step;
        step.description = fmt::format("Checking element at index {}: {}", i, m_array[i]);
        step.currentIndex = static_cast<int>(i);
        step.isComparison = true;
        m_steps.push_back(step);
        
        m_totalComparisons++; // Track comparisons
        
        if (m_array[i] == target) {
            step.description = fmt::format("Found target {} at index {}", target, i);
            step.foundIndex = static_cast<int>(i);
            step.isFound = true;
            m_steps.push_back(step);
            if (m_audioManager) m_audioManager->PlayComparisonSound();
            CompleteSearch(i);
            return;
        }
    }
    
    // Not found
    SearchStep step;
    step.description = "Target not found in array";
    m_steps.push_back(step);
    CompleteSearch(-1);
}

void SearchVisualizer::BinarySearch(int target) {
    int low = 0;
    int high = m_array.size() - 1;
    
    RecordStep("Starting Binary Search", -1, low, high, false);
    
    while (low <= high) {
        int mid = low + (high - low) / 2;
        
        RecordStep(fmt::format("Checking middle element at index {} (value: {})", mid, m_array[mid]), 
                  mid, low, high, true);
        
        if (m_array[mid] == target) {
            RecordStep(fmt::format("Found target {} at index {}!", target, mid), mid, low, high, false);
            SearchStep step;
            step.description = "Search completed successfully!";
            step.foundIndex = mid;
            step.isFound = true;
            m_steps.push_back(step);
            CompleteSearch(mid);
            return;
        }
        
        if (m_array[mid] < target) {
            RecordStep(fmt::format("Target is greater than {}, searching right half", m_array[mid]), 
                      mid, low, high, false);
            low = mid + 1;
        } else {
            RecordStep(fmt::format("Target is less than {}, searching left half", m_array[mid]), 
                      mid, low, high, false);
            high = mid - 1;
        }
    }
    
    // Not found
    if (m_audioManager) m_audioManager->PlayComparisonSound();
    CompleteSearch(-1);
}

void SearchVisualizer::InterpolationSearch(int target) {
    int low = 0;
    int high = m_array.size() - 1;
    
    RecordStep("Starting Interpolation Search", -1, low, high, false);
    
    while (low <= high && target >= m_array[low] && target <= m_array[high]) {
        if (low == high) {
            RecordStep(fmt::format("Single element remaining at index {}", low), low, low, high, true);
            if (m_array[low] == target) {
                SearchStep step;
                step.description = "Found target!";
                step.foundIndex = low;
                step.isFound = true;
                m_steps.push_back(step);
                CompleteSearch(low);
            }
            return;
        }
        
        // Interpolation formula
        int pos = low + ((double)(target - m_array[low]) / (m_array[high] - m_array[low])) * (high - low);
        pos = std::max(low, std::min(pos, high)); // Ensure pos is in bounds
        
        RecordStep(fmt::format("Interpolating position: {} (value: {})", pos, m_array[pos]), 
                  pos, low, high, true);
        
        if (m_array[pos] == target) {
            RecordStep(fmt::format("Found target {} at index {}!", target, pos), pos, low, high, false);
            SearchStep step;
            step.description = "Search completed successfully!";
            step.foundIndex = pos;
            step.isFound = true;
            m_steps.push_back(step);
            CompleteSearch(pos);
            return;
        }
        
        if (m_array[pos] < target) {
            RecordStep(fmt::format("Target is greater than {}, searching right", m_array[pos]), 
                      pos, low, high, false);
            low = pos + 1;
        } else {
            RecordStep(fmt::format("Target is less than {}, searching left", m_array[pos]), 
                      pos, low, high, false);
            high = pos - 1;
        }
    }
    
    // Not found
    if (m_audioManager) m_audioManager->PlayComparisonSound();
    CompleteSearch(-1);
}

void SearchVisualizer::ExponentialSearch(int target) {
    RecordStep("Starting Exponential Search", -1, -1, -1, false);
    
    if (m_array[0] == target) {
        RecordStep("Found target at index 0!", 0, -1, -1, false);
        SearchStep step;
        step.description = "Search completed successfully!";
        step.foundIndex = 0;
        step.isFound = true;
        m_steps.push_back(step);
        CompleteSearch(0);
        return;
    }
    
    // Find range for binary search
    int bound = 1;
    while (bound < static_cast<int>(m_array.size()) && m_array[bound] < target) {
        RecordStep(fmt::format("Checking bound at index {} (value: {})", bound, m_array[bound]), 
                  bound, -1, -1, true);
        bound *= 2;
    }
    
    // Binary search in found range
    int low = bound / 2;
    int high = std::min(bound, static_cast<int>(m_array.size()) - 1);
    
    RecordStep(fmt::format("Found range [{}..{}], starting binary search", low, high), 
              -1, low, high, false);
    
    while (low <= high) {
        int mid = low + (high - low) / 2;
        
        RecordStep(fmt::format("Binary search: checking index {} (value: {})", mid, m_array[mid]), 
                  mid, low, high, true);
        
        if (m_array[mid] == target) {
            RecordStep(fmt::format("Found target {} at index {}!", target, mid), mid, low, high, false);
            SearchStep step;
            step.description = "Search completed successfully!";
            step.foundIndex = mid;
            step.isFound = true;
            m_steps.push_back(step);
            CompleteSearch(mid);
            return;
        }
        
        if (m_array[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    
    // Not found
    if (m_audioManager) m_audioManager->PlayComparisonSound();
    CompleteSearch(-1);
}

void SearchVisualizer::JumpSearch(int target) {
    int step = static_cast<int>(std::sqrt(m_array.size()));
    int prev = 0;
    
    RecordStep(fmt::format("Starting Jump Search with step size {}", step), -1, -1, -1, false);
    
    // Find block containing target
    while (m_array[std::min(step, static_cast<int>(m_array.size())) - 1] < target) {
        RecordStep(fmt::format("Jumping to index {} (value: {})", 
                  std::min(step, static_cast<int>(m_array.size())) - 1, 
                  m_array[std::min(step, static_cast<int>(m_array.size())) - 1]), 
                  std::min(step, static_cast<int>(m_array.size())) - 1, -1, -1, true);
        
        prev = step;
        step += static_cast<int>(std::sqrt(m_array.size()));
        
        if (prev >= static_cast<int>(m_array.size())) {
            RecordStep("Target not found in array", -1, -1, -1, false);
            CompleteSearch(-1);
            return;
        }
    }
    
    // Linear search in identified block
    RecordStep(fmt::format("Found potential block [{}..{}], linear searching", 
              prev, std::min(step, static_cast<int>(m_array.size())) - 1), 
              -1, prev, std::min(step, static_cast<int>(m_array.size())) - 1, false);
    
    while (m_array[prev] < target) {
        RecordStep(fmt::format("Linear search: checking index {} (value: {})", prev, m_array[prev]), 
                  prev, -1, -1, true);
        prev++;
        
        if (prev == std::min(step, static_cast<int>(m_array.size()))) {
            RecordStep("Target not found in array", -1, -1, -1, false);
            CompleteSearch(-1);
            return;
        }
    }
    
    if (m_array[prev] == target) {
        RecordStep(fmt::format("Found target {} at index {}!", target, prev), prev, -1, -1, false);
        SearchStep step;
        step.description = "Search completed successfully!";
        step.foundIndex = prev;
        step.isFound = true;
        m_steps.push_back(step);
        CompleteSearch(prev);
    } else {
        RecordStep("Target not found in array", -1, -1, -1, false);
        CompleteSearch(-1);
    }
}

void SearchVisualizer::RecordStep(const std::string& description, int currentIndex, 
                                 int low, int high, bool isComparison) {
    SearchStep step;
    step.description = description;
    step.currentIndex = currentIndex;
    step.lowIndex = low;
    step.highIndex = high;
    step.isComparison = isComparison;
    m_steps.push_back(step);
}

void SearchVisualizer::PlayStepSound(bool isComparison, bool isFound) {
    if (m_audioManager) {
        if (isFound) {
            m_audioManager->PlaySearchSuccess();
        } else if (isComparison) {
            m_audioManager->PlaySearchComparison();
        } else {
            m_audioManager->PlaySearchStep();
        }
    }
}

void SearchVisualizer::CompleteSearch([[maybe_unused]] int foundIndex) {
    // Report performance when search completes
    if (m_performanceCallback) {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_searchStartTime);
        
        std::string algorithmName = GetAlgorithmName(m_currentAlgorithm);
        m_performanceCallback(algorithmName, duration.count(), m_totalComparisons, 0);
    }
}

std::string SearchVisualizer::GetAlgorithmName(SearchAlgorithm algorithm) const {
    switch (algorithm) {
        case SearchAlgorithm::LinearSearch: return "Linear Search";
        case SearchAlgorithm::BinarySearch: return "Binary Search";
        case SearchAlgorithm::InterpolationSearch: return "Interpolation Search";
        case SearchAlgorithm::ExponentialSearch: return "Exponential Search";
        case SearchAlgorithm::JumpSearch: return "Jump Search";
        default: return "Unknown Search";
    }
}

} // namespace AlgorithmVisualizer