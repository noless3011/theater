#pragma once
#include "theater.h"
#include "audio_types.h"
#include <queue>
#include <mutex>
#include <atomic>

/**
 * @class AudioInterface
 * @brief An abstract base class for handling audio input and providing a thread-safe queue for audio data.
 *
 * This class defines a common contract for platform-specific audio recording
 * implementations. It manages a producer-consumer pattern where a derived
 * recording class (the producer) adds audio chunks to a queue, and another
 * part of the application (the consumer) retrieves them for processing.
 * All queue operations are guaranteed to be thread-safe.
 */

namespace theater {
    class AudioInterface {
    public:
        /**
         * @brief Default constructor.
         */
        AudioInterface() = default;

        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~AudioInterface() noexcept = default;

    public:
        /**
         * @brief Adds an audio chunk to the end of the processing queue.
         * This method is thread-safe.
         * @param chunk The AudioChunk object to be enqueued.
         */
        void QueueAudioChunk(const AudioChunk& chunk);

        /**
         * @brief Retrieves and removes the next audio chunk from the front of the queue.
         * This method is thread-safe.
         * @param outChunk A reference to an AudioChunk object that will be populated with the data.
         * @return true if a chunk was successfully retrieved, false if the queue was empty.
         */
        bool GetNextAudioChunk(AudioChunk& outChunk);

        /**
         * @brief Pure virtual function to start the audio capture process.
         * Derived classes must implement this to begin recording from a specific audio API.
         */
        virtual void StartRecording() = 0;

        /**
         * @brief Pure virtual function to stop the audio capture process.
         * Derived classes must implement this to stop recording from a specific audio API.
         */
        virtual void StopRecording() = 0;

        /**
         * @brief Pure virtual function to check if audio is currently being captured.
         * @return true if recording is active, false otherwise.
         */
        virtual bool IsRecording() const = 0;

        /**
         * @brief Gets the current number of audio chunks waiting in the queue.
         * This method is thread-safe.
         * @return The number of elements in the queue.
         */
        size_t GetQueueSize();

        /**
         * @brief Removes all pending audio chunks from the queue.
         * This method is thread-safe.
         */
        void ClearQueue();

    protected:
        // The queue holding incoming audio data.
        std::queue<AudioChunk> m_audioQueue;

        // A mutex to protect access to m_audioQueue, ensuring thread-safety.
        std::mutex m_audioMutex;

        // An atomic flag to indicate the desired recording state.
        std::atomic<bool> g_isRecording = true;
    };


} // namespace theater::common::process
