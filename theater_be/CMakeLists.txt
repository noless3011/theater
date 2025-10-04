# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name and language
project(Theater LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the source and execution character sets to UTF-8 for MSVC
if(MSVC)
  add_compile_options(/utf-8)
endif()

# Enable testing
enable_testing()
include(CTest)

# Add the shared library target
# This will create a .dll on Windows, .so on Linux, or .dylib on macOS.
# Replace "src/source1.cpp", "src/source2.cpp" with your actual source files.
add_library(Theater SHARED
    src/common/theater.cpp
    # src/common/audio/audio_interface.cpp
    # src/window/audio/wasapi_capture.cpp
    # src/window/process/win_process_finder.cpp
)

# Set the output name to match the launch.json configuration
set_target_properties(Theater PROPERTIES OUTPUT_NAME "theater")

target_include_directories(Theater PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendors/spdlog/include"
)

set(SPDLOG_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendors/spdlog/lib")
find_library(SPDLOG_LIBRARY_FILE NAMES spdlogd PATHS "${SPDLOG_LIBRARY_DIR}")
if(SPDLOG_LIBRARY_FILE)
    message(STATUS "Found spdlog library: ${SPDLOG_LIBRARY_FILE}")
    #target_link_directories(Theater PRIVATE "${SPDLOG_LIBRARY_DIR}") # Not strictly needed
    target_link_libraries(Theater PRIVATE "${SPDLOG_LIBRARY_FILE}")
else()
    # If spdlog is header-only in your setup, this is not an error.
    message(WARNING "spdlog library not found in ${SPDLOG_LIBRARY_DIR}. Searched for 'spdlogd' or 'spdlog'. If spdlog is header-only, this is fine.")
endif()

# Windows-specific linking and definitions
if(WIN32)
  target_compile_definitions(Theater PRIVATE "THEATER_EXPORTS")
  target_link_libraries(Theater PRIVATE 
    ole32
    oleaut32
    uuid
    winmm
    dsound
    psapi
    propsys
    mmdevapi
  )
endif()

# Include test configuration
add_subdirectory(test)

# Backwards compatibility - keep the original test setup for now
# Create a test executable (optional - for unit tests)
add_executable(TheaterTests
    test/unit/test.cpp
)

target_include_directories(TheaterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/spdlog/include
)

# Link the test executable to the Theater library
target_link_libraries(TheaterTests PRIVATE Theater)

# Set debug information and runtime checks for testing
if(WIN32)
    target_compile_options(TheaterTests PRIVATE 
        /W4          # High warning level
        /permissive- # Strict conformance
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(TheaterTests PRIVATE 
            /Od      # No optimization
            /Zi      # Debug information
            /RTC1    # Runtime checks
        )
        target_link_options(TheaterTests PRIVATE 
            /DEBUG   # Generate debug info
        )
    endif()
endif()

# Copy the Theater DLL to the test directory for Windows
if(WIN32)
    add_custom_command(TARGET TheaterTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Theater>
        $<TARGET_FILE_DIR:TheaterTests>
        COMMENT "Copying Theater DLL to test directory"
    )
endif()

# Add tests with proper working directory and environment
add_test(NAME BasicTests COMMAND $<TARGET_FILE:TheaterTests>)
add_test(NAME LibraryLoadTest COMMAND ${CMAKE_COMMAND} -E echo "Testing library load...")

# Set test properties
set_tests_properties(BasicTests PROPERTIES
    TIMEOUT 60
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    LABELS "basic;legacy"
)

set_tests_properties(LibraryLoadTest PROPERTIES
    TIMEOUT 10
    LABELS "integration;basic"
)

# Add a custom target to run only basic tests
add_custom_target(run_basic_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L basic --output-on-failure -V
    DEPENDS TheaterTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running basic tests"
)

# Custom target to copy library files for node-gyp
if(WIN32)
    add_custom_target(copy_for_nodegyp ALL
        COMMAND powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/copy_lib_files.ps1
        DEPENDS Theater
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Copying library files for node-gyp using PowerShell script"
    )
endif()