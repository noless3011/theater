// include/my_library.h

#ifndef MY_LIBRARY_H
#define MY_LIBRARY_H

// This is a standard pattern for creating a portable DLL.
// When we build the DLL, MYLIB_EXPORTS will be defined, and MYLIB_API
// will be __declspec(dllexport). When an application includes this header,
// MYLIB_EXPORTS will not be defined, and MYLIB_API will be __declspec(dllimport).
#ifdef _WIN32
#ifdef MYLIB_EXPORTS
#define MYLIB_API __declspec(dllexport)
#else
#define MYLIB_API __declspec(dllimport)
#endif
#else
    // For non-Windows platforms (like Linux), this is not needed.
#define MYLIB_API
#endif

// Declare the functions we want to export from our DLL.
extern "C" {
    MYLIB_API int add(int a, int b);
    MYLIB_API void print_message();
}

#endif // MY_LIBRARY_H