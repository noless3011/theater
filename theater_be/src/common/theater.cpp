#include "theater.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <mutex>
#include "audio/audio_interface.h"
#include "process/process_interface.h"

#ifdef _WIN32
#include "window/audio/wasapi_capture.h"
#include "window/process/win_process_finder.h"
#endif

// Internal wrapper classes for C interface
class ProcessMapWrapper {
public:
    ProcessMapWrapper() {
#ifdef _WIN32
        processFinder = std::make_unique<theater::WinProcessFinder>();
#else
        // For non-Windows platforms, implement LinuxProcessFinder
        processFinder = nullptr;
#endif
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }

    ~ProcessMapWrapper() {
        CoUninitialize();
    }

    std::unique_ptr<theater::ProcessFinder> processFinder;
    std::map<ProcessId, std::wstring> cachedProcesses;
};

class AudioCaptureWrapper {
public:
    AudioCaptureWrapper() : activationEvent(nullptr) {
#ifdef _WIN32
        activationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        audioCapture = std::make_unique<theater::WasapiAudio>(activationEvent);
#else
        // For non-Windows platforms, implement LinuxAudioCapture
        audioCapture = nullptr;
#endif
    }

    ~AudioCaptureWrapper() {
        if (audioCapture) {
            audioCapture->StopRecording();
        }
#ifdef _WIN32
        if (activationEvent) {
            CloseHandle(activationEvent);
        }
#endif
    }

    std::unique_ptr<theater::AudioInterface> audioCapture;
    HANDLE activationEvent;
};

// Global storage for process names to ensure they remain valid
static std::map<ProcessId, std::wstring> g_processNames;
static std::mutex g_processNamesMutex;

// ==============================================
// Basic API test functions
// ==============================================

THEATER_API int add(int a, int b) {
    return a + b;
}

THEATER_API void print_message() {
    log(spdlog::level::info, "Hello from Theater DLL!");
}

// ==============================================
// Process Management API Implementation
// ==============================================

THEATER_API ProcessMapHandle CreateProcessMap() {
    try {
        return new ProcessMapWrapper();
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to create process map: {}", e.what());
        return nullptr;
    }
}

THEATER_API void DestroyProcessMap(ProcessMapHandle handle) {
    if (handle) {
        delete static_cast<ProcessMapWrapper*>(handle);
    }
}

THEATER_API ProcessId* GetProcessList(ProcessMapHandle handle, int* count) {
    if (!handle || !count) {
        if (count) *count = 0;
        return nullptr;
    }

    try {
        ProcessMapWrapper* wrapper = static_cast<ProcessMapWrapper*>(handle);
        if (!wrapper->processFinder) {
            if (count) *count = 0;
            return nullptr;
        }

        // Get the list of processes
        wrapper->cachedProcesses = wrapper->processFinder->ListProcesses();

        if (wrapper->cachedProcesses.empty()) {
            *count = 0;
            return nullptr;
        }

        // Allocate array for process IDs
        ProcessId* processArray = new ProcessId[wrapper->cachedProcesses.size()];
        int index = 0;

        for (const auto& pair : wrapper->cachedProcesses) {
            processArray[index++] = pair.first;
        }

        *count = static_cast<int>(wrapper->cachedProcesses.size());
        return processArray;
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to get process list: {}", e.what());
        if (count) *count = 0;
        return nullptr;
    }
}

THEATER_API int IsProcessRunning(ProcessId processId) {
    try {
#ifdef _WIN32
        theater::WinProcessFinder finder;
        return finder.IsProcessRunning(processId) ? 1 : 0;
#else
        return 0; // Not implemented for non-Windows platforms
#endif
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to check if process is running: {}", e.what());
        return 0;
    }
}

THEATER_API const wchar_t* GetProcessName(ProcessId processId) {
    try {
        std::lock_guard<std::mutex> lock(g_processNamesMutex);

#ifdef _WIN32
        theater::WinProcessFinder finder;
        std::wstring name = finder.GetProcessName(processId);
        g_processNames[processId] = name;
        return g_processNames[processId].c_str();
#else
        return L""; // Not implemented for non-Windows platforms
#endif
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to get process name: {}", e.what());
        return L"";
    }
}

