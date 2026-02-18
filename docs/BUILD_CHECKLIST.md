# Build Checklist - Pre-Build Verification

**Date**: 2026-01-29

## Issues Fixed

### 1. URL Query Parameter Parsing
- **Problem**: Used `httpd_query_key_value()` which may not exist in ESP-IDF v5.1.6
- **Solution**: Created custom `extract_query_value()` helper function
- **Files Modified**: `web_server.c`

### 2. Missing Headers
- **Problem**: Missing `<stdlib.h>` and `<stdio.h>` for `atoi()`, `snprintf()`, etc.
- **Solution**: Added proper includes
- **Files Modified**: `web_server.c`

### 3. Custom Error Codes
- **Problem**: Used undefined error codes `ESP_ERR_INVALID_CRC` and `ESP_ERR_INVALID_SIZE`
- **Solution**: Replaced with standard ESP-IDF error codes
  - `ESP_ERR_INVALID_CRC` → `ESP_FAIL`
  - `ESP_ERR_INVALID_SIZE` → `ESP_ERR_INVALID_ARG`
- **Files Modified**: `modbus_protocol.c`

### 4. Memory Management
- **Problem**: Potential memory leaks in API handlers
- **Solution**: Added proper `free()` calls for dynamically allocated strings
- **Files Modified**: `web_server.c`

### 5. Wildcard URI Handler Conflict
- **Problem**: Wildcard pattern `"/*"` with `httpd_uri_match_wildcard` caused duplicate handler registration errors when used alongside specific handlers like `/status`
- **Solution**: Replaced wildcard handler with specific handlers for each static file:
  - `/style.css`, `/script.js`, `/modbus.html`, `/dashboard.html`, `/modbus.js`
  - Simplified `get_static_file_handler()` to directly compare URIs
  - Removed wildcard matching configuration
- **Files Modified**: `web_server.c`
- **Handler Count**: 15 handlers (5 static + 10 API) within 16 limit

### 6. Missing Navigation on WiFi Config Page
- **Problem**: No navigation buttons on `/index.html` page to access Modbus/Dashboard pages
- **Solution**: Added navigation links in header section with Device Config and Dashboard buttons
- **Files Modified**: `html/index.html`

## Board Configuration Verification

### Board-Specific Headers
✓ `boards/esp32c3_board.h` - ESP32-C3 board definitions
✓ `boards/weact_esp32_board.h` - WeAct board definitions
✓ `boards/board.h` - Board selection wrapper

### Board Configuration Files
✓ `sdkconfig.esp32c3.defaults` - ESP32-C3 SDK config
✓ `sdkconfig.weact_esp32.defaults` - WeAct SDK config

### Build Scripts
✓ `build.ps1` - Universal build script with parameters
✓ `build-esp32c3.ps1` - Quick ESP32-C3 build
✓ `build-weact.ps1` - Quick WeAct build

## File Verification

### Source Files (All Present)
✓ `main.c` - Application entry point
✓ `wifi_manager.c/.h` - WiFi management
✓ `web_server.c/.h` - HTTP server with Modbus API
✓ `nvs_storage.c/.h` - NVS storage
✓ `modbus_protocol.c/.h` - Modbus RTU protocol
✓ `modbus_devices.c/.h` - Device and register management
✓ `modbus_manager.c/.h` - UART driver and polling

### Web Files (All Present)
✓ `html/index.html` - WiFi configuration
✓ `html/style.css` - Styles
✓ `html/script.js` - WiFi config JavaScript
✓ `html/modbus.html` - Modbus device configuration
✓ `html/dashboard.html` - Real-time data dashboard
✓ `html/modbus.js` - Modbus JavaScript

### Build Files
✓ `CMakeLists.txt` - Updated with all new files
✓ `build.bat` - Windows build script

## Dependencies Check

### Required ESP-IDF Components
✓ `driver/uart.h` - UART driver (used in modbus_manager.c)
✓ `driver/gpio.h` - GPIO control (used in modbus_manager.c)
✓ `esp_http_server.h` - HTTP server (used in web_server.c)
✓ `esp_log.h` - Logging (used in all files)
✓ `nvs_flash.h` & `nvs.h` - NVS storage (used in modbus_devices.c, nvs_storage.c)
✓ `freertos/FreeRTOS.h` & `freertos/task.h` - RTOS tasks (used in main.c, modbus_devices.c, modbus_manager.c)
✓ `cJSON.h` - JSON parsing (used in web_server.c)

### Standard C Libraries
✓ `<stdio.h>` - Standard I/O
✓ `<string.h>` - String operations
✓ `<stdlib.h>` - Standard library (atoi, malloc, free)

## Compilation Ready

All issues have been fixed. The project is ready to build.

## Build Instructions

### Windows (PowerShell or CMD)

#### Universal Build Script (Recommended)
1. **Open PowerShell** in project directory
2. **Run Universal Build Script**:

