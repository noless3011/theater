# Test configuration for Theater library
# This file can be included from the main CMakeLists.txt for better organization

# Find all test source files
file(GLOB_RECURSE UNIT_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.c"
)

file(GLOB_RECURSE INTEGRATION_TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/integration/*.c"
)

# Create unit test executable if sources exist
if(UNIT_TEST_SOURCES)
    add_executable(TheaterUnitTests ${UNIT_TEST_SOURCES})
    
    target_include_directories(TheaterUnitTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../vendors/spdlog/include
    )
    
    target_link_libraries(TheaterUnitTests PRIVATE Theater)
    
    # Set compiler options for testing
    if(WIN32)
        target_compile_options(TheaterUnitTests PRIVATE 
            /W4 /permissive-
        )
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(TheaterUnitTests PRIVATE /Od /Zi /RTC1)
            target_link_options(TheaterUnitTests PRIVATE /DEBUG)
        endif()
    else()
        target_compile_options(TheaterUnitTests PRIVATE 
            -Wall -Wextra -Wpedantic
        )
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(TheaterUnitTests PRIVATE -g -O0)
        endif()
    endif()
    
    # Copy DLL to test directory on Windows
    if(WIN32)
        add_custom_command(TARGET TheaterUnitTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Theater>
            $<TARGET_FILE_DIR:TheaterUnitTests>
            COMMENT "Copying Theater DLL to unit test directory"
        )
    endif()
    
    # Add unit tests
    add_test(NAME UnitTests COMMAND $<TARGET_FILE:TheaterUnitTests>)
    set_tests_properties(UnitTests PROPERTIES
        TIMEOUT 120
        LABELS "unit"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Create integration test executable if sources exist
if(INTEGRATION_TEST_SOURCES)
    add_executable(TheaterIntegrationTests ${INTEGRATION_TEST_SOURCES})
    
    target_include_directories(TheaterIntegrationTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../vendors/spdlog/include
    )
    
    target_link_libraries(TheaterIntegrationTests PRIVATE Theater)
    
    # Set compiler options for testing
    if(WIN32)
        target_compile_options(TheaterIntegrationTests PRIVATE 
            /W4 /permissive-
        )
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(TheaterIntegrationTests PRIVATE /Od /Zi /RTC1)
            target_link_options(TheaterIntegrationTests PRIVATE /DEBUG)
        endif()
    endif()
    
    # Copy DLL to test directory on Windows
    if(WIN32)
        add_custom_command(TARGET TheaterIntegrationTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Theater>
            $<TARGET_FILE_DIR:TheaterIntegrationTests>
            COMMENT "Copying Theater DLL to integration test directory"
        )
    endif()
    
    # Add integration tests
    add_test(NAME IntegrationTests COMMAND $<TARGET_FILE:TheaterIntegrationTests>)
    set_tests_properties(IntegrationTests PROPERTIES
        TIMEOUT 300
        LABELS "integration"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Add custom targets for running specific test categories
if(UNIT_TEST_SOURCES)
    add_custom_target(run_unit_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -L unit --output-on-failure -V
        DEPENDS TheaterUnitTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running unit tests only"
    )
endif()

if(INTEGRATION_TEST_SOURCES)
    add_custom_target(run_integration_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -L integration --output-on-failure -V
        DEPENDS TheaterIntegrationTests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running integration tests only"
    )
endif()

# Add a target to run all tests with verbose output
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -V
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all tests with verbose output"
)

# Add a target to run tests with memory checking (if available)
find_program(VALGRIND_PROGRAM valgrind)
if(VALGRIND_PROGRAM AND NOT WIN32)
    add_custom_target(run_tests_memcheck
        COMMAND ${CMAKE_CTEST_COMMAND} -T memcheck
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running tests with memory checking"
    )
endif()