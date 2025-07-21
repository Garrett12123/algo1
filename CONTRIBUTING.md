# Contributing to Algo1

Thank you for your interest in contributing to **Algo1**! We welcome contributions from the community and are excited to see what you'll build.

## 🎯 Ways to Contribute

- **New Algorithms**: Add additional sorting, searching, pathfinding, graph, or tree algorithms
- **Visualization Improvements**: Enhance graphics, animations, and visual feedback
- **Performance Optimizations**: Improve speed and memory efficiency
- **Educational Content**: Add detailed algorithm explanations and complexity analysis
- **Bug Fixes**: Fix issues and improve stability
- **Documentation**: Improve README, code comments, and documentation
- **Platform Support**: Add support for additional operating systems
- **Testing**: Add unit tests and integration tests

## 🚀 Getting Started

### Prerequisites

- **C++20 Compiler**: GCC 10+, Clang 12+, or MSVC 2019+
- **CMake**: 3.20 or higher
- **vcpkg**: Microsoft's C++ package manager
- **Git**: For version control

### Setting Up Your Development Environment

1. **Fork the repository** on GitHub
2. **Clone your fork**:
   ```bash
   git clone https://github.com/Garrett12123/algo1.git
   cd algo1
   ```

3. **Set up vcpkg** (if not already installed):
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg && ./bootstrap-vcpkg.sh
   export VCPKG_ROOT=/path/to/vcpkg
   ```

4. **Build the project**:
   ```bash
   cmake --preset=default
   cmake --build --preset=default
   ```

5. **Run the application**:
   ```bash
   ./build/default/algo1
   ```

## 📝 Development Guidelines

### Code Style

- **C++20 Standards**: Use modern C++20 features (ranges, concepts, etc.)
- **Naming Conventions**:
  - Classes: `PascalCase` (e.g., `SortingVisualizer`)
  - Functions/Methods: `camelCase` (e.g., `runAlgorithm`)
  - Variables: `camelCase` (e.g., `arraySize`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_ARRAY_SIZE`)
  - Private members: `m_` prefix (e.g., `m_audioManager`)

- **Formatting**:
  - Use 4 spaces for indentation (no tabs)
  - Maximum line length: 100 characters
  - Use consistent brace placement
  - Include proper spacing around operators

### File Organization

```
algo1/
├── src/
│   ├── algorithms/          # Algorithm implementations
│   ├── audio/              # Audio management
│   ├── renderer/           # Graphics and rendering
│   └── utils/              # Utility classes
├── include/
│   ├── algorithms/         # Algorithm headers
│   ├── audio/             # Audio headers
│   ├── renderer/          # Renderer headers
│   └── utils/             # Utility headers
└── resources/             # Assets (sounds, textures, etc.)
```

### Adding New Algorithms

When adding a new algorithm:

1. **Create the algorithm class** in the appropriate visualizer (e.g., `SortingVisualizer`)
2. **Add visualization logic** with appropriate colors and animations
3. **Include audio feedback** for key operations (comparisons, swaps, etc.)
4. **Add performance metrics** (comparisons, swaps, time complexity)
5. **Update the README** with algorithm information
6. **Add appropriate comments** explaining the algorithm logic

### Example: Adding a New Sorting Algorithm

```cpp
// In SortingVisualizer.h
void cocktailSort();

// In SortingVisualizer.cpp
void SortingVisualizer::cocktailSort() {
    m_stats = AlgorithmStats{};
    auto start = std::chrono::high_resolution_clock::now();
    
    bool swapped = true;
    int start_idx = 0;
    int end_idx = m_array.size() - 1;
    
    while (swapped) {
        swapped = false;
        
        // Forward pass
        for (int i = start_idx; i < end_idx; ++i) {
            m_stats.comparisons++;
            if (m_array[i] > m_array[i + 1]) {
                std::swap(m_array[i], m_array[i + 1]);
                m_stats.swaps++;
                playSwapSound();
                swapped = true;
                
                // Update visualization
                updateVisualization();
                if (m_shouldStep) waitForStep();
            }
        }
        end_idx--;
        
        if (!swapped) break;
        
        // Backward pass
        for (int i = end_idx; i > start_idx; --i) {
            m_stats.comparisons++;
            if (m_array[i] < m_array[i - 1]) {
                std::swap(m_array[i], m_array[i - 1]);
                m_stats.swaps++;
                playSwapSound();
                swapped = true;
                
                updateVisualization();
                if (m_shouldStep) waitForStep();
            }
        }
        start_idx++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    m_stats.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    playCompletionSound();
}
```

