#define NOMINMAX

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <propvarutil.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <audioclientactivationparams.h>
#include <endpointvolume.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <comdef.h>
#include <atomic>
#include <map>
#include <limits>

// --- Configuration ---
const int RECORD_SECONDS = 10;

// --- Helper Macro for HRESULT checking ---
#define CHECK_HR(hr, msg) if (FAILED(hr)) { _com_error err(hr); std::wcerr << msg << L": " << err.ErrorMessage() << std::endl; throw err; }

// --- Helper to safely release COM objects ---
template <class T> void SafeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

// WAV file header structure
struct WavHeader {
    char riff[4] = { 'R', 'I', 'F', 'F' };
    uint32_t fileSize;
    char wave[4] = { 'W', 'A', 'V', 'E' };
    char fmt[4] = { 'f', 'm', 't', ' ' };
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4] = { 'd', 'a', 't', 'a' };
    uint32_t dataSize;
};

void WriteWavHeader(std::ofstream& file, WAVEFORMATEX* format, uint32_t dataSize) {
    WavHeader header;
    header.numChannels = format->nChannels;
    header.sampleRate = format->nSamplesPerSec;
    header.bitsPerSample = format->wBitsPerSample;
    header.byteRate = format->nAvgBytesPerSec;
    header.blockAlign = format->nBlockAlign;
    header.dataSize = dataSize;
    header.fileSize = dataSize + sizeof(WavHeader) - 8;

    file.seekp(0);
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
}

class CActivateAudioInterfaceCompletionHandler : public IActivateAudioInterfaceCompletionHandler, public IAgileObject {
private:
    LONG m_refCount;
    HANDLE m_hActivateEvent;
    IAudioClient* m_pAudioClient;
    HRESULT m_hrActivate;
    CRITICAL_SECTION m_cs;

public:
    CActivateAudioInterfaceCompletionHandler(HANDLE hEvent)
        : m_refCount(1),
        m_hActivateEvent(hEvent),
        m_pAudioClient(NULL),
        m_hrActivate(S_OK) {
        InitializeCriticalSection(&m_cs);
    }

    virtual ~CActivateAudioInterfaceCompletionHandler() {
        EnterCriticalSection(&m_cs);
        SafeRelease(&m_pAudioClient);
        LeaveCriticalSection(&m_cs);
        DeleteCriticalSection(&m_cs);
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) {
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

    STDMETHOD_(ULONG, AddRef)() {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHOD_(ULONG, Release)() {
        LONG refCount = InterlockedDecrement(&m_refCount);
        if (0 == refCount) {
            delete this;
        }
        return refCount;
    }

    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* pAsyncOp) {
        HRESULT hr = S_OK;
        HRESULT hrActivate = S_OK;
        IUnknown* pUnknown = NULL;

        hr = pAsyncOp->GetActivateResult(&hrActivate, &pUnknown);

        EnterCriticalSection(&m_cs);

        if (SUCCEEDED(hr) && SUCCEEDED(hrActivate)) {
            hr = pUnknown->QueryInterface(__uuidof(IAudioClient), (void**)&m_pAudioClient);
            if (FAILED(hr)) {
                std::wcerr << L"Failed to get IAudioClient from activation result" << std::endl;
            }
        }
        else {
            std::wcerr << L"Audio interface activation failed with HRESULT: 0x"
                << std::hex << hrActivate << std::dec << std::endl;

            if (hrActivate == E_ACCESSDENIED) {
                std::wcerr << L"  -> Access denied. Try running as Administrator." << std::endl;
            }
            else if (hrActivate == AUDCLNT_E_DEVICE_INVALIDATED) {
                std::wcerr << L"  -> The audio device has been invalidated." << std::endl;
            }
            else if (hrActivate == AUDCLNT_E_DEVICE_IN_USE) {
                std::wcerr << L"  -> The device is already in use." << std::endl;
            }
            else if (hrActivate == E_INVALIDARG) {
                std::wcerr << L"  -> Invalid argument. The target process may have terminated." << std::endl;
            }
        }

        m_hrActivate = hrActivate;

        LeaveCriticalSection(&m_cs);

        SafeRelease(&pUnknown);
        SetEvent(m_hActivateEvent);

        return S_OK;
    }

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
        return m_hrActivate;
    }
};

bool IsProcessRunning(DWORD processId) {
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

std::wstring GetProcessNameByPid(DWORD processId) {
    WCHAR processName[MAX_PATH] = L"<unknown>";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    }

    if (hProcess != NULL) {
        DWORD size = MAX_PATH;
        if (!QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
            GetModuleBaseNameW(hProcess, NULL, processName, sizeof(processName) / sizeof(WCHAR));
        }
        CloseHandle(hProcess);

        WCHAR* lastSlash = wcsrchr(processName, L'\\');
        if (lastSlash != NULL) {
            return std::wstring(lastSlash + 1);
        }
    }
    return std::wstring(processName);
}

