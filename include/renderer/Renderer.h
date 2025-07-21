#pragma once

namespace AlgorithmVisualizer {

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;
    
    void Initialize();
    void Render();
    void Shutdown();
};

} // namespace AlgorithmVisualizer 