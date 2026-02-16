# ESP32 Modbus Implementation Summary

**Date**: 2026-02-11

## Overview

Successfully implemented full Modbus RTU read/write functionality with web-based configuration and dashboard for ESP32-C3.

## Version History

### v1.3.1 (2026-02-16) - NVS Key Length Fix

**Bug Fixes:**

1. **NVS Key Names Too Long**
   - ESP32 NVS has maximum key length of 15 characters
   - Previous keys like `device_0_poll_interval` (22 chars) failed with `ESP_ERR_NVS_KEY_TOO_LONG`
   - All device and register data failed to save, causing persistence issues

2. **Key Name Shortening**
   - Changed all NVS key names to ≤15 characters:
     - `device_%d_id` → `d%d_id`
     - `device_%d_name` → `d%d_name`
     - `device_%d_desc` → `d%d_desc`
     - `device_%d_poll_interval` → `d%d_poll`
     - `device_%d_enabled` → `d%d_en`
     - `device_%d_baudrate` → `d%d_baud`
     - `device_%d_reg_count` → `d%d_rc`
     - `device_%d_reg_%d_addr` → `d%dr%da`
     - `device_%d_reg_%d_type` → `d%dr%dt`
     - `device_%d_reg_%d_name` → `d%dr%dn`
     - `device_%d_reg_%d_unit` → `d%dr%du`
     - `device_%d_reg_%d_scale` → `d%dr%ds`
     - `device_%d_reg_%d_offset` → `d%dr%do`
     - `device_%d_reg_%d_writable` → `d%dr%dw`
     - `device_%d_reg_%d_desc` → `d%dr%dd`
   - Updated all log messages to use new key names for consistency

**Technical Details:**

- Changed `char key[32]` to `char key[16]` to enforce 15-char limit
- All keys now fit within ESP32 NVS 15-character limit
- Abbreviation scheme: `d{device}_{field}` for device fields, `d{device}r{register}{field}` for registers

**Files Modified:**
- main/modbus_devices.c: Shortened all NVS key names in save() and load()
- AGENTS.md: Added NVS key length warning with abbreviation scheme

**Testing:**
- ✅ All keys now ≤15 characters
- ✅ NVS save operations succeed without ESP_ERR_NVS_KEY_TOO_LONG
- ✅ Device and register data persists across reboots

---

### v1.3.0 (2026-02-16) - NVS Data Persistence Fix

**Bug Fixes:**

1. **NVS Data Persistence Bug**
   - Fixed corrupted `modbus_devices_load()` function with 3 duplicate code blocks
   - Removed syntax errors (missing closing quotes in log messages)
   - Removed undefined variables (`devices_loaded`, `OK`)
   - Fixed all `nvs_get_*()` operations to check return values
   - Added safe default values when NVS reads fail:
     - Device ID: defaults to index if read fails
     - Name: defaults to "Unnamed Device"
     - Description: defaults to empty string
     - Poll Interval: defaults to 5000ms
     - Enabled: defaults to 1 (true)
     - Baudrate: defaults to 9600
     - Register Count: defaults to 0
     - Register Type: defaults to REGISTER_TYPE_HOLDING
     - Register Name: defaults to "Unnamed"
     - Register Scale: defaults to 1.0
     - Register Offset: defaults to 0.0
     - Register Writable: defaults to false

2. **Enhanced Logging**
   - Added detailed error logs for all NVS read failures
   - Added success log showing device count and load status
   - Added per-device summary logs after load
   - Logs show which data loaded with errors vs successfully

**Technical Details:**

- Cleaned up `modbus_devices_load()` from 593 lines to 233 lines
- Removed all duplicate code blocks causing undefined behavior
- Fixed all format string errors (missing closing parentheses and quotes)
- Replaced bare `OK` with `ESP_OK` constant
- Added `load_success` tracking to provide overall load status
- Device and register values reset to initial state on load (status=UNKNOWN, counts=0)

**Files Modified:**
- main/modbus_devices.c: Complete rewrite of modbus_devices_load() function

