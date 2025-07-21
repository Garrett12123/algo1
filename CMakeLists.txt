cmake_minimum_required(VERSION 3.20)

# Project definition
project(algo1 
    VERSION 2.0.0
    DESCRIPTION "Algo1 - Retro Algorithm Visualizer with GUI"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Platform-specific definitions
if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)  # Silence OpenGL deprecation warnings on macOS
endif()

# Debug/Release configurations
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Check if we're using vcpkg
if(DEFINED ENV{VCPKG_ROOT} OR DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using vcpkg for dependencies")
    # Find packages (vcpkg integration)
    find_package(fmt CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(implot CONFIG REQUIRED)
    find_package(OpenAL CONFIG REQUIRED)
else()
    message(STATUS "Using system packages (vcpkg not found)")
    # Try to find system packages
    find_package(PkgConfig QUIET)
    
    # Find fmt
    find_package(fmt QUIET)
    if(NOT fmt_FOUND)
        message(WARNING "fmt not found - you may need to install it manually")
        # Create a simple fallback
        add_library(fmt_fallback INTERFACE)
        target_compile_definitions(fmt_fallback INTERFACE FMT_HEADER_ONLY=1)
        add_library(fmt::fmt ALIAS fmt_fallback)
    endif()
    
    # For GLFW, try system installation
    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GLFW REQUIRED glfw3)
        add_library(glfw INTERFACE)
        target_link_libraries(glfw INTERFACE ${GLFW_LIBRARIES})
        target_include_directories(glfw INTERFACE ${GLFW_INCLUDE_DIRS})
    endif()
    
    # For OpenAL
    find_package(OpenAL QUIET)
    if(NOT OpenAL_FOUND)
        message(WARNING "OpenAL not found - audio features may not work")
        # Create dummy target
        add_library(OpenAL_dummy INTERFACE)
        add_library(OpenAL::OpenAL ALIAS OpenAL_dummy)
    endif()
    
    # ImGui and ImPlot are harder to find as system packages
    # For now, we'll require vcpkg for these
    message(FATAL_ERROR "ImGui and ImPlot require vcpkg. Please install vcpkg and set VCPKG_ROOT environment variable.")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/algorithms/SortingVisualizer.cpp
    src/algorithms/PathfindingVisualizer.cpp
    src/algorithms/GraphVisualizer.cpp
    src/algorithms/SearchVisualizer.cpp
    src/algorithms/TreeVisualizer.cpp
    src/renderer/Renderer.cpp
    src/utils/Timer.cpp
    src/audio/AudioManager.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries (fmt excluded to avoid duplicate with transitive dependencies)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui::imgui
    implot::implot
    fmt::fmt-header-only
    glfw
    OpenGL::GL
    OpenAL::OpenAL
)

# Resources directory removed - using enhanced default fonts instead

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin) 