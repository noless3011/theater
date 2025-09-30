//include <theater.h>
#pragma once
#ifndef THEATER_H
#define THEATER_H

#if defined(_WIN32) || defined(_WIN64)
#define CHECK_HR(hr) switch(hr){case S_OK:break;default:if(FAILED(hr)){std::cerr<<"Error: 0x"<<std::hex<<hr<<std::endl;return E_FAIL;}}
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

#endif
