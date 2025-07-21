#include "audio/AudioManager.h"
#include <fmt/core.h>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace AlgorithmVisualizer {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Open the default device
    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
        fmt::print("Failed to open OpenAL device\n");
        return false;
    }
    
    // Create context
    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        fmt::print("Failed to create OpenAL context\n");
        alcCloseDevice(m_device);
        m_device = nullptr;
        return false;
    }
    
    // Make context current
    if (!alcMakeContextCurrent(m_context)) {
        fmt::print("Failed to make OpenAL context current\n");
        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
        m_context = nullptr;
        m_device = nullptr;
        return false;
    }
    
    // Generate sound buffers
    alGenBuffers(1, &m_comparisonBuffer);
    alGenBuffers(1, &m_swapBuffer);
    alGenBuffers(1, &m_completionBuffer);
    alGenBuffers(1, &m_errorBuffer);
    
    if (!CheckALError("Generate buffers")) {
        Shutdown();
        return false;
    }
    
    // Generate sound sources
    m_sources.resize(MAX_SOURCES);
    alGenSources(static_cast<ALsizei>(MAX_SOURCES), m_sources.data());
    
    if (!CheckALError("Generate sources")) {
        Shutdown();
        return false;
    }
    
    // Pre-generate sounds
    GenerateBeepBuffer(m_comparisonBuffer, 800.0f, 0.1f);
    GenerateClickBuffer(m_swapBuffer);
    GenerateSuccessBuffer(m_completionBuffer);
    GenerateBeepBuffer(m_errorBuffer, 200.0f, 0.3f);
    
    // Set listener properties
    alListener3f(AL_POSITION, 0.0f, 0.0f, 1.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, listenerOri);
    
    m_initialized = true;
    fmt::print("AudioManager initialized successfully\n");
    return true;
}

void AudioManager::Shutdown() {
    if (!m_initialized) {
        return;
    }
    
    CleanupSources();
    CleanupBuffers();
    
    if (m_context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_context);
        m_context = nullptr;
    }
    
    if (m_device) {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
    
    m_initialized = false;
    fmt::print("AudioManager shut down\n");
}

void AudioManager::PlayComparisonSound(float pitch) {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, pitch);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.3f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_comparisonBuffer);
    
    alSourcePlay(source);
    CheckALError("Play comparison sound");
}

void AudioManager::PlaySwapSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.5f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_swapBuffer);
    
    alSourcePlay(source);
    CheckALError("Play swap sound");
}

void AudioManager::PlayCompletionSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.7f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_completionBuffer);
    
    alSourcePlay(source);
    CheckALError("Play completion sound");
}

void AudioManager::PlayErrorSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.4f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_errorBuffer);
    
    alSourcePlay(source);
    CheckALError("Play error sound");
}

void AudioManager::PlayValueTone(int value, int maxValue, float duration) {
    if (!m_enabled || !m_initialized) return;
    
    // Map value to frequency range (200Hz to 1000Hz)
    float normalizedValue = static_cast<float>(value) / static_cast<float>(maxValue);
    float frequency = 200.0f + (normalizedValue * 800.0f);
    
    // Create a temporary buffer for this tone
    ALuint tempBuffer;
    alGenBuffers(1, &tempBuffer);
    GenerateToneBuffer(tempBuffer, frequency, duration, 0.2f);
    
    ALuint source = GetAvailableSource();
    if (source == 0) {
        alDeleteBuffers(1, &tempBuffer);
        return;
    }
    
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.3f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, tempBuffer);
    
    alSourcePlay(source);
    
    // Note: In a production app, you'd want to track and clean up temp buffers
    // For simplicity, we'll let them accumulate and clean up on shutdown
    CheckALError("Play value tone");
}

void AudioManager::SetMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

void AudioManager::SetEnabled(bool enabled) {
    m_enabled = enabled;
    
    if (!enabled) {
        // Stop all currently playing sounds
        for (ALuint source : m_sources) {
            alSourceStop(source);
        }
    }
}

void AudioManager::Update() {
    if (!m_initialized) return;
    
    // Clean up finished sources (optional optimization)
    for (ALuint source : m_sources) {
        ALint sourceState;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
        if (sourceState == AL_STOPPED) {
            alSourcei(source, AL_BUFFER, 0); // Unbind buffer
        }
    }
}

