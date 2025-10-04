#pragma once

#include <string>
#include <map>
#include <memory>
#include "theater.h"

namespace theater {

    class ProcessFinder {
    public:
        ProcessFinder() = default;
        virtual ~ProcessFinder() = default;

        virtual std::map<ProcessId, std::wstring> ListProcesses() = 0;
        virtual bool IsProcessRunning(ProcessId processId) = 0;
        virtual std::wstring GetProcessName(ProcessId processId) = 0;
    };

} // namespace theater