THEATER_API void FreeProcessList(ProcessId* processList) {
    if (processList) {
        delete[] processList;
    }
}

THEATER_API void FreeProcessName(const wchar_t* processName) {
    // Process names are managed internally, no need to free
    // This function exists for API completeness
}

// ==============================================
// Audio Capture API Implementation
// ==============================================

THEATER_API void* CreateAudioCapture() {
    try {
        return new AudioCaptureWrapper();
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to create audio capture: {}", e.what());
        return nullptr;
    }
}

THEATER_API void DestroyAudioCapture(void* audioCapture) {
    if (audioCapture) {
        delete static_cast<AudioCaptureWrapper*>(audioCapture);
    }
}

THEATER_API int StartAudioCapture(void* audioCapture) {
    if (!audioCapture) {
        return 0;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return 0;
        }

        wrapper->audioCapture->StartRecording();
        return wrapper->audioCapture->IsRecording() ? 1 : 0;
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to start audio capture: {}", e.what());
        return 0;
    }
}

THEATER_API int StopAudioCapture(void* audioCapture) {
    if (!audioCapture) {
        return 0;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return 0;
        }

        wrapper->audioCapture->StopRecording();
        return wrapper->audioCapture->IsRecording() ? 0 : 1;
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to stop audio capture: {}", e.what());
        return 0;
    }
}

THEATER_API int IsAudioRecording(void* audioCapture) {
    if (!audioCapture) {
        return 0;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return 0;
        }

        return wrapper->audioCapture->IsRecording() ? 1 : 0;
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to check recording status: {}", e.what());
        return 0;
    }
}

THEATER_API void SetTargetProcessList(void* audioCapture, ProcessId* processIds, int count) {
    if (!audioCapture || !processIds || count <= 0) {
        return;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return;
        }

#ifdef _WIN32
        // Cast to WASAPI implementation to access SetListeningProcessIdList
        theater::WasapiAudio* wasapiCapture = static_cast<theater::WasapiAudio*>(wrapper->audioCapture.get());

        std::vector<DWORD> processIdList;
        for (int i = 0; i < count; i++) {
            processIdList.push_back(static_cast<DWORD>(processIds[i]));
        }

        wasapiCapture->SetListeningProcessIdList(processIdList);
#endif
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to set target process list: {}", e.what());
    }
}

// ==============================================
// Audio Data Retrieval API Implementation
// ==============================================

THEATER_API int GetNextAudioChunk(void* audioCapture, unsigned char** data, int* size) {
    if (!audioCapture || !data || !size) {
        return 0;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return 0;
        }

        theater::AudioChunk chunk;
        if (wrapper->audioCapture->GetNextAudioChunk(chunk)) {
            // Allocate memory for the audio data
            unsigned char* audioData = new unsigned char[chunk.data.size()];
            memcpy(audioData, chunk.data.data(), chunk.data.size());

            *data = audioData;
            *size = static_cast<int>(chunk.data.size());
            return 1;
        }

        *data = nullptr;
        *size = 0;
        return 0;
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to get next audio chunk: {}", e.what());
        if (data) *data = nullptr;
        if (size) *size = 0;
        return 0;
    }
}

THEATER_API void FreeAudioChunk(unsigned char* data) {
    if (data) {
        delete[] data;
    }
}

THEATER_API int GetAudioQueueSize(void* audioCapture) {
    if (!audioCapture) {
        return 0;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return 0;
        }

        return static_cast<int>(wrapper->audioCapture->GetQueueSize());
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to get audio queue size: {}", e.what());
        return 0;
    }
}

THEATER_API void ClearAudioQueue(void* audioCapture) {
    if (!audioCapture) {
        return;
    }

    try {
        AudioCaptureWrapper* wrapper = static_cast<AudioCaptureWrapper*>(audioCapture);
        if (!wrapper->audioCapture) {
            return;
        }

        wrapper->audioCapture->ClearQueue();
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to clear audio queue: {}", e.what());
    }
}