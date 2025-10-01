#include "wasapi_capture.h"


WASAPIAudio::WASAPIAudio()
{
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CHECK_HR(hr, "CoInitializeEx failed");

    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);

    CHECK_HR(hr, "CoCreateInstance for MMDeviceEnumerator failed");

    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    CHECK_HR(hr, "GetDefaultAudioEndpoint failed");

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    CHECK_HR(hr, "Device Activate failed");

    hr = pAudioClient->GetMixFormat(&pwfx);
    CHECK_HR(hr, "GetMixFormat failed");

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 00, 0, pwfx, NULL);
    CHECK_HR(hr, "AudioClient Initialize failed");

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    CHECK_HR(hr, "GetService for AudioCaptureClient failed");

    log(spdlog::level::info, "WASAPI Audio initialized successfully with format: {} channels, {} Hz, {} bits per sample",
        pwfx->nChannels, pwfx->nSamplesPerSec, pwfx->wBitsPerSample);

}

WASAPIAudio::~WASAPIAudio()
{
    if (pCaptureClient) {
        pCaptureClient->Release();
        pCaptureClient = nullptr;
    }
    if (pAudioClient) {
        pAudioClient->Stop();
        pAudioClient->Release();
        pAudioClient = nullptr;
    }
    if (pDevice) {
        pDevice->Release();
        pDevice = nullptr;
    }
    if (pEnumerator) {
        pEnumerator->Release();
        pEnumerator = nullptr;
    }
    if (pwfx) {
        CoTaskMemFree(pwfx);
        pwfx = nullptr;
    }
    CoUninitialize();
}
void WASAPIAudio::StartRecording()
{
    if (g_isRecording) {
        return; // Already recording
    }

    g_isRecording = true;
    hr = pAudioClient->Start();
    CHECK_HR(hr, "AudioClient Start failed");

    m_captureThread = std::thread(&WASAPIAudio::CaptureThread, this);
}

void WASAPIAudio::StopRecording()
{
    if (!g_isRecording) {
        return; // Not recording
    }

    g_isRecording = false;
    if (m_captureThread.joinable()) {
        m_captureThread.join();
    }

    hr = pAudioClient->Stop();
    CHECK_HR(hr, "pAudioClient->Stop failed");
}


void WASAPIAudio::CaptureThread()
{
    while (g_isRecording) {
        UINT32 packetLength = 0;
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        CHECK_HR(hr, "GetNextPacketSize failed");

        if (packetLength > 0) {
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;

            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            CHECK_HR(hr, "GetBuffer failed");

            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                DWORD bytesToCopy = numFramesAvailable * pwfx->nBlockAlign;
                AudioChunk chunk;
                chunk.data.assign(pData, pData + bytesToCopy);
                {
                    std::lock_guard<std::mutex> lock(m_audioMutex);
                    m_audioQueue.push(chunk);
                }
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            CHECK_HR(hr, "ReleaseBuffer failed");
        }
        else {
            Sleep(10); // Wait a bit if no data
        }
    }
}


bool WASAPIAudio::IsRecording() const
{
    return g_isRecording;
}

