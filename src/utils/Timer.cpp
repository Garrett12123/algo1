#include "utils/Timer.h"

namespace AlgorithmVisualizer {

void Timer::Start() {
    m_startTime = std::chrono::steady_clock::now();
    m_running = true;
}

void Timer::Stop() {
    if (m_running) {
        m_endTime = std::chrono::steady_clock::now();
        m_running = false;
    }
}

void Timer::Reset() {
    m_startTime = {};
    m_endTime = {};
    m_running = false;
}

std::chrono::milliseconds Timer::GetElapsed() const {
    if (m_running) {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime);
    } else {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime);
    }
}

} // namespace AlgorithmVisualizer 