ALuint AudioManager::GetAvailableSource() {
    for (ALuint source : m_sources) {
        ALint sourceState;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
        if (sourceState != AL_PLAYING) {
            return source;
        }
    }
    return 0; // No available source
}

void AudioManager::GenerateToneBuffer(ALuint buffer, float frequency, float duration, float amplitude) {
    const int sampleRate = 44100;
    const int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<short> data(samples);
    
    for (int i = 0; i < samples; ++i) {
        float time = static_cast<float>(i) / static_cast<float>(sampleRate);
        float sample = amplitude * std::sin(2.0f * M_PI * frequency * time);
        
        // Apply envelope to avoid clicks
        float envelope = 1.0f;
        if (i < samples * 0.1f) {
            envelope = static_cast<float>(i) / (samples * 0.1f);
        } else if (i > samples * 0.9f) {
            envelope = static_cast<float>(samples - i) / (samples * 0.1f);
        }
        
        sample *= envelope;
        data[i] = static_cast<short>(sample * 32767.0f);
    }
    
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), 
                static_cast<ALsizei>(data.size() * sizeof(short)), sampleRate);
}

void AudioManager::GenerateBeepBuffer(ALuint buffer, float frequency, float duration) {
    GenerateToneBuffer(buffer, frequency, duration, 0.3f);
}

void AudioManager::GenerateClickBuffer(ALuint buffer) {
    const int sampleRate = 44100;
    const float duration = 0.05f; // 50ms
    const int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<short> data(samples);
    
    for (int i = 0; i < samples; ++i) {
        float time = static_cast<float>(i) / static_cast<float>(sampleRate);
        
        // Create a click sound with multiple frequency components
        float sample = 0.0f;
        sample += 0.3f * std::sin(2.0f * M_PI * 1000.0f * time);
        sample += 0.2f * std::sin(2.0f * M_PI * 1500.0f * time);
        sample += 0.1f * std::sin(2.0f * M_PI * 2000.0f * time);
        
        // Sharp envelope for click effect
        float envelope = std::exp(-time * 20.0f);
        sample *= envelope;
        
        data[i] = static_cast<short>(sample * 32767.0f);
    }
    
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), 
                static_cast<ALsizei>(data.size() * sizeof(short)), sampleRate);
}

void AudioManager::GenerateSuccessBuffer(ALuint buffer) {
    const int sampleRate = 44100;
    const float duration = 0.8f;
    const int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<short> data(samples);
    
    // Create a pleasant ascending chord
    float frequencies[] = { 523.25f, 659.25f, 783.99f }; // C5, E5, G5
    
    for (int i = 0; i < samples; ++i) {
        float time = static_cast<float>(i) / static_cast<float>(sampleRate);
        
        float sample = 0.0f;
        for (float freq : frequencies) {
            sample += 0.2f * std::sin(2.0f * M_PI * freq * time);
        }
        
        // Gentle envelope
        float envelope = std::exp(-time * 2.0f);
        sample *= envelope;
        
        data[i] = static_cast<short>(sample * 32767.0f);
    }
    
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), 
                static_cast<ALsizei>(data.size() * sizeof(short)), sampleRate);
}

void AudioManager::CleanupBuffers() {
    if (m_comparisonBuffer) {
        alDeleteBuffers(1, &m_comparisonBuffer);
        m_comparisonBuffer = 0;
    }
    if (m_swapBuffer) {
        alDeleteBuffers(1, &m_swapBuffer);
        m_swapBuffer = 0;
    }
    if (m_completionBuffer) {
        alDeleteBuffers(1, &m_completionBuffer);
        m_completionBuffer = 0;
    }
    if (m_errorBuffer) {
        alDeleteBuffers(1, &m_errorBuffer);
        m_errorBuffer = 0;
    }
}

void AudioManager::CleanupSources() {
    if (!m_sources.empty()) {
        // Stop all sources
        for (ALuint source : m_sources) {
            alSourceStop(source);
        }
        
        alDeleteSources(static_cast<ALsizei>(m_sources.size()), m_sources.data());
        m_sources.clear();
    }
}

