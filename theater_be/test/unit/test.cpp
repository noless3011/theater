#include "theater.h"
#include <iostream>
#include <cassert>

// Basic test functions
bool test_add_function() {
    // Test basic addition
    if (add(2, 3) != 5) {
        std::cerr << "Test failed: add(2, 3) expected 5, got " << add(2, 3) << std::endl;
        return false;
    }

    // Test negative numbers
    if (add(-1, 1) != 0) {
        std::cerr << "Test failed: add(-1, 1) expected 0, got " << add(-1, 1) << std::endl;
        return false;
    }

    // Test zero
    if (add(0, 0) != 0) {
        std::cerr << "Test failed: add(0, 0) expected 0, got " << add(0, 0) << std::endl;
        return false;
    }

    std::cout << "All addition tests passed!" << std::endl;
    return true;
}

bool test_print_message() {
    // This is a basic test - just call the function to make sure it doesn't crash
    try {
        print_message();
        std::cout << "Print message test passed!" << std::endl;
        return true;
    }
    catch (...) {
        std::cerr << "Test failed: print_message() threw an exception" << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Running Theater library tests..." << std::endl;

    bool all_tests_passed = true;

    // Run tests
    all_tests_passed &= test_add_function();
    all_tests_passed &= test_print_message();

    if (all_tests_passed) {
        std::cout << "All tests passed!" << std::endl;
        return 0;  // Success
    }
    else {
        std::cerr << "Some tests failed!" << std::endl;
        return 1;  // Failure
    }
}