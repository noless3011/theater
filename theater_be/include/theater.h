//include <theater.h>
#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include<spdlog/spdlog.h>
#ifndef THEATER_H
#define THEATER_H

#if defined(_WIN32) || defined(_WIN64)
#define CHECK_HR(hr) do { if(FAILED(hr)) { log(spdlog::level::critical, "HRESULT failed: 0x{:x}", hr); throw std::runtime_error("HRESULT check failed."); } } while(0)
#ifdef THEATER_EXPORTS
#define THEATER_API __declspec(dllexport)
#else
#define THEATER_API __declspec(dllimport)
#endif
#else
// For non-Windows platforms (like Linux), this is not needed.
#define THEATER_API __attribute__((visibility("default")))
#endif

// Declare the functions we want to export from our DLL.
extern "C" {
    THEATER_API int add(int a, int b);
    THEATER_API void print_message();
}

template<typename... Args>
void log(spdlog::level::level_enum level, spdlog::format_string_t<Args...> fmt, Args &&...args)
{
    spdlog::log(level, fmt, std::forward<Args>(args)...);
}

#endif