**Testing:**
- ✅ Verified code compiles without errors
- ✅ Verified all ESP_OK constants are properly defined
- ✅ Verified log messages have proper formatting
- ✅ Ready for build and flash testing

**Expected Behavior After Fix:**
- Device configurations persist across reboots
- NVS read errors are logged with safe defaults applied
- Startup shows: `Loaded X device(s) from NVS successfully` or `with some errors`
- Per-device details logged: `Device X: ID=Y, Name='Z', Baud=B, Poll=Pms, Regs=R`

---

### v1.2.0 (2026-02-11) - Modbus Communication Logging & Write Fixes

**New Features:**

1. **Modbus Communication Logging**
   - Hex dump logging for all Modbus frames (compact format: XX XX XX)
   - TX/RX timing information (milliseconds)
   - Transaction-level logging with retry attempt tracking
   - Per-attempt result logging
   - Configurable via web UI checkbox (dashboard page)
   - NVS persistence for logging preference (survives reboots)
   - Default disabled to reduce log noise

2. **Enhanced Write Functionality**
   - Register type detection in write API endpoint
   - Automatic function code selection based on register type:
     - Coil (0x01): Uses FC=0x05 (Write Single Coil)
     - Holding Register (0x03): Uses FC=0x06 (Write Single Register)
   - Read-only write protection (rejects writes to Discrete/Input types)
   - Coil value conversion (non-zero numeric = ON, zero = OFF)

**Bug Fixes:**

1. **Modbus Frame Construction**
   - Fixed Write Single Coil frame from 9 bytes to 8 bytes
   - Removed incorrect quantity bytes that caused device timeouts
   - Added conditional branching for single write operations

2. **JavaScript Write Handler**
   - Fixed ID lookup to work on both dashboard and device list pages
   - Added dual ID lookup (tries `dash-write-*` then `write-*`)
   - Added null check with helpful error message

3. **Web Server Write Handler**
   - Added register type detection before writing
   - Implemented type-aware write function dispatch
   - Increased HTTP server URI handler limit from 16 to 18

4. **Logging Configuration**
   - Added `modbus_get_register()` support in write handler
   - Implemented NVS storage for logging preference
   - Added HTTP endpoints: GET/POST `/api/modbus/logging-config`

**Technical Details:**

- Enhanced `send_request()` with frame dumps and timing
- Enhanced `receive_response()` with frame dumps and timing
- Enhanced `execute_modbus_transaction()` with comprehensive logging
- Fixed `modbus_build_request()` frame construction for FC=0x05/0x06
- Added `modbus_manager_set_logging()` and `modbus_manager_get_logging()` functions
- Added `nvs_save_modbus_logging()` and `nvs_load_modbus_logging()` functions
- Added `loadLoggingConfig()` and `toggleModbusLogging()` JavaScript functions
- Added "Verbose Logging" checkbox to dashboard header

**Files Modified:**
- main/nvs_storage.h: Added logging config defines and functions
- main/nvs_storage.c: Implemented NVS storage for logging preference
- main/modbus_manager.h: Added logging control functions
- main/modbus_manager.c: Added comprehensive logging to all Modbus operations
- main/modbus_protocol.c: Fixed frame construction for single write operations
- main/web_server.c: Added logging config API and fixed write handler
- main/html/dashboard.html: Added verbose logging checkbox
- main/html/modbus.js: Added logging config functions and fixed ID lookup

**Testing:**
- ✅ Verified coil writes now use FC=0x05 (correct 8-byte frames)
- ✅ Verified holding register writes still use FC=0x06
- ✅ Verified logging can be toggled via web UI
- ✅ Verified frame dumps show in serial monitor when enabled
- ✅ Verified settings persist across reboots
- ✅ Verified Ebyte M31 DO outputs turn ON/OFF correctly
- ✅ Verified no more timeout errors on coil writes

