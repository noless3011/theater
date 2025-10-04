#include "win_process_finder.h"

theater::WinProcessFinder::~WinProcessFinder()
{
    SafeRelease(&m_pSessionEnumerator);
    SafeRelease(&m_pSessionManager);
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pEnumerator);
}

std::map<ProcessId, std::wstring> theater::WinProcessFinder::ListProcesses()
{
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

    HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
    CHECK_HR(hr);

    hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice);
    CHECK_HR(hr);

    log(spdlog::level::info, "Searching for applications making sound...");
    hr = m_pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&m_pSessionManager);
    CHECK_HR(hr);

    hr = m_pSessionManager->GetSessionEnumerator(&m_pSessionEnumerator);
    CHECK_HR(hr);

    int sessionCount;
    hr = m_pSessionEnumerator->GetCount(&sessionCount);
    CHECK_HR(hr);

    std::map<ProcessId, std::wstring> soundingProcesses;

    IAudioSessionControl* pSessionControl = nullptr;
    IAudioSessionControl2* pSessionControl2 = nullptr;
    IAudioMeterInformation* pMeterInfo = nullptr;
    for (int i = 0; i < sessionCount; i++) {
        pSessionControl = nullptr;
        pSessionControl2 = nullptr;
        pMeterInfo = nullptr;
        try {
            hr = m_pSessionEnumerator->GetSession(i, &pSessionControl);
            if (FAILED(hr)) continue;

            hr = pSessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&pMeterInfo);
            if (SUCCEEDED(hr)) {
                float peakValue = 0;
                pMeterInfo->GetPeakValue(&peakValue);
                if (peakValue > 0.0f) {
                    hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
                    if (SUCCEEDED(hr)) {
                        ProcessId processId = 0;
                        pSessionControl2->GetProcessId(&processId);
                        if (processId != 0 && soundingProcesses.find(processId) == soundingProcesses.end()) {
                            soundingProcesses[processId] = GetProcessName(processId);
                        }
                    }
                }
            }

        }
        catch (...) {}
    }

    SafeRelease(&pMeterInfo);
    SafeRelease(&pSessionControl2);
    SafeRelease(&pSessionControl);
    return soundingProcesses;
}

bool theater::WinProcessFinder::IsProcessRunning(ProcessId processId)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (process) {
        ProcessId exitCode;
        if (GetExitCodeProcess(process, &exitCode)) {
            CloseHandle(process);
            return exitCode == STILL_ACTIVE;
        }
        CloseHandle(process);
    }
    return false;
}

std::wstring theater::WinProcessFinder::GetProcessName(ProcessId processId)
{
    WCHAR processName[MAX_PATH] = L"<unknown>";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    }

    if (hProcess != NULL) {
        ProcessId size = MAX_PATH;
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
