#pragma once  
#include "theater.h"
#include "common/process/process_interface.h"
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <endpointvolume.h>
namespace theater {
    class WinProcessFinder : public ProcessFinder {
    public:
        ~WinProcessFinder() override;

        std::map<ProcessId, std::wstring> ListProcesses() override;
        bool IsProcessRunning(ProcessId processId) override;
        std::wstring GetProcessName(ProcessId processId) override;

    private:
        IMMDeviceEnumerator* m_pEnumerator;
        IMMDevice* m_pDevice;
        IAudioSessionManager2* m_pSessionManager;
        IAudioSessionEnumerator* m_pSessionEnumerator;
    };

} // namespace theater
