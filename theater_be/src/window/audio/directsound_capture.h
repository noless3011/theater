#pragma once

#include <iostream>
#include "theater.h"
#include "common/audio/audio_interface.h"

class DirectSoundCapture : public AudioInterface
{
public:
    DirectSoundCapture()
    {
        LogNotImplemented("Constructor");
    }

    ~DirectSoundCapture()
    {
        LogNotImplemented("Destructor");
    }

    bool Initialize()
    {
        LogNotImplemented("Initialize");
        return false;
    }
    void StartRecording() override
    {
        LogNotImplemented("StartRecording");
    }
    void StopRecording() override
    {
        LogNotImplemented("StopRecording");
    }
    bool IsRecording() const override
    {
        LogNotImplemented("IsRecording");
        return false;
    }

private:
    void LogNotImplemented(const char* functionName) const
    {
        log(spdlog::level::warn, "{} is not implemented on this platform.", functionName);
    }
};