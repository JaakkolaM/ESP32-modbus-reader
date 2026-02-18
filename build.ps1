<#
.SYNOPSIS
    Universal build script for ESP32 Modbus Reader with board selection
.DESCRIPTION
    Supports incremental builds, clean builds, and flashing for ESP32-C3 and WeAct ESP32 boards
.PARAMETER Board
    Board selection: 'esp32c3' (default) or 'weact'
.PARAMETER Clean
    Perform full clean before build
.PARAMETER Flash
    Flash firmware after successful build
.PARAMETER Monitor
    Open serial monitor after flashing
.PARAMETER Port
    Serial port for flashing (default: auto-detect)
.EXAMPLE
    .\build.ps1 -Board esp32c3
    .\build.ps1 -Board weact -Clean
    .\build.ps1 -Board weact -Flash -Monitor
#>

param(
    [ValidateSet('esp32c3', 'weact')]
    [string]$Board = 'esp32c3',

    [switch]$Clean,
    [switch]$Flash,
    [switch]$Monitor,
    [string]$Port
)

$env:IDF_PATH = "C:\Users\jaakk\esp\v5.1.6\esp-idf"
$ErrorActionPreference = "Stop"

$BoardConfig = @{
    'esp32c3' = @{
        'Name' = 'ESP32-C3'
        'Target' = 'esp32c3'
        'ConfigFile' = 'sdkconfig.esp32c3.defaults'
    };
    'weact' = @{
        'Name' = 'WeAct ESP32-D0WD-V3'
        'Target' = 'esp32'
        'ConfigFile' = 'sdkconfig.weact_esp32.defaults'
    }
}

$currentBoard = $BoardConfig[$Board]
$buildType = if ($Clean) { "FULL CLEAN" } else { "INCREMENTAL" }

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32 Modbus Reader Build System" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Board: $($currentBoard.Name)" -ForegroundColor Green
Write-Host "Build Type: $buildType" -ForegroundColor Yellow
Write-Host ""

Write-Host "[1/4] Activating ESP-IDF..." -ForegroundColor Gray
& "$env:IDF_PATH\export.ps1"

Write-Host "[2/4] Loading board configuration..." -ForegroundColor Gray
if (Test-Path "sdkconfig") {
    Move-Item -Path "sdkconfig" -Destination "sdkconfig.prev" -Force
}
Copy-Item -Path $currentBoard.ConfigFile -Destination "sdkconfig" -Force

Write-Host "[3/4] Setting target to $($currentBoard.Target)..." -ForegroundColor Gray
idf.py set-target $currentBoard.Target

# CRITICAL: Re-apply board config AFTER set-target (it overwrites sdkconfig)
Write-Host "        Re-applying board configuration..." -ForegroundColor Yellow
Copy-Item -Path $currentBoard.ConfigFile -Destination "sdkconfig" -Force

if ($Board -eq "weact") {
    Write-Host "        Forcing 8MB flash size for WeAct board..." -ForegroundColor Yellow
    (Get-Content sdkconfig) -replace 'CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y', 'CONFIG_ESPTOOLPY_FLASHSIZE_4MB=n' `
                           -replace '# CONFIG_ESPTOOLPY_FLASHSIZE_8MB is not set', 'CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y' `
                           -replace 'CONFIG_ESPTOOLPY_FLASHSIZE="4MB"', 'CONFIG_ESPTOOLPY_FLASHSIZE="8MB"' `
                           | Set-Content sdkconfig
}

# Re-apply partition configuration (set-target may override it)
if ($Board -eq "weact") {
    Write-Host "        Re-applying partition configuration..." -ForegroundColor Yellow
    (Get-Content sdkconfig) -replace 'CONFIG_PARTITION_TABLE_SINGLE_APP=y', 'CONFIG_PARTITION_TABLE_SINGLE_APP=n' `
                               -replace '# CONFIG_PARTITION_TABLE_CUSTOM is not set', 'CONFIG_PARTITION_TABLE_CUSTOM=y' `
                               | Set-Content sdkconfig
}

if ($Clean) {
    Write-Host "        Performing full clean..." -ForegroundColor Yellow
    idf.py fullclean
}

Write-Host "[4/4] Building..." -ForegroundColor Gray

# Pass board selection to CMake
$cmakeBoard = if ($Board -eq "weact") { "weact_esp32" } else { $Board }
Write-Host "        Using board: $cmakeBoard" -ForegroundColor Cyan

$buildResult = idf.py build -D BOARD=$cmakeBoard

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output: build\$($currentBoard.Target)/esp32-modbus-reader.bin" -ForegroundColor Cyan
    Write-Host ""

    if ($Flash) {
        Write-Host ""
        Write-Host "Flashing to board..." -ForegroundColor Yellow

        if ([string]::IsNullOrEmpty($Port)) {
            Write-Host "Detecting COM port..." -ForegroundColor Gray
            try {
                Add-Type -AssemblyName System.IO.Ports
                $ports = [System.IO.Ports.SerialPort]::GetPortNames()
                if ($ports.Count -eq 1) {
                    $Port = $ports[0]
                    Write-Host "Auto-detected: $Port" -ForegroundColor Green
                } elseif ($ports.Count -gt 1) {
                    Write-Host "Multiple COM ports found:" -ForegroundColor Yellow
                    $ports | ForEach-Object { Write-Host "  - $_" }
                    $Port = Read-Host "Enter COM port (e.g., COM3)"
                } else {
                    Write-Host "No COM ports found!" -ForegroundColor Red
                    exit 1
                }
            } catch {
                Write-Host "Unable to detect COM ports. Please specify manually with -Port parameter." -ForegroundColor Yellow
                $Port = Read-Host "Enter COM port (e.g., COM3)"
            }
        }

        $flashCmd = "idf.py -p $Port flash"
        if ($Monitor) {
            $flashCmd += " monitor"
        }

        Invoke-Expression $flashCmd
    } else {
        Write-Host ""
        Write-Host "To flash to board:" -ForegroundColor Cyan
        Write-Host "  idf.py -p COM3 flash monitor" -ForegroundColor White
        Write-Host ""
        Write-Host "Or use build script:" -ForegroundColor Cyan
        Write-Host "  .\build.ps1 -Board $Board -Flash" -ForegroundColor White
        Write-Host ""
        Write-Host "========================================" -ForegroundColor Green
    }
} else {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "BUILD FAILED!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    exit 1
}

Write-Host ""
Read-Host -Prompt "Press Enter to exit"