## 🐛 Bug Reports

When reporting bugs:

1. **Check existing issues** to avoid duplicates
2. **Use the bug report template** (if available)
3. **Include**:
   - Operating system and version
   - Compiler and version
   - CMake version
   - Steps to reproduce
   - Expected vs actual behavior
   - Screenshots/videos if applicable
   - Console output/error messages

## 💡 Feature Requests

For new features:

1. **Check existing feature requests** to avoid duplicates
2. **Use the feature request template** (if available)
3. **Describe**:
   - The problem you're trying to solve
   - Proposed solution
   - Alternative solutions considered
   - How it benefits the project

## 🔄 Pull Request Process

1. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes** following the development guidelines

3. **Test your changes**:
   ```bash
   cmake --build --preset=default
   ./build/default/algo1
   ```

4. **Commit your changes**:
   ```bash
   git commit -m "Add cocktail sort algorithm with visualization"
   ```

5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a Pull Request** on GitHub

### Pull Request Guidelines

- **Descriptive title**: Clearly describe what the PR does
- **Detailed description**: Explain the changes and why they were made
- **Link related issues**: Reference any issues this PR addresses
- **Screenshots/videos**: Include visual proof of changes (especially for UI/UX)
- **Testing**: Describe how you tested the changes
- **Breaking changes**: Clearly document any breaking changes

### Code Review Process

1. **Automated checks** must pass (build, formatting, etc.)
2. **At least one maintainer** must review and approve
3. **Address feedback** promptly and respectfully
4. **Squash commits** if requested before merging

## 🧪 Testing

Currently, the project focuses on visual testing through the GUI. When contributing:

- **Manual testing**: Test your changes thoroughly in the application
- **Cross-platform testing**: Test on different operating systems if possible
- **Performance testing**: Ensure your changes don't significantly impact performance
- **Edge cases**: Test with various array sizes, data patterns, and configurations

## 📚 Documentation

When contributing:

- **Update README.md** if adding new features or algorithms
- **Add inline comments** for complex logic
- **Document public APIs** with clear parameter descriptions
- **Update algorithm complexity** information when applicable

## 💬 Communication

- **GitHub Issues**: For bug reports and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Pull Request Comments**: For code-specific discussions

## 🎉 Recognition

Contributors will be recognized in:

- **README.md**: Listed in the contributors section
- **Release Notes**: Major contributions highlighted in releases
- **GitHub**: Contributor badge and statistics

## 📋 Checklist for Contributors

Before submitting a Pull Request:

- [ ] Code follows the project's style guidelines
- [ ] Changes have been tested locally
- [ ] Documentation has been updated (if applicable)
- [ ] Commit messages are clear and descriptive
- [ ] No unnecessary files are included
- [ ] Performance impact has been considered
- [ ] Accessibility guidelines are followed (for UI changes)

## ❓ Questions?

If you have questions about contributing:

1. Check the [README.md](README.md) for basic information
2. Search existing [GitHub Issues](https://github.com/YOUR_REPO/algo1/issues)
3. Create a new issue with the "question" label
4. Join discussions in [GitHub Discussions](https://github.com/YOUR_REPO/algo1/discussions)

---

Thank you for contributing to Algo1! Your efforts help make algorithm education more accessible and engaging for everyone. 🚀

*Happy coding!* 💻✨ 