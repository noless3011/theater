//include <theater.h>

#ifndef THEATER_H
#define THEATER_H

#ifdef _WIN32
#ifdef THEATER_EXPORTS
#define THEATER_API __declspec(dllexport)
#else
#define THEATER_API __declspec(dllimport)
#endif
#else
// For non-Windows platforms (like Linux), this is not needed.
#define THEATER_API
#endif

// Declare the functions we want to export from our DLL.
extern "C" {
    THEATER_API int add(int a, int b);
    THEATER_API void print_message();
}

#endif
