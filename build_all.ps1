# Build script to generate library files for node-gyp integration
# This script builds the C++ library and copies the necessary files

Write-Host "Building Theater C++ Library for Node.js Integration" -ForegroundColor Green

# Change to the backend directory
Set-Location "theater_be"

Write-Host "Configuring CMake..." -ForegroundColor Yellow
try {
    cmake --preset gcc
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
} catch {
    Write-Host "Error: CMake configuration failed: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Building library..." -ForegroundColor Yellow
try {
    cmake --build out/build/gcc
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed"
    }
} catch {
    Write-Host "Error: CMake build failed: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Copying library files for node-gyp..." -ForegroundColor Yellow
try {
    .\copy_lib_files.ps1
} catch {
    Write-Host "Error: Failed to copy library files: $_" -ForegroundColor Red
    exit 1
}

# Change to theater directory and build the native addon
Set-Location "..\theater"

Write-Host "Building Node.js native addon..." -ForegroundColor Yellow
try {
    npm run build:native
    if ($LASTEXITCODE -ne 0) {
        throw "Node.js native addon build failed"
    }
} catch {
    Write-Host "Error: Node.js native addon build failed: $_" -ForegroundColor Red
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Generated files:" -ForegroundColor Cyan
Write-Host "  - theater_be/out/build/gcc/libtheater.dll (C++ shared library)" -ForegroundColor Gray
Write-Host "  - theater_be/out/build/gcc/libtheater.dll.a (import library)" -ForegroundColor Gray
Write-Host "  - theater/native/libtheater.lib (copied for node-gyp)" -ForegroundColor Gray
Write-Host "  - theater/native/libtheater.dll (copied for runtime)" -ForegroundColor Gray
Write-Host "  - theater/native/build/Release/native_addon.node (Node.js addon)" -ForegroundColor Gray