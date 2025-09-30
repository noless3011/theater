#include "theater.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <fstream>

THEATER_API int add(int a, int b) {
    return a + b;
}

THEATER_API void print_message() {
    std::cout << "Hello from the DLL!" << std::endl;
}