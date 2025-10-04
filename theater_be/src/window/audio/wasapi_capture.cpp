#include "wasapi_capture.h"

using namespace theater;

WasapiAudio::WasapiAudio(HANDLE hEvent) : m_refCount(1), m_hActivateEvent(hEvent), m_pAudioClient(nullptr), m_hr(S_OK)
{
    InitializeCriticalSection(&m_cs);
}

WasapiAudio::~WasapiAudio() {
    // Stop recording first
    StopRecording();

    EnterCriticalSection(&m_cs);
    SafeRelease(&m_pCaptureClient);
    SafeRelease(&m_pAudioClient);
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pEnumerator);
    CoTaskMemFree(m_pWaveFormat);

    if (m_hActivateEvent)
    {
        CloseHandle(m_hActivateEvent);
        m_hActivateEvent = nullptr;
    }

    if (m_captureThreadHandle)
    {
        CloseHandle(m_captureThreadHandle);
        m_captureThreadHandle = nullptr;
    }
    if (m_shutdownEvent)
    {
        CloseHandle(m_shutdownEvent);
        m_shutdownEvent = nullptr;
    }

    if (m_audioSamplesReadyEvent)
    {
        CloseHandle(m_audioSamplesReadyEvent);
        m_audioSamplesReadyEvent = nullptr;
    }
    LeaveCriticalSection(&m_cs);
    DeleteCriticalSection(&m_cs);
    if (comInitialized) {
        CoUninitialize();
    }
}

void theater::WasapiAudio::StartRecording()
{
    if (IsRecording()) {
        return; // Already recording
    }

    HRESULT hr = S_OK;

    try {
        // Initialize COM
        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(hr)) {
            comInitialized = true;
        }

        hr = CoInitializeSecurity(
            NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL
        );
        CHECK_HR(hr);

        // Create device enumerator
        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
        CHECK_HR(hr);

        // Get default audio endpoint
        hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice);
        CHECK_HR(hr);

        // Create shutdown event
        m_shutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_shutdownEvent == NULL) {
            CHECK_HR(E_FAIL);
        }

        // Start recording for all processes in the list
        if (!m_processIdList.empty()) {
            for (DWORD processId : m_processIdList) {
                StartProcessRecording(processId);
                break; // For now, only handle one process at a time
            }
        }
        else {
            log(spdlog::level::warn, "No process IDs specified for recording");
            return;
        }

        // Start capture thread
        g_isRecording = true;
        m_captureThread = std::thread(&WasapiAudio::CaptureThread, this);

        log(spdlog::level::info, "WASAPI recording started successfully");
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to start recording: {}", e.what());
        StopRecording();
    }
}

bool theater::WasapiAudio::IsRecording() const
{
    return g_isRecording.load();
}

void theater::WasapiAudio::StopRecording()
{
    if (!IsRecording()) {
        return; // Already stopped
    }

    g_isRecording = false;

    // Signal shutdown
    if (m_shutdownEvent) {
        SetEvent(m_shutdownEvent);
    }

    // Wait for capture thread to finish
    if (m_captureThread.joinable()) {
        m_captureThread.join();
    }

    // Stop audio client
    if (m_pAudioClient) {
        m_pAudioClient->Stop();
    }

    log(spdlog::level::info, "WASAPI recording stopped");
}

void theater::WasapiAudio::SetListeningProcessIdList(std::vector<DWORD> processIdList)
{
    m_processIdList = std::move(processIdList);
    log(spdlog::level::info, "Set {} process(es) for audio capture", m_processIdList.size());
}


