#include "Application.h"
#include <fmt/core.h>
#include <fmt/color.h>
#include <iostream>
#include <exception>

int main() {
    try {
        fmt::print(fg(fmt::color::cyan), "Modern C++ Algorithm Visualizer Starting...\n");
        
        auto app = std::make_unique<AlgorithmVisualizer::Application>();
        
        if (!app->Initialize()) {
            fmt::print(fg(fmt::color::red), "Failed to initialize application\n");
            return -1;
        }
        
        fmt::print(fg(fmt::color::green), "Application initialized successfully\n");
        
        app->Run();
        app->Shutdown();
        
        fmt::print(fg(fmt::color::cyan), "Application closed successfully\n");
        
    } catch (const std::exception& e) {
        fmt::print(fg(fmt::color::red), "Exception caught: {}\n", e.what());
        return -1;
    } catch (...) {
        fmt::print(fg(fmt::color::red), "Unknown exception caught\n");
        return -1;
    }
    
    return 0;
} 