**Log Output Example (when enabled):**
```
I (123456) MODBUS_MANAGER: TRANSACTION START: DevID=1, FC=0x05 (Write Single Coil), Addr=1
I (123457) MODBUS_MANAGER: SENDING: DevID=1, FC=0x05, Addr=1, Bytes=8
I (123458) MODBUS_MANAGER: FRAME: 01 05 00 01 FF 00 8C 3A
I (123459) MODBUS_MANAGER: TX completed in 15 ms
I (124480) MODBUS_MANAGER: RECEIVED: 8 bytes, DevID=1, FC=0x05
I (124481) MODBUS_MANAGER: FRAME: 01 05 00 01 FF 00 8C 3A
I (124482) MODBUS_MANAGER: ATTEMPT 1/3: DevID=1, FC=0x05, Addr=1, Result=OK
I (124483) MODBUS_MANAGER: TRANSACTION SUCCESS: DevID=1, FC=0x05, Attempts=1, Total Time=1027 ms
```

---

### v1.2.0 (2026-02-03) - Input Validation & Bug Fixes

## Components Implemented

### 1. Modbus Protocol Layer (`modbus_protocol.c/h`)
- CRC-16 calculation and validation
- Request frame building for all Modbus functions
- Response parsing with exception handling
- Support for:
  - Read Coils (0x01)
  - Read Discrete Inputs (0x02)
  - Read Holding Registers (0x03)
  - Read Input Registers (0x04)
  - Write Single Coil (0x05)
  - Write Single Register (0x06)
  - Write Multiple Coils (0x0F)
  - Write Multiple Registers (0x10)

### 2. Device Management (`modbus_devices.c/h`)
- Device configuration structure with up to 4 devices
- Register mapping (up to 50 registers per device)
- NVS storage for persistent configurations
- Register scaling and offset support
- Device status tracking (online/offline/error)
- Poll and error statistics

### 3. Modbus Manager (`modbus_manager.c/h`)
- UART1 driver for RS485 communication
- DE/RE pin control (separate GPIOs)
- Configurable baudrate (default: 9600, 19200, 38400, 115200)
- Automatic retry logic (3 attempts)
- Background polling task
- Read/write functions for all register types

### 4. Web Interface

#### Modbus Configuration Page (`modbus.html`)
- Add/remove devices
- Configure device settings (ID, name, poll interval, baudrate)
- Add/remove registers to devices
- Register configuration (address, type, name, unit, scale, offset, writable)
- Device presets for eAirMD and eWind
- Real-time device status display

#### Dashboard Page (`dashboard.html`)
- Real-time data display from all configured registers
- Value formatting with units and scaling
- Write capability for writable registers
- Auto-refresh functionality
- Device status indicators

#### REST API Endpoints
- `GET /api/modbus/devices` - List all devices
- `POST /api/modbus/devices` - Add new device
- `DELETE /api/modbus/devices/{id}` - Delete device
- `POST /api/modbus/registers` - Add register to device
- `DELETE /api/modbus/devices/{id}/registers/{addr}` - Delete register
- `POST /api/modbus/devices/{id}/write/{addr}` - Write register value

## Hardware Configuration

### XIAO ESP32C3 Pin Assignments
| Signal | GPIO | XIAO Pin | Description |
|--------|-------|------------|-------------|
| TX     | 21    | TX (Back)  | UART1 TX |
| RX     | 20    | RX (Back)  | UART1 RX |
| DE     | 7     | D5 (Front) | Driver Enable |
| RE     | 6     | D4 (Front) | Receiver Enable |

### RS485 Adapter (MAX485)
- Connect ESP32 TX → MAX485 DI
- Connect ESP32 RX → MAX485 RO
- Connect ESP32 DE (GPIO7) → MAX485 DE
- Connect ESP32 RE (GPIO6) → MAX485 RE
- Connect MAX485 A/B to RS485 bus
- Power MAX485 with 5V

## Supported Devices

### eAirMD (Enervent Pingvin)
- Device ID: 1-100
- Default Baudrate: 19200
- Preset includes common temperature and humidity registers
- Supports holding registers (0x03) only

### eWind (Kotilämpö)
- Device ID: 1-100
- Default Baudrate: 9600
- Preset includes common temperature, humidity, and control registers
- Supports coils (0x01) and holding registers (0x03)

