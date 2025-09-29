# Test Configuration for Theater Library

## Running Tests

You can run the tests in several ways:

### Using CTest with presets

```bash
cd theater_be
cmake --preset gcc
cmake --build out/build/gcc
ctest --preset gcc-test
```

### Using CMake directly

```bash
cd theater_be
cmake --preset gcc
cmake --build out/build/gcc
cd out/build/gcc
ctest --verbose
```

### Using VS Code Tasks

- Use Ctrl+Shift+P and run "Tasks: Run Task"
- Select "Run CTest" or "Build and Test"

## Test Structure

- `src/test_main.cpp`: Main test file containing unit tests
- Tests are automatically discovered by CTest
- Test output is shown in the terminal with detailed results

## Available Tests

1. **BasicTest**: Runs the main test executable with comprehensive unit tests
2. **LibraryLoadTest**: Simple test to verify basic functionality

## Test Configuration

- Tests timeout after 30 seconds (BasicTest) or 10 seconds (LibraryLoadTest)
- Failed tests will show detailed output
- Tests run in the build directory context