bool AudioManager::CheckALError(const char* operation) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        fmt::print("OpenAL error in {}: {}\n", operation, error);
        return false;
    }
    return true;
}

bool AudioManager::CheckALCError(const char* operation) {
    ALCenum error = alcGetError(m_device);
    if (error != ALC_NO_ERROR) {
        fmt::print("OpenAL context error in {}: {}\n", operation, error);
        return false;
    }
    return true;
}

void AudioManager::PlayExploreSound(float pitch) {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, pitch * 0.8f);  // Lower pitch for exploration
    alSourcef(source, AL_GAIN, m_masterVolume * 0.4f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_comparisonBuffer);
    
    alSourcePlay(source);
    CheckALError("Play explore sound");
}

void AudioManager::PlayFrontierSound(float pitch) {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, pitch * 1.2f);  // Higher pitch for frontier
    alSourcef(source, AL_GAIN, m_masterVolume * 0.3f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_swapBuffer);
    
    alSourcePlay(source);
    CheckALError("Play frontier sound");
}

void AudioManager::PlayVisitedSound(float pitch) {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, pitch * 0.6f);  // Lower pitch for visited
    alSourcef(source, AL_GAIN, m_masterVolume * 0.25f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_comparisonBuffer);
    
    alSourcePlay(source);
    CheckALError("Play visited sound");
}

void AudioManager::PlayPathFoundSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.2f);
    alSourcef(source, AL_GAIN, m_masterVolume);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_completionBuffer);
    
    alSourcePlay(source);
    CheckALError("Play path found sound");
}

void AudioManager::PlayNoPathSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 0.5f);
    alSourcef(source, AL_GAIN, m_masterVolume);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_errorBuffer);
    
    alSourcePlay(source);
    CheckALError("Play no path sound");
}

void AudioManager::PlayNodeSelectSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.5f);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.7f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_comparisonBuffer);
    
    alSourcePlay(source);
    CheckALError("Play node select sound");
}

void AudioManager::PlayEdgeAddSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.8f);
    alSourcef(source, AL_GAIN, m_masterVolume * 0.8f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_swapBuffer);
    
    alSourcePlay(source);
    CheckALError("Play edge add sound");
}

void AudioManager::PlayMSTCompleteSound() {
    if (!m_enabled || !m_initialized) return;
    
    ALuint source = GetAvailableSource();
    if (source == 0) return;
    
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, m_masterVolume);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, m_completionBuffer);
    
    alSourcePlay(source);
    CheckALError("Play MST complete sound");
}

void AudioManager::PlayPathfindingExplore() {
    PlayExploreSound(); // Use existing method
}

void AudioManager::PlayPathfindingFrontier() {
    PlayFrontierSound();
}

void AudioManager::PlayPathfindingVisited() {
    PlayVisitedSound();
}

void AudioManager::PlayPathfindingPathFound() {
    PlayPathFoundSound();
}

void AudioManager::PlayGraphExplore() {
    PlayNodeSelectSound();
}

void AudioManager::PlayGraphEdgeFound() {
    PlayEdgeAddSound();
}

void AudioManager::PlayGraphNodeProcessed() {
    PlayNodeSelectSound();
}

// Search algorithm audio
void AudioManager::PlaySearchStep() {
    PlayExploreSound();
}

void AudioManager::PlaySearchComparison() {
    PlayExploreSound(1.2f); // Higher pitch for comparison
}

void AudioManager::PlaySearchSuccess() {
    PlayPathFoundSound();
}

void AudioManager::PlaySearchFail() {
    PlayNoPathSound();
}

// Tree algorithm audio
void AudioManager::PlayTreeInsert() {
    PlayEdgeAddSound();
}

void AudioManager::PlayTreeDelete() {
    PlayExploreSound();
}

void AudioManager::PlayTreeRotation() {
    PlayMSTCompleteSound();
}

void AudioManager::PlayTreeTraversal() {
    PlayExploreSound(0.8f); // Lower pitch for traversal
}

void AudioManager::PlayTreeComparison() {
    PlayExploreSound(1.1f); // Slight pitch variation
}

} // namespace AlgorithmVisualizer 