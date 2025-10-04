//include <theater.h>
#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include<spdlog/spdlog.h>

#ifndef THEATER_H
#define THEATER_H

typedef void* ProcessMapHandle;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
using ProcessId = DWORD;
#else
#include <sys/types.h>
using ProcessId = pid_t;
#endif
#if defined(_WIN32) || defined(_WIN64)
#define CHECK_HR(hr) do { if(FAILED(hr)) { log(spdlog::level::critical, "HRESULT failed: 0x{:x}", hr); throw std::runtime_error("HRESULT check failed."); } } while(0)
#ifdef THEATER_EXPORTS
#define THEATER_API __declspec(dllexport)
#else
#define THEATER_API __declspec(dllimport)
#endif
#else
// For non-Windows platforms (like Linux), this is not needed.
#define THEATER_API __attribute__((visibility("default")))
#endif

// Declare the functions we want to export from our DLL.
extern "C" {
    // Basic API test functions
    THEATER_API int add(int a, int b);
    THEATER_API void print_message();

    // Process Management API
    THEATER_API ProcessMapHandle CreateProcessMap();
    THEATER_API void DestroyProcessMap(ProcessMapHandle handle);
    THEATER_API ProcessId* GetProcessList(ProcessMapHandle handle, int* count);
    THEATER_API int IsProcessRunning(ProcessId processId);
    THEATER_API const wchar_t* GetProcessName(ProcessId processId);
    THEATER_API void FreeProcessList(ProcessId* processList);
    THEATER_API void FreeProcessName(const wchar_t* processName);

    // Audio Capture API
    THEATER_API void* CreateAudioCapture();
    THEATER_API void DestroyAudioCapture(void* audioCapture);
    THEATER_API int StartAudioCapture(void* audioCapture);
    THEATER_API int StopAudioCapture(void* audioCapture);
    THEATER_API int IsAudioRecording(void* audioCapture);
    THEATER_API void SetTargetProcessList(void* audioCapture, ProcessId* processIds, int count);

    // Audio Data Retrieval API
    THEATER_API int GetNextAudioChunk(void* audioCapture, unsigned char** data, int* size);
    THEATER_API void FreeAudioChunk(unsigned char* data);
    THEATER_API int GetAudioQueueSize(void* audioCapture);
    THEATER_API void ClearAudioQueue(void* audioCapture);
}

template<typename... Args>
void log(spdlog::level::level_enum level, spdlog::format_string_t<Args...> fmt, Args &&...args)
{
    spdlog::log(level, fmt, std::forward<Args>(args)...);
}

template <class T> void SafeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#endif


