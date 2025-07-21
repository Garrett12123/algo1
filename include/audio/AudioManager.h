#pragma once

#include <memory>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>

namespace AlgorithmVisualizer {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool Initialize();
    void Shutdown();
    
    // Sound generation and playback
    void PlayComparisonSound(float pitch = 1.0f);
    void PlaySwapSound();
    void PlayCompletionSound();
    void PlayErrorSound();
    
    // Pathfinding sounds
    void PlayExploreSound(float pitch = 1.0f);
    void PlayFrontierSound(float pitch = 1.0f);
    void PlayVisitedSound(float pitch = 1.0f);
    void PlayPathFoundSound();
    void PlayNoPathSound();
    
    // Graph algorithm sounds
    void PlayNodeSelectSound();
    void PlayEdgeAddSound();
    void PlayMSTCompleteSound();
    
    // Tone generation for values
    void PlayValueTone(int value, int maxValue, float duration = 0.1f);
    
    // Volume and settings
    void SetMasterVolume(float volume); // 0.0f to 1.0f
    void SetEnabled(bool enabled);
    
    [[nodiscard]] bool IsEnabled() const { return m_enabled; }
    [[nodiscard]] float GetMasterVolume() const { return m_masterVolume; }
    
    // Update (call this regularly to clean up finished sounds)
    void Update();

    // Pathfinding audio
    void PlayPathfindingExplore();
    void PlayPathfindingFrontier();
    void PlayPathfindingVisited();
    void PlayPathfindingPathFound();
    
    // Graph audio
    void PlayGraphExplore();
    void PlayGraphEdgeFound();
    void PlayGraphNodeProcessed();
    
    // Search algorithm audio
    void PlaySearchStep();
    void PlaySearchComparison();
    void PlaySearchSuccess();
    void PlaySearchFail();
    
    // Tree algorithm audio
    void PlayTreeInsert();
    void PlayTreeDelete();
    void PlayTreeRotation();
    void PlayTreeTraversal();
    void PlayTreeComparison();

private:
    // OpenAL context
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    
    // Sound buffers and sources
    ALuint m_comparisonBuffer = 0;
    ALuint m_swapBuffer = 0;
    ALuint m_completionBuffer = 0;
    ALuint m_errorBuffer = 0;
    
    std::vector<ALuint> m_sources;
    static constexpr size_t MAX_SOURCES = 32;
    
    // Settings
    float m_masterVolume = 0.5f;
    bool m_enabled = true;
    bool m_initialized = false;
    
    // Helper methods
    ALuint GetAvailableSource();
    void GenerateToneBuffer(ALuint buffer, float frequency, float duration, float amplitude = 0.3f);
    void GenerateBeepBuffer(ALuint buffer, float frequency, float duration);
    void GenerateClickBuffer(ALuint buffer);
    void GenerateSuccessBuffer(ALuint buffer);
    
    // Cleanup
    void CleanupBuffers();
    void CleanupSources();
    
    // Error checking
    bool CheckALError(const char* operation);
    bool CheckALCError(const char* operation);
};

} // namespace AlgorithmVisualizer 