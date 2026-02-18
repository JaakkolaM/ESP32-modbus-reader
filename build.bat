@echo off
echo ESP32 Modbus Reader - Quick Build (ESP32-C3)
echo.
call C:\Users\jaakk\esp\v5.1.6\esp-idf\export.bat
idf.py set-target esp32c3
idf.py build
if %errorlevel% neq 0 (
    echo.
    echo BUILD FAILED!
    pause
    exit /b 1
)
echo.
echo BUILD SUCCESSFUL!
pause
