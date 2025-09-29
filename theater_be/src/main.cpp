#include "main.h"
#include <iostream>

int add(int a, int b) {
    return a + b;
}

void print_message() {
    std::cout << "Hello from the DLL!" << std::endl;
}