## Usage

### Initial Setup
1. Flash firmware to ESP32-C3
2. Connect to WiFi via `/` page (or AP mode)
3. Navigate to `/modbus.html`
4. Add a device (use preset or manual configuration)
5. Add registers to monitor
6. Navigate to `/dashboard.html` to view real-time data

### Adding a Device Preset
1. Go to `/modbus.html`
2. Click "Presets" tab
3. Click "Add eAirMD" or "Add eWind"
4. Device and common registers are automatically configured

### Manual Device Configuration
1. Go to `/modbus.html`
2. Enter device details (ID, name, baudrate, etc.)
3. Click "Add Device"
4. Click "Add Register" for each register to monitor
5. Configure register (address, type, name, unit, scale, offset)
6. Save register
7. Go to `/dashboard.html` to view data

### Writing to Registers
1. On dashboard page, writable registers show input field
2. Enter new value (already scaled)
3. Click "Write"
4. Value is sent to device

## Data Flow

```
Web Dashboard
    ↓
Web Server (REST API)
    ↓
Modbus Devices Manager
    ↓ (polling)
Modbus Manager (UART1)
    ↓ (RS485)
HVAC Devices
```

## Files Created/Modified

### New Files
- `main/modbus_protocol.h` - Protocol layer header
- `main/modbus_protocol.c` - Protocol implementation
- `main/modbus_devices.h` - Device management header
- `main/modbus_devices.c` - Device management
- `main/modbus_manager.h` - Manager header
- `main/modbus_manager.c` - Manager with UART driver
- `main/html/modbus.html` - Configuration page
- `main/html/dashboard.html` - Dashboard page
- `main/html/modbus.js` - JavaScript

### Modified Files
- `main/main.c` - Initialize Modbus components
- `main/web_server.c` - Add Modbus API endpoints
- `main/CMakeLists.txt` - Include new source files
- `docs/PLAN.md` - Update implementation status
- `docs/devices/MAX485_RS485_Module.md` - Update DE/RE wiring

## Technical Details

### Communication Settings
- UART: UART_NUM_1
- Data bits: 8
- Parity: None
- Stop bits: 1
- Timeout: 1000ms
- Retry attempts: 3

### Polling
- Automatic background polling task
- Per-device configurable interval
- Individual register updates
- Statistics tracking (poll count, error count)

### Storage
- All configurations stored in NVS
- Namespace: `modbus_config`
- Automatically saved on changes
- Loaded on startup

### Error Handling
- CRC validation on all responses
- Exception detection and reporting
- Device status tracking
- Automatic retry for failed requests

## Testing Recommendations

1. **Hardware Test**
   - Verify RS485 connections
   - Check DE/RE pin control (should toggle during transmission)
   - Confirm device responds to simple read request

2. **Communication Test**
   - Add one register from device documentation
   - Verify value appears in dashboard
   - Check for CRC errors or timeouts

3. **Write Test**
   - Configure writable register (e.g., setpoint)
   - Write value via dashboard
   - Verify device accepts write

4. **Multiple Devices**
   - Add multiple devices with different IDs
   - Verify all devices are polled
   - Check for bus contention issues

5. **Long-term Test**
   - Monitor for 24+ hours
   - Check memory usage
   - Verify no memory leaks
   - Check polling reliability

## Future Enhancements

- MQTT integration for home automation
- WebSockets for real-time updates
- Historical data logging
- Graph/trend visualization
- Alarm threshold configuration
- Modbus TCP support
- User authentication
- Firmware OTA updates

## Documentation Updates

- Updated MAX485 wiring documentation (DE/RE on separate pins)
- Updated plan document with implementation status
- Created this implementation summary

## Notes

- All code follows ESP-IDF v5.1.6 conventions
- Uses FreeRTOS for task management
- Implements proper error handling throughout
- Ready for MQTT integration (data structures prepared)
- Compatible with standard Modbus RTU devices

---

**Implementation complete and ready for testing!**