void theater::WasapiAudio::CaptureThread()
{
    HRESULT hr = S_OK;

    try {
        while (g_isRecording.load()) {
            if (!m_pAudioClient || !m_pCaptureClient) {
                Sleep(10);
                continue;
            }

            UINT32 packetLength = 0;
            hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                log(spdlog::level::err, "GetNextPacketSize failed: 0x{:x}", hr);
                break;
            }

            while (packetLength != 0 && g_isRecording.load()) {
                BYTE* pData = nullptr;
                UINT32 numFramesAvailable = 0;
                DWORD flags = 0;

                hr = m_pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
                if (FAILED(hr)) {
                    log(spdlog::level::err, "GetBuffer failed: 0x{:x}", hr);
                    break;
                }

                if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && pData && numFramesAvailable > 0) {
                    // Calculate the size of the audio data
                    UINT32 bytesToCapture = numFramesAvailable * m_pWaveFormat->nBlockAlign;

                    // Create an audio chunk and copy the data
                    AudioChunk chunk;
                    chunk.data.resize(bytesToCapture);
                    memcpy(chunk.data.data(), pData, bytesToCapture);

                    // Queue the audio chunk for processing
                    QueueAudioChunk(chunk);
                }

                hr = m_pCaptureClient->ReleaseBuffer(numFramesAvailable);
                if (FAILED(hr)) {
                    log(spdlog::level::err, "ReleaseBuffer failed: 0x{:x}", hr);
                    break;
                }

                hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
                if (FAILED(hr)) {
                    log(spdlog::level::err, "GetNextPacketSize failed: 0x{:x}", hr);
                    break;
                }
            }

            // Check for shutdown event
            if (WaitForSingleObject(m_shutdownEvent, 10) == WAIT_OBJECT_0) {
                break;
            }
        }
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Capture thread error: {}", e.what());
    }

    log(spdlog::level::info, "Capture thread exiting");
}

void theater::WasapiAudio::StartProcessRecording(DWORD processId)
{
    // Check if process is running first
    if (!IsProcessRunning(processId)) {
        log(spdlog::level::err, "Process {} is not running", processId);
        return;
    }

    HRESULT hr = S_OK;
    LPWSTR pwszDeviceId = nullptr;
    IActivateAudioInterfaceAsyncOperation* pAsyncOp = nullptr;

    try {
        // Get device ID
        hr = m_pDevice->GetId(&pwszDeviceId);
        CHECK_HR(hr);

        // Setup activation parameters for process loopback
        AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
        activationParams.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
        activationParams.ProcessLoopbackParams.TargetProcessId = processId;
        activationParams.ProcessLoopbackParams.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

        PROPVARIANT activateProps = {};
        PropVariantInit(&activateProps);
        activateProps.vt = VT_BLOB;
        activateProps.blob.cbSize = sizeof(activationParams);
        activateProps.blob.pBlobData = (BYTE*)&activationParams;

        // Activate audio interface asynchronously
        hr = ActivateAudioInterfaceAsync(
            VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,
            __uuidof(IAudioClient),
            &activateProps,
            this, // Use this object as completion handler
            &pAsyncOp
        );
        CHECK_HR(hr);

        // Wait for activation to complete
        log(spdlog::level::info, "Waiting for audio activation for process {}", processId);
        DWORD waitResult = WaitForSingleObject(m_hActivateEvent, 5000);
        if (waitResult != WAIT_OBJECT_0) {
            log(spdlog::level::err, "Audio activation timed out for process {}", processId);
            return;
        }

        // Check if activation was successful
        if (FAILED(m_hr)) {
            log(spdlog::level::err, "Audio activation failed for process {} with HRESULT: 0x{:x}", processId, m_hr);
            return;
        }

        if (!m_pAudioClient) {
            log(spdlog::level::err, "Failed to get AudioClient for process {}", processId);
            return;
        }

        // Set up audio format - allocate on heap to ensure it persists
        m_pWaveFormat = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));
        if (!m_pWaveFormat) {
            CHECK_HR(E_OUTOFMEMORY);
        }
        ZeroMemory(m_pWaveFormat, sizeof(WAVEFORMATEX));
        m_pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
        m_pWaveFormat->nChannels = 2;
        m_pWaveFormat->nSamplesPerSec = 44100;
        m_pWaveFormat->wBitsPerSample = 16;
        m_pWaveFormat->nBlockAlign = m_pWaveFormat->nChannels * m_pWaveFormat->wBitsPerSample / 8;
        m_pWaveFormat->nAvgBytesPerSec = m_pWaveFormat->nSamplesPerSec * m_pWaveFormat->nBlockAlign;

        log(spdlog::level::info, "Audio format: {} Hz, {} channels, {}-bit",
            m_pWaveFormat->nSamplesPerSec, m_pWaveFormat->nChannels, m_pWaveFormat->wBitsPerSample);

        // Initialize audio client
        hr = m_pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            0,
            0,
            m_pWaveFormat,
            NULL
        );
        CHECK_HR(hr);

        // Get capture service
        hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_pCaptureClient);
        CHECK_HR(hr);

        // Start audio client
        hr = m_pAudioClient->Start();
        CHECK_HR(hr);

        log(spdlog::level::info, "Successfully started audio capture for process {}", processId);
    }
    catch (const std::exception& e) {
        log(spdlog::level::err, "Failed to start process recording for PID {}: {}", processId, e.what());
    }

    // Cleanup
    if (pwszDeviceId) {
        CoTaskMemFree(pwszDeviceId);
    }
    SafeRelease(&pAsyncOp);
}

