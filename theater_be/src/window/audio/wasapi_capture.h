#pragma once
#include "common/audio/audio_interface.h"
#include "theater.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
class WASAPIAudio : public AudioInterface {
public:
    WASAPIAudio();
    ~WASAPIAudio() override;

    void StartRecording() override;
    void StopRecording() override;
    bool IsRecording() const override;

private:
    void CaptureThread();

    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;
    HANDLE hAudioSamplesReadyEvent = nullptr;
    HANDLE hCaptureThread = nullptr;
    HANDLE hShutdownEvent = nullptr;

    std::thread m_captureThread;

};