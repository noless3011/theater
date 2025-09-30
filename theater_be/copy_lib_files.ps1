# PowerShell script to copy the generated library files to the theater/native directory

$SourceDir = "out\build\gcc"
$TargetDir = "..\theater\native"

# Create target directory if it doesn't exist
if (-not (Test-Path $TargetDir)) {
    New-Item -ItemType Directory -Path $TargetDir -Force
}

# Copy the DLL
if (Test-Path "$SourceDir\libtheater.dll") {
    Copy-Item "$SourceDir\libtheater.dll" "$TargetDir\libtheater.dll" -Force
    Write-Host "Copied libtheater.dll to $TargetDir"
} else {
    Write-Host "libtheater.dll not found in $SourceDir"
}

# Copy the import library and rename it to .lib for node-gyp compatibility
if (Test-Path "$SourceDir\libtheater.dll.a") {
    Copy-Item "$SourceDir\libtheater.dll.a" "$TargetDir\libtheater.lib" -Force
    Write-Host "Copied libtheater.dll.a as libtheater.lib to $TargetDir"
} else {
    Write-Host "libtheater.dll.a not found in $SourceDir"
}

# Also copy the header file
$HeaderSource = "include\theater.h"
$HeaderTarget = "..\theater\include\theater.h"

if (Test-Path $HeaderSource) {
    # Create include directory if it doesn't exist
    $HeaderDir = Split-Path $HeaderTarget -Parent
    if (-not (Test-Path $HeaderDir)) {
        New-Item -ItemType Directory -Path $HeaderDir -Force
    }
    Copy-Item $HeaderSource $HeaderTarget -Force
    Write-Host "Copied theater.h to $HeaderTarget"
} else {
    Write-Host "theater.h not found in $HeaderSource"
}

Write-Host "Library files copied successfully!"