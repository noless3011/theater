#pragma once
#include "common/audio/audio_interface.h"
#include "theater.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <propvarutil.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <audioclientactivationparams.h>
#include <endpointvolume.h>
#include <string>
#include <vector>
#include <comdef.h>
#include <atomic>
#include <map>
#include <limits>
#include <iostream>
#include <fstream>
#include <thread>
namespace theater {
    class WasapiAudio : public AudioInterface,
        public IAgileObject,
        public IActivateAudioInterfaceCompletionHandler {
    public:
        WasapiAudio(HANDLE hEvent);
        ~WasapiAudio() override;

        void StartRecording() override;
        void StopRecording() override;
        bool IsRecording() const override;
        void SetListeningProcessIdList(std::vector<DWORD> processIdList);

        STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override;
        STDMETHOD_(ULONG, AddRef)() override;
        STDMETHOD_(ULONG, Release)() override;
        STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* pAsyncOp) override;
        IAudioClient* GetAudioClient() {
            EnterCriticalSection(&m_cs);
            if (m_pAudioClient) {
                m_pAudioClient->AddRef();
            }
            IAudioClient* pClient = m_pAudioClient;
            LeaveCriticalSection(&m_cs);
            return pClient;
        }

        HRESULT GetActivateResult() {
            return m_hr;
        }

    private:
        void CaptureThread();
        void StartProcessRecording(DWORD processId);
        bool IsProcessRunning(DWORD processId) const;

    private:
        std::vector<DWORD> m_processIdList;

        HRESULT m_hr;
        bool comInitialized = false;
        IMMDeviceEnumerator* m_pEnumerator = nullptr;
        IMMDevice* m_pDevice = nullptr;

        IAudioClient* m_pAudioClient = nullptr;
        IAudioCaptureClient* m_pCaptureClient = nullptr;
        WAVEFORMATEX* m_pWaveFormat = nullptr;

        HANDLE m_audioSamplesReadyEvent = nullptr;
        HANDLE m_captureThreadHandle = nullptr;
        HANDLE m_shutdownEvent = nullptr;

        CRITICAL_SECTION m_cs;
        LONG m_refCount;
        HANDLE m_hActivateEvent;
        std::thread m_captureThread;
    };
}