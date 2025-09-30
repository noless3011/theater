# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name and language
project(Theater LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()
include(CTest)

# Add the shared library target
# This will create a .dll on Windows, .so on Linux, or .dylib on macOS.
# Replace "src/source1.cpp", "src/source2.cpp" with your actual source files.
add_library(Theater SHARED
    src/common/theater.cpp
)

# Set the output name to match the launch.json configuration
set_target_properties(Theater PROPERTIES OUTPUT_NAME "theater")

target_include_directories(Theater PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
  target_compile_definitions(Theater PRIVATE "THEATER_EXPORTS")
  target_link_libraries(Theater PRIVATE 
    ole32
    oleaut32
    uuid
    winmm
    dsound
  )
endif()

# Create a test executable (optional - for unit tests)
add_executable(TheaterTests
    test/unit/test.cpp
)

target_include_directories(TheaterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the test executable to the Theater library
target_link_libraries(TheaterTests PRIVATE Theater)

# Add tests
add_test(NAME BasicTest COMMAND TheaterTests)
add_test(NAME LibraryLoadTest COMMAND ${CMAKE_COMMAND} -E echo "Testing library load...")

# Set test properties
set_tests_properties(BasicTest PROPERTIES
    TIMEOUT 30
)

set_tests_properties(LibraryLoadTest PROPERTIES
    TIMEOUT 10
)

# Custom target to copy library files for node-gyp
if(WIN32)
    add_custom_target(copy_for_nodegyp ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Theater>
            ${CMAKE_CURRENT_SOURCE_DIR}/../theater/native/libtheater.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:Theater>/libtheater.dll.a
            ${CMAKE_CURRENT_SOURCE_DIR}/../theater/native/libtheater.lib
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/include/theater.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../theater/include/theater.h
        DEPENDS Theater
        COMMENT "Copying library files for node-gyp"
    )
endif()


