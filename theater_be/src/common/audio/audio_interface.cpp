#include "audio_interface.h"

void AudioInterface::QueueAudioChunk(const AudioChunk& chunk)
{
    std::lock_guard<std::mutex> lock(m_audioMutex);
    m_audioQueue.push(chunk);
}

bool AudioInterface::GetNextAudioChunk(AudioChunk& outChunk)
{
    std::lock_guard<std::mutex> lock(m_audioMutex);
    if (m_audioQueue.empty()) {
        return false;
    }
    outChunk = m_audioQueue.front();
    m_audioQueue.pop();
    return true;
}


size_t AudioInterface::GetQueueSize()
{
    std::lock_guard<std::mutex> lock(m_audioMutex);
    return m_audioQueue.size();
}

void AudioInterface::ClearQueue()
{
    std::lock_guard<std::mutex> lock(m_audioMutex);
    std::queue<AudioChunk> emptyQueue;
    m_audioQueue.swap(emptyQueue);
}