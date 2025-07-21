#pragma once

#include <chrono>

namespace AlgorithmVisualizer {

class Timer {
public:
    Timer() = default;
    ~Timer() = default;
    
    void Start();
    void Stop();
    void Reset();
    
    [[nodiscard]] std::chrono::milliseconds GetElapsed() const;
    [[nodiscard]] bool IsRunning() const { return m_running; }

private:
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_endTime;
    bool m_running = false;
};

} // namespace AlgorithmVisualizer 