bool theater::WasapiAudio::IsProcessRunning(DWORD processId) const
{
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (process) {
        DWORD exitCode;
        if (GetExitCodeProcess(process, &exitCode)) {
            CloseHandle(process);
            return (exitCode == STILL_ACTIVE);
        }
        CloseHandle(process);
    }
    return false;
}


STDMETHODIMP_(HRESULT __stdcall) theater::WasapiAudio::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject) {
        return E_POINTER;
    }

    *ppvObject = NULL;

    if (riid == IID_IUnknown) {
        *ppvObject = static_cast<IUnknown*>(static_cast<IActivateAudioInterfaceCompletionHandler*>(this));
        AddRef();
        return S_OK;
    }
    else if (riid == __uuidof(IActivateAudioInterfaceCompletionHandler)) {
        *ppvObject = static_cast<IActivateAudioInterfaceCompletionHandler*>(this);
        AddRef();
        return S_OK;
    }
    else if (riid == __uuidof(IAgileObject)) {
        *ppvObject = static_cast<IAgileObject*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG __stdcall) theater::WasapiAudio::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}


STDMETHODIMP_(ULONG __stdcall) theater::WasapiAudio::Release()
{
    LONG refCount = InterlockedDecrement(&m_refCount);
    if (0 == refCount) {
        delete this;
    }
    return refCount;
}


STDMETHODIMP_(HRESULT __stdcall) theater::WasapiAudio::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* pAsyncOp)
{
    HRESULT hr = S_OK;
    HRESULT hrActivate = S_OK;
    IUnknown* pUnknown = nullptr;

    hr = pAsyncOp->GetActivateResult(&hrActivate, &pUnknown);

    EnterCriticalSection(&m_cs);

    if (SUCCEEDED(hr) && SUCCEEDED(hrActivate)) {
        hr = pUnknown->QueryInterface(__uuidof(IAudioClient), (void**)&m_pAudioClient);
        if (FAILED(hr)) {
            log(spdlog::level::err, "Failed to get IAudioClient from activation result. HRESULT: 0x{:x}", hr);
        }
    }
    else {
        log(spdlog::level::err, "Audio interface activation failed with HRESULT: 0x{:x}", hrActivate);

        switch (hrActivate) {
        case E_ACCESSDENIED:
            log(spdlog::level::err, "  -> Access denied. Check microphone privacy settings or run as Administrator.");
            break;
        case AUDCLNT_E_DEVICE_INVALIDATED:
            log(spdlog::level::err, "  -> The audio device has been invalidated.");
            break;
        case AUDCLNT_E_DEVICE_IN_USE:
            log(spdlog::level::err, "  -> The device is already in use.");
            break;
        case E_INVALIDARG:
            log(spdlog::level::err, "  -> Invalid argument. The target process may have terminated.");
            break;
        }
    }

    m_hr = hrActivate;

    LeaveCriticalSection(&m_cs);

    SafeRelease(&pUnknown);
    SetEvent(m_hActivateEvent);

    return S_OK;
}

