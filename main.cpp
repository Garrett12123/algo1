#include <algorithm>
#include <fmt/core.h>
#include <fmt/color.h>
#include <iostream>
#include <vector>
#include <ranges>

auto main() -> int {
    // Modern C++20 Hello World with fmt library
    fmt::print(fg(fmt::color::green), "🚀 Hello, Modern C++20!\n");
    fmt::print(fg(fmt::color::cyan), "Welcome to the Modern C++ Algorithms Project!\n\n");
    
    // Demonstrate some C++20 features
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    fmt::print("Original numbers: ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");
    
    // C++20 ranges and views
    auto even_numbers = numbers 
                       | std::views::filter([](int n) { return n % 2 == 0; })
                       | std::views::transform([](int n) { return n * n; });
    
    fmt::print("Even numbers squared: ");
    for (const auto& num : even_numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");
    
    fmt::print(fg(fmt::color::yellow), "\n✨ Ready for algorithm development!\n");
    
    return 0;
} 