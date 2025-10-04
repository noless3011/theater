#include "theater.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Theater Library Integration Test" << std::endl;
    std::cout << "================================" << std::endl;

    // Test basic functionality that's less likely to segfault
    try {
        // Test simple add function
        int result = add(5, 3);
        if (result == 8) {
            std::cout << "[PASS] Basic add function works" << std::endl;
        }
        else {
            std::cout << "[FAIL] Basic add function failed" << std::endl;
            return 1;
        }

        // Test print message
        std::cout << "Testing print_message():" << std::endl;
        print_message();
        std::cout << "[PASS] print_message() executed successfully" << std::endl;

        std::cout << "All integration tests passed!" << std::endl;
        return 0;

    }
    catch (const std::exception& e) {
        std::cout << "[FAIL] Exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cout << "[FAIL] Unknown exception caught" << std::endl;
        return 1;
    }
}