int main() {
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    HRESULT hr = S_OK;
    bool comInitialized = false;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioSessionManager2* pSessionManager = NULL;
    IAudioSessionEnumerator* pSessionEnumerator = NULL;
    LPWSTR pwszDeviceId = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    HANDLE hActivateEvent = NULL;
    CActivateAudioInterfaceCompletionHandler* pCompletionHandler = NULL;
    IActivateAudioInterfaceAsyncOperation* pAsyncOp = NULL;

    std::ofstream outputFile;
    uint32_t totalBytesWritten = 0;

    try {
        hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        CHECK_HR(hr, L"CoInitializeEx failed");
        comInitialized = true;

        hr = CoInitializeSecurity(
            NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL
        );
        CHECK_HR(hr, "CoInitializeSecurity failed");

        outputFile.open("process_output.wav", std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "Failed to open output file." << std::endl;
            throw std::runtime_error("File open failed");
        }

        // Reserve space for WAV header
        WavHeader dummyHeader = {};
        outputFile.write(reinterpret_cast<char*>(&dummyHeader), sizeof(dummyHeader));

        const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
        CHECK_HR(hr, L"CoCreateInstance failed for enumerator");

        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        CHECK_HR(hr, L"GetDefaultAudioEndpoint failed");

        std::wcout << L"Searching for applications making sound..." << std::endl;
        hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
        CHECK_HR(hr, L"Failed to activate session manager");

        hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
        CHECK_HR(hr, L"Failed to get session enumerator");

        int sessionCount;
        hr = pSessionEnumerator->GetCount(&sessionCount);
        CHECK_HR(hr, L"Failed to get session count");

        std::map<DWORD, std::wstring> soundingProcesses;
        for (int i = 0; i < sessionCount; i++) {
            IAudioSessionControl* pSessionControl = NULL;
            IAudioSessionControl2* pSessionControl2 = NULL;
            IAudioMeterInformation* pMeterInfo = NULL;

            try {
                hr = pSessionEnumerator->GetSession(i, &pSessionControl);
                if (FAILED(hr)) continue;

                hr = pSessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&pMeterInfo);
                if (SUCCEEDED(hr)) {
                    float peakValue = 0;
                    pMeterInfo->GetPeakValue(&peakValue);
                    if (peakValue > 0.0f) {
                        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
                        if (SUCCEEDED(hr)) {
                            DWORD processId = 0;
                            pSessionControl2->GetProcessId(&processId);
                            if (processId != 0 && soundingProcesses.find(processId) == soundingProcesses.end()) {
                                soundingProcesses[processId] = GetProcessNameByPid(processId);
                            }
                        }
                    }
                }
            }
            catch (...) {}

            SafeRelease(&pSessionControl2);
            SafeRelease(&pMeterInfo);
            SafeRelease(&pSessionControl);
        }

        if (soundingProcesses.empty()) {
            std::wcerr << L"No applications are currently playing audio." << std::endl;
            hr = E_FAIL;
        }
        else {
            std::vector<DWORD> pids;
            int choice = 0;
            std::wcout << L"\nFound the following applications making sound:" << std::endl;
            for (const auto& pair : soundingProcesses) {
                pids.push_back(pair.first);
                std::wcout << L"  " << (pids.size()) << L". " << pair.second << L" (PID: " << pair.first << L")" << std::endl;
            }

            while (true) {
                std::wcout << L"\nEnter the number of the application to record: ";
                std::cin >> choice;
                if (std::cin.fail() || choice < 1 || choice > pids.size()) {
                    std::wcerr << L"Invalid input. Please enter a number from the list." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                else {
                    break;
                }
            }

            DWORD targetPid = pids[choice - 1];
            std::wcout << L"Targeting " << soundingProcesses[targetPid] << L" (PID: " << targetPid << L") for recording." << std::endl;

            if (!IsProcessRunning(targetPid)) {
                std::wcerr << L"Error: Target process is no longer running." << std::endl;
                throw std::runtime_error("Process not running");
            }

            hr = pDevice->GetId(&pwszDeviceId);
            CHECK_HR(hr, L"Get device ID failed");

            hActivateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (hActivateEvent == NULL) CHECK_HR(E_FAIL, L"CreateEvent failed");

            pCompletionHandler = new CActivateAudioInterfaceCompletionHandler(hActivateEvent);
            if (pCompletionHandler == NULL) CHECK_HR(E_OUTOFMEMORY, L"Failed to create completion handler");
            pCompletionHandler->AddRef();

            AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
            activationParams.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
            activationParams.ProcessLoopbackParams.TargetProcessId = targetPid;
            activationParams.ProcessLoopbackParams.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

            PROPVARIANT activateProps = {};
            PropVariantInit(&activateProps);
            activateProps.vt = VT_BLOB;
            activateProps.blob.cbSize = sizeof(activationParams);
            activateProps.blob.pBlobData = (BYTE*)&activationParams;

            hr = ActivateAudioInterfaceAsync(VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK, __uuidof(IAudioClient), &activateProps, pCompletionHandler, &pAsyncOp);
            if (FAILED(hr)) {
                std::wcerr << L"ActivateAudioInterfaceAsync failed with HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
                throw _com_error(hr);
            }

            std::wcout << L"Waiting for audio stream from process..." << std::endl;
            DWORD waitResult = WaitForSingleObject(hActivateEvent, 5000);
            if (waitResult != WAIT_OBJECT_0) CHECK_HR(E_FAIL, L"Activation timed out or failed.");

            pAudioClient = pCompletionHandler->GetAudioClient();
            if (pAudioClient == NULL) CHECK_HR(E_FAIL, L"Failed to get AudioClient from callback");

            // Get the actual format from the audio client
            WAVEFORMATEX wfx = {};
            pwfx = &wfx;
            pwfx->wFormatTag = WAVE_FORMAT_PCM;
            pwfx->nChannels = 2;
            pwfx->nSamplesPerSec = 44100;
            pwfx->wBitsPerSample = 16;
            pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
            pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

            std::wcout << L"Recording format: " << pwfx->nSamplesPerSec << L" Hz, "
                << pwfx->nChannels << L" channels, "
                << pwfx->wBitsPerSample << L"-bit" << std::endl;

            hr = pAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK,
                0,
                0,
                pwfx,
                NULL
            );
            CHECK_HR(hr, L"AudioClient Initialize failed");

            hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
            CHECK_HR(hr, L"GetService for IAudioCaptureClient failed");

            std::wcout << L"Recording for " << RECORD_SECONDS << L" seconds..." << std::endl;

            hr = pAudioClient->Start();
            CHECK_HR(hr, L"AudioClient Start failed");

            long long startTime = GetTickCount64();
            while (GetTickCount64() - startTime < (RECORD_SECONDS * 1000)) {
                UINT32 packetLength = 0;
                hr = pCaptureClient->GetNextPacketSize(&packetLength);
                CHECK_HR(hr, L"GetNextPacketSize failed");

                while (packetLength != 0) {
                    BYTE* pData;
                    UINT32 numFramesAvailable;
                    DWORD flags;
                    hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
                    CHECK_HR(hr, L"GetBuffer failed");

                    if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                        UINT32 bytesToWrite = numFramesAvailable * pwfx->nBlockAlign;
                        outputFile.write(reinterpret_cast<char*>(pData), bytesToWrite);
                        totalBytesWritten += bytesToWrite;
                    }

                    hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
                    CHECK_HR(hr, L"ReleaseBuffer failed");

                    hr = pCaptureClient->GetNextPacketSize(&packetLength);
                    CHECK_HR(hr, L"GetNextPacketSize failed");
                }
                Sleep(10);
            }

            hr = pAudioClient->Stop();
            CHECK_HR(hr, L"AudioClient Stop failed");

            // Write the WAV header now that we know the data size
            WriteWavHeader(outputFile, pwfx, totalBytesWritten);

            std::wcout << L"Recording finished. Saved to 'process_output.wav' ("
                << totalBytesWritten << L" bytes)" << std::endl;
        }
    }
    catch (const _com_error& e) {
        hr = e.Error();
    }
    catch (const std::exception& e) {
        std::cerr << "An unhandled C++ exception occurred: " << e.what() << std::endl;
        hr = E_FAIL;
    }
    catch (...) {
        std::cerr << "An unknown exception occurred." << std::endl;
        hr = E_FAIL;
    }

    std::wcout << L"Cleaning up resources." << std::endl;
    if (outputFile.is_open()) outputFile.close();
    if (pwszDeviceId) CoTaskMemFree(pwszDeviceId);
    if (hActivateEvent) CloseHandle(hActivateEvent);

    SafeRelease(&pCompletionHandler);
    SafeRelease(&pAsyncOp);
    SafeRelease(&pEnumerator);
    SafeRelease(&pDevice);
    SafeRelease(&pSessionManager);
    SafeRelease(&pSessionEnumerator);
    SafeRelease(&pAudioClient);
    SafeRelease(&pCaptureClient);

    if (comInitialized) {
        CoUninitialize();
    }

    return FAILED(hr) ? 1 : 0;
}