#pragma once
#include "theater.h"
#include "audio_types.h"
#include <queue>
#include <mutex>
#include <atomic>
class AudioInterface {
public:
    AudioInterface() = default;
    virtual ~AudioInterface() noexcept = default;
public:
    void QueueAudioChunk(const AudioChunk& chunk);
    bool GetNextAudioChunk(AudioChunk& outChunk);
    virtual void StartRecording() = 0;
    virtual void StopRecording() = 0;
    virtual bool IsRecording() const = 0;
    size_t GetQueueSize();
    void ClearQueue();
protected:
    std::queue<AudioChunk> m_audioQueue;
    std::mutex m_audioMutex;
    std::atomic<bool> g_isRecording = true;
};