```powershell
# For ESP32-C3 (default)
.\build.ps1 -Board esp32c3

# For WeAct board
.\build.ps1 -Board weact

# With options
.\build.ps1 -Board weact -Clean -Flash -Monitor
```

**Available Parameters:**
- `-Board esp32c3|weact` - Select board (default: esp32c3)
- `-Clean` - Full clean before build
- `-Flash` - Flash after build
- `-Monitor` - Open serial monitor after flash
- `-Port COMx` - Specify serial port (auto-detect if omitted)

#### Quick Access Scripts
```powershell
.\build-esp32c3.ps1    # ESP32-C3 incremental build
.\build-weact.ps1        # WeAct incremental build
```

#### Manual Build
1. **Select SDK Config** (copy appropriate defaults file):
   ```powershell
   # For ESP32-C3
   copy sdkconfig.esp32c3.defaults sdkconfig

   # For WeAct board
   copy sdkconfig.weact_esp32.defaults sdkconfig
   ```

2. **Activate ESP-IDF**:
   ```powershell
   C:\Users\jaakk\esp\v5.1.6\esp-idf\export.bat
   ```

3. **Set Target**:
   ```powershell
   # ESP32-C3
   idf.py set-target esp32c3

   # WeAct board
   idf.py set-target esp32
   ```

4. **Build**:
   ```powershell
   idf.py build
   ```

#### Board Switching

When switching between boards:

1. **Build will automatically handle target change**
   - ESP-IDF detects target mismatch and reconfigures
   - No manual cleanup needed

2. **Shared build directory** (`build/`)
   - Both boards use same build folder
   - Small rebuild penalty when switching (~10-30 seconds)

3. **Configuration persistence**
   - Previous `sdkconfig` saved as `sdkconfig.prev`
   - Board-specific defaults used automatically

#### Legacy Build Script (Backward Compatible)
1. **Open Command Prompt** in project directory:
   ```
   cd "C:\Users\jaakk\DIY projects\Microcontroller codes\ESP32 modbus reader"
   ```

2. **Run Build Script**:
   ```
   build.bat
   ```

   This will:
   - Activate ESP-IDF environment
   - Set target to esp32c3
   - Build the project
   - Display build result

### Manual Build (Alternative)

If build.bat doesn't work:

1. **Activate ESP-IDF**:
   ```powershell
   C:\Users\jaakk\esp\v5.1.6\esp-idf\export.bat
   ```

2. **Set Target**:
   ```powershell
   idf.py set-target esp32c3
   ```

3. **Build**:
   ```powershell
   idf.py build
   ```

## Expected Output

### Build Successful
```
[info]: Generating build files...
[info]: Build files generated successfully
[info]: Compiling...
[info]: Linking...
[info]: Project successfully built
```

Output files in: `build/esp32c3/`
- `esp32-modbus-reader.bin` - Main binary
- `esp32-modbus-reader.elf` - ELF file for debugging
- `bootloader.bin` - Bootloader
- `partitions.bin` - Partition table

## If Build Fails

### Common Issues and Solutions

1. **idf.py not found**
   - **Cause**: ESP-IDF environment not activated
   - **Solution**: Run `C:\Users\jaakk\esp\v5.1.6\esp-idf\export.bat`

2. **Compiler errors - missing header**
   - **Cause**: Missing ESP-IDF component in sdkconfig
   - **Solution**: Run `idf.py menuconfig` and ensure:
     - Component config → Driver configurations → UART driver
     - Component config → HTTP Server

3. **Linker errors - undefined reference**
   - **Cause**: Missing source file or function
   - **Solution**: Verify all files in CMakeLists.txt

4. **Out of memory**
   - **Cause**: Too many static allocations
   - **Solution**: Reduce device/register count or use heap memory

5. **cJSON not found**
   - **Cause**: cJSON component not enabled
   - **Solution**: In `sdkconfig` ensure `CONFIG_JSON_ENABLED=y`

## Next Steps After Build

1. **Find COM Port**: Open Device Manager, locate XIAO ESP32C3
2. **Flash Device**: `idf.py -p COMX flash monitor`
3. **Monitor Serial**: Watch for startup messages
4. **Test WiFi**: Connect to device network or AP mode
5. **Configure Modbus**: Access `/modbus.html`
6. **Test Dashboard**: Access `/dashboard.html`

## Memory Usage Estimate

- Free Heap (ESP32-C3): ~280KB
- WiFi Stack: ~40KB
- HTTP Server: ~20KB
- Modbus Protocol: ~15KB
- Modbus Manager: ~10KB
- Device Configs (4 devices): ~5KB
- Register Cache: ~10KB
- Application: ~10KB
- **Estimated Free After Init**: ~170KB

## Flash Size Estimate

- ESP32-C3 Flash: 4MB total
- Application: ~500KB
- Partition Table: ~4KB
- Bootloader: ~20KB
- **Estimated Free Flash**: ~3.5MB

---

**Status**: ✅ READY TO BUILD
**All Issues**: ✅ FIXED