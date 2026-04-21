# CarGreeter ZERO-CLICK Multi-Variant Builder
# This script automatically prepares, compiles, and organizes all firmware variants.

$ProjectRoot = Get-Location
$SourceFile = "$ProjectRoot\src\CarGreeter\src\build_config.h"
$ReleaseDir = "$ProjectRoot\releases"
$CliPath = "$ProjectRoot\arduino-cli.exe"
$SketchPath = "$ProjectRoot\src\CarGreeter"

# 1. Setup Check: Does Arduino-CLI exist?
if (!(Test-Path $CliPath)) {
    Write-Host "Downloading Arduino-CLI for automation..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip" -OutFile "$ProjectRoot\cli.zip"
    Expand-Archive -Path "$ProjectRoot\cli.zip" -DestinationPath $ProjectRoot -Force
    Remove-Item "$ProjectRoot\cli.zip"
}

# 2. Prepare Directory
if (!(Test-Path $ReleaseDir)) { 
    New-Item -ItemType Directory -Path $ReleaseDir 
}

function Build-Variant($Version, $TrackIndex) {
    Write-Host "BUILDING VERSION: $Version (Track $TrackIndex)..." -ForegroundColor Cyan
    
    # Update the "DNA" Header
    $Content = @"
#pragma once

#define BUILD_VERSION "$Version"
#define DEFAULT_HOTSPOT_SSID "CarGreeter v$Version"
#define DEFAULT_HOTSPOT_PASS "car12345"

#define DEFAULT_WELCOME_TRACK $TrackIndex

#define DEFAULT_DELAY_SECONDS 5
#define DEFAULT_VOLUME 20
#define DEFAULT_ADMIN_USER "admin"
#define DEFAULT_ADMIN_PASS "1234"
"@
    $Content | Out-File -FilePath $SourceFile -Encoding ASCII -Force

    # Trigger Command-Line Compilation
    Write-Host "Compiling... (please wait)" -ForegroundColor DarkGray
    # Note: We assume the ESP32 platform is already installed in your Arduino IDE
    & $CliPath compile --fqbn esp32:esp32:esp32c3 --output-dir "$ReleaseDir" "$SketchPath"

    # Rename the resulting bin
    $OriginalBin = "$ReleaseDir\CarGreeter.ino.bin"
    if (Test-Path $OriginalBin) {
        $FinalName = "CarGreeter-$Version.bin"
        Move-Item -Path $OriginalBin -Destination "$ReleaseDir\$FinalName" -Force
        
        # Cleanup extra files
        Get-ChildItem -Path $ReleaseDir -Include "*.elf", "*.map", "*.partitions.bin", "*.bootloader.bin" -Recurse | Remove-Item -Force
        
        Write-Host "SUCCESS: Created $FinalName" -ForegroundColor Green
    } else {
        Write-Host "FAILED: Binary not found. Check if the code has errors." -ForegroundColor Red
    }
}

# --- START ASSEMBLY LINE ---

Write-Host "Starting automated build for all variants..." -ForegroundColor Blue

# Define builds here: (VersionString, TrackIndex)
Build-Variant "2.1.0" 1
Build-Variant "2.1.0-1.Tata" 1
Build-Variant "2.1.0-3.Toyota" 3

Write-Host "DONE! Your releases are ready in: $ReleaseDir" -ForegroundColor Cyan
