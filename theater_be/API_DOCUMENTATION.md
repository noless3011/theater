# Theater Audio Capture API Documentation

The Theater library provides a comprehensive C API for capturing audio from specific Windows processes using WASAPI (Windows Audio Session API). This library allows you to:

- Enumerate running processes
- Identify processes that are producing audio
- Capture audio streams from specific processes
- Retrieve audio data in real-time

## Table of Contents

1. [Getting Started](#getting-started)
2. [API Reference](#api-reference)
3. [Usage Examples](#usage-examples)
4. [Error Handling](#error-handling)
5. [Memory Management](#memory-management)
6. [Platform Support](#platform-support)

## Getting Started

### Prerequisites

- Windows 10 or later (WASAPI support required)
- Visual Studio 2019 or later with C++ support
- Administrator privileges (recommended for full functionality)

### Basic Usage Pattern

```cpp
#include "theater.h"

int main() {
    // Initialize COM
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    // 1. Create process map
    ProcessMapHandle processMap = CreateProcessMap();

    // 2. Get process list
    int count;
    ProcessId* processes = GetProcessList(processMap, &count);

    // 3. Create audio capture
    void* audioCapture = CreateAudioCapture();

    // 4. Set target processes
    SetTargetProcessList(audioCapture, processes, count);

    // 5. Start capture
    StartAudioCapture(audioCapture);

    // 6. Get audio data
    unsigned char* data;
    int size;
    while (GetNextAudioChunk(audioCapture, &data, &size)) {
        // Process audio data
        FreeAudioChunk(data);
    }

    // 7. Cleanup
    StopAudioCapture(audioCapture);
    DestroyAudioCapture(audioCapture);
    FreeProcessList(processes);
    DestroyProcessMap(processMap);

    CoUninitialize();
    return 0;
}
```

## API Reference

### Basic Functions

#### `int add(int a, int b)`

Simple test function that adds two integers.

#### `void print_message()`

Prints a test message to the log.

### Process Management API

#### `ProcessMapHandle CreateProcessMap()`

Creates a new process map for enumerating system processes.

**Returns:** Handle to the process map, or `nullptr` on failure.

#### `void DestroyProcessMap(ProcessMapHandle handle)`

Destroys a process map and frees associated resources.

**Parameters:**

- `handle`: Process map handle to destroy

#### `ProcessId* GetProcessList(ProcessMapHandle handle, int* count)`

Retrieves a list of all running processes that are currently producing audio.

**Parameters:**

- `handle`: Process map handle
- `count`: Pointer to integer that will receive the number of processes

**Returns:** Array of process IDs, or `nullptr` on failure. Must be freed with `FreeProcessList()`.

#### `int IsProcessRunning(ProcessId processId)`

Checks if a specific process is currently running.

**Parameters:**

- `processId`: The process ID to check

**Returns:** 1 if running, 0 if not running or error

#### `const wchar_t* GetProcessName(ProcessId processId)`

Gets the executable name of a process.

**Parameters:**

- `processId`: The process ID

**Returns:** Wide string containing the process name, or empty string on error

#### `void FreeProcessList(ProcessId* processList)`

Frees memory allocated by `GetProcessList()`.

**Parameters:**

- `processList`: Array returned by `GetProcessList()`

#### `void FreeProcessName(const wchar_t* processName)`

Reserved for future use. Process names are managed internally.

### Audio Capture API

#### `void* CreateAudioCapture()`

Creates a new audio capture instance using WASAPI.

**Returns:** Handle to audio capture instance, or `nullptr` on failure.

#### `void DestroyAudioCapture(void* audioCapture)`

Destroys an audio capture instance and frees resources.

**Parameters:**

- `audioCapture`: Audio capture handle to destroy

#### `int StartAudioCapture(void* audioCapture)`

Starts audio recording for the configured target processes.

**Parameters:**

- `audioCapture`: Audio capture handle

**Returns:** 1 on success, 0 on failure

#### `int StopAudioCapture(void* audioCapture)`

Stops audio recording.

**Parameters:**

- `audioCapture`: Audio capture handle

**Returns:** 1 on success, 0 on failure

#### `int IsAudioRecording(void* audioCapture)`

Checks if audio capture is currently active.

**Parameters:**

- `audioCapture`: Audio capture handle

**Returns:** 1 if recording, 0 if not recording

#### `void SetTargetProcessList(void* audioCapture, ProcessId* processIds, int count)`

Sets the list of processes to capture audio from.

**Parameters:**

- `audioCapture`: Audio capture handle
- `processIds`: Array of process IDs
- `count`: Number of process IDs

### Audio Data Retrieval API

#### `int GetNextAudioChunk(void* audioCapture, unsigned char** data, int* size)`

Retrieves the next available audio chunk from the capture queue.

**Parameters:**

- `audioCapture`: Audio capture handle
- `data`: Pointer to receive audio data pointer
- `size`: Pointer to receive data size in bytes

**Returns:** 1 if chunk retrieved, 0 if no data available

**Note:** The returned data must be freed with `FreeAudioChunk()`.

#### `void FreeAudioChunk(unsigned char* data)`

Frees memory allocated for an audio chunk.

**Parameters:**

- `data`: Pointer returned by `GetNextAudioChunk()`

#### `int GetAudioQueueSize(void* audioCapture)`

Gets the current number of audio chunks waiting in the queue.

**Parameters:**

- `audioCapture`: Audio capture handle

**Returns:** Number of chunks in queue

#### `void ClearAudioQueue(void* audioCapture)`

Removes all pending audio chunks from the queue.

**Parameters:**

- `audioCapture`: Audio capture handle

## Usage Examples

### Example 1: List All Audio-Producing Processes

```cpp
ProcessMapHandle processMap = CreateProcessMap();
if (processMap) {
    int count;
    ProcessId* processes = GetProcessList(processMap, &count);

    for (int i = 0; i < count; i++) {
        const wchar_t* name = GetProcessName(processes[i]);
        wprintf(L"Process: %s (PID: %d)\\n", name, processes[i]);
    }

    FreeProcessList(processes);
    DestroyProcessMap(processMap);
}
```

### Example 2: Capture Audio from Specific Process

```cpp
void* audioCapture = CreateAudioCapture();
if (audioCapture) {
    ProcessId targetProcess = 1234; // Your target process ID
    SetTargetProcessList(audioCapture, &targetProcess, 1);

    if (StartAudioCapture(audioCapture)) {
        // Capture for 5 seconds
        time_t startTime = time(nullptr);
        while (time(nullptr) - startTime < 5) {
            unsigned char* data;
            int size;

            if (GetNextAudioChunk(audioCapture, &data, &size)) {
                // Process your audio data here
                printf("Got %d bytes of audio data\\n", size);
                FreeAudioChunk(data);
            }

            Sleep(10); // Small delay
        }

        StopAudioCapture(audioCapture);
    }

    DestroyAudioCapture(audioCapture);
}
```

### Example 3: Real-time Audio Processing

```cpp
void ProcessAudioInRealTime() {
    void* audioCapture = CreateAudioCapture();
    // ... setup code ...

    while (IsAudioRecording(audioCapture)) {
        int queueSize = GetAudioQueueSize(audioCapture);

        if (queueSize > 10) {
            // Queue is getting full, process chunks quickly
            printf("Processing %d audio chunks\\n", queueSize);
        }

        unsigned char* data;
        int size;
        if (GetNextAudioChunk(audioCapture, &data, &size)) {
            // Your audio processing code here
            AnalyzeAudioData(data, size);
            FreeAudioChunk(data);
        }
    }
}
```

## Error Handling

All functions are designed to be robust and handle errors gracefully:

- Functions returning pointers return `nullptr` on failure
- Functions returning integers return 0 on failure, 1 on success
- Check return values before using results
- The library logs errors internally using spdlog

Example error handling:

```cpp
ProcessMapHandle processMap = CreateProcessMap();
if (!processMap) {
    fprintf(stderr, "Failed to create process map\\n");
    return -1;
}

int count;
ProcessId* processes = GetProcessList(processMap, &count);
if (!processes || count == 0) {
    fprintf(stderr, "No processes found\\n");
    DestroyProcessMap(processMap);
    return -1;
}
```

## Memory Management

The API follows consistent memory management patterns:

### Allocation/Deallocation Pairs

- `CreateProcessMap()` / `DestroyProcessMap()`
- `CreateAudioCapture()` / `DestroyAudioCapture()`
- `GetProcessList()` / `FreeProcessList()`
- `GetNextAudioChunk()` / `FreeAudioChunk()`

### Important Notes

- Always free resources in reverse order of allocation
- Never use freed pointers
- Process names returned by `GetProcessName()` are managed internally
- Audio chunks must be freed immediately after processing

## Platform Support

### Windows (Primary Platform)

- **Supported:** Windows 10, Windows 11
- **Requirements:** WASAPI support, COINIT_MULTITHREADED COM initialization
- **Permissions:** Administrator privileges recommended for full process access

### Linux/macOS

- **Status:** Not currently implemented
- **Future:** Linux implementation using PulseAudio/ALSA planned
- **Workaround:** Functions return safe defaults (empty lists, failure codes)

## Technical Details

### Audio Format

- **Sample Rate:** 44.1 kHz
- **Channels:** Stereo (2 channels)
- **Bit Depth:** 16-bit PCM
- **Format:** Raw PCM data in little-endian format

### Thread Safety

- Process enumeration functions are thread-safe
- Audio capture functions are thread-safe
- Multiple audio capture instances can run simultaneously
- Each audio capture instance manages its own thread

### Performance Considerations

- Audio chunks are queued in memory - process them promptly
- Use `GetAudioQueueSize()` to monitor queue depth
- Clear queue with `ClearAudioQueue()` if falling behind
- Each process capture requires separate audio session

## Troubleshooting

### Common Issues

1. **"Failed to create process map"**

   - Ensure COM is initialized: `CoInitializeEx(nullptr, COINIT_MULTITHREADED)`
   - Check if running with sufficient privileges

2. **"No processes found"**

   - Ensure target applications are running and producing audio
   - Check Windows audio privacy settings
   - Try running as Administrator

3. **"Audio activation failed"**

   - Target process may have terminated
   - Check Windows audio permissions
   - Ensure process is actively playing audio

4. **"GetNextAudioChunk returns 0"**
   - Normal behavior when no audio data is available
   - Check if target process is playing audio
   - Verify audio capture was started successfully

### Debug Information

The library uses spdlog for internal logging. Enable debug output by configuring spdlog before using the API:

```cpp
#include <spdlog/spdlog.h>

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S] [%l] %v");

    // Your API calls here
}
```
