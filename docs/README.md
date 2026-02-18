# ESP32-C3 Modbus RTU Gateway

A complete Modbus RTU gateway for ESP32-C3 microcontrollers with WiFi connectivity and modern web interface. Automatically creates an Access Point when no WiFi credentials are saved, allows easy configuration of WiFi and Modbus devices, and provides real-time monitoring of HVAC systems like Enervent Pingvin and Kotilämpö.

![ESP32-C3](https://img.shields.io/badge/ESP32--C3-RISC--V-blue)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.1.6-orange)
![Modbus](https://img.shields.io/badge/Modbus-RTU-green)
![License](https://img.shields.io/badge/License-MIT-green)

## Features

### Core Functionality

- ✅ **Automatic AP Mode** - Creates access point when no WiFi credentials saved
- ✅ **Modbus RTU Master** - Full Modbus RTU implementation over UART/RS485
- ✅ **Multi-Device Support** - Configure and monitor up to 4 Modbus devices
- ✅ **Web Configuration Interface** - Modern, responsive web page for setup
- ✅ **Real-Time Dashboard** - Live display of register values with auto-refresh
- ✅ **Credential Persistence** - Stores WiFi and device configs in flash memory (NVS)
- ✅ **Auto-Reconnect** - Automatically connects to WiFi and Modbus devices
- ✅ **Status Monitoring** - Real-time connection status and device health
- ✅ **Reconfiguration** - Easy credential clearing and device management
- ✅ **MQTT Support** - Home Assistant auto-discovery, remote control, and status

### Modbus Features

- 📡 **Modbus RTU Protocol** - Full master implementation
- 🏭 **HVAC Device Support** - Pre-configured for Enervent Pingvin and Kotilämpö
- 📊 **Register Read/Write** - Support for holding, input, and coil registers
- 🔄 **Automatic Polling** - Configurable polling intervals for each device
- 📈 **Data Caching** - Efficient register value caching
- 🎯 **Auto-Discovery** - Scan and detect connected Modbus devices
- ⚙️ **Flexible Configuration** - Customizable register mappings and scaling

### Web Interface

- 🎨 **Styled CSS** - Modern, card-based layout with smooth animations
- 📱 **Mobile-Friendly** - Responsive design works on phones and tablets
- 🎯 **User-Friendly** - Simple forms for WiFi and device configuration
- 💚 **Status Indicators** - Visual connection states (green/red indicators)
- 🔍 **Device Info** - Display register values with units and descriptions
- 📊 **Live Dashboard** - Real-time data refresh with configurable intervals
- 🎛️ **Control Panel** - Write to writable registers from web interface

### Technical Features

- ⚡ **ESP-IDF v5.1.6** - Latest Espressif IoT Development Framework
- 🎯 **ESP32-C3 Optimized** - Specifically designed for RISC-V based ESP32-C3
- 💾 **NVS Storage** - Reliable flash storage for all configurations
- 🌐 **HTTP Server** - Built-in web server on port 80
- 🔧 **REST API** - JSON API for programmatic access
- 📡 **Dual WiFi Mode** - AP mode for setup, Station mode for normal operation
- 🔄 **FreeRTOS Tasks** - Efficient multitasking for WiFi, Modbus, and web server
- 🔧 **Configurable** - Easy customization via source code

## Hardware Requirements

### Required Components

- **Microcontroller**: ESP32-C3 (Seeed Studio or compatible)
  
  - RISC-V single-core processor
  - 2.4 GHz Wi-Fi and Bluetooth 5 (LE)
  - 400 KB SRAM, 384 KB ROM
  - 4 MB Flash (or more)

- **USB Cable**: USB-C for power and programming

- **RS485 Adapter**: For Modbus RTU communication
  
  - UART to RS485 converter (e.g., MAX485)
  - Default pins: TX=GPIO4, RX=GPIO5 (configurable)

- **HVAC Devices**: (optional)
  
  - Enervent Pingvin ventilation system
  - Kotilämpö heating system
  - Any standard Modbus RTU slave device

- **Computer**: Windows, Linux, or macOS with development tools

### Supported Boards

#### ESP32-C3 Boards (Default)
- **Seeed Studio XIAO ESP32C3** - Development board (current target)
- **Espressif ESP32-C3-DevKitM-1** - Official dev kit
- Any ESP32-C3 based development board

**Specifications:**
- Architecture: RISC-V single-core
- Flash: 4MB
- RS485 Pins: TX=GPIO21, RX=GPIO20, DE=GPIO7, RE=GPIO6

#### WeAct ESP32-D0WD-V3 CAN485DevBoardV1
- **WeAct Studio CAN485DevBoardV1_ESP32** - Industrial development board

**Specifications:**
- Architecture: Xtensa dual-core
- Flash: 8MB
- Features: 2.5kV isolated CAN + RS485, TF Card, WS2812 LED
- RS485 Pins: TX=GPIO22, RX=GPIO21, DE=GPIO17 (DE/RE tied together)

**Documentation:** See `docs/devices/WeAct_CAN485DevBoardV1.md` for complete details.

#### Board Selection

By default, the project builds for ESP32-C3. To build for WeAct board:

```powershell
.\build-weact.ps1           # Quick WeAct build
.\build.ps1 -Board weact    # Universal build script
.\build.ps1 -Board weact -Clean -Flash -Monitor  # Full clean build with flash
```

To switch back to ESP32-C3:
```powershell
.\build-esp32c3.ps1
```

### Supported Modbus Devices

- Seeed Studio ESP32-C3 series
- Espressif ESP32-C3-DevKitM-1
- Any ESP32-C3 based development board
- Any ESP32 or ESP32-C3 based development board
- Any Modbus RTU compliant device

## Software Requirements

### Development Environment

- **ESP-IDF**: v5.1.6 or later
  
  - Download: [https://github.com/espressif/esp-idf](https://github.com/espressif/esp-idf)
  - Installation guide: [https://docs.espressif.com/projects/esp-idf](https://docs.espressif.com/projects/esp-idf)

- **Build Tools**:
  
  - Python 3.8+
  - CMake 3.16+
  - Ninja build system
  - Git

- **Tools** (installed with ESP-IDF):
  
  - Xtensa/RISC-V toolchains
  - esptool flash tool
  - ESP-IDF build system

## Installation

### Step 1: Install ESP-IDF

1. **Clone ESP-IDF Repository**
   
   ```bash
   cd C:\Users\jaakk\esp
   git clone --recursive https://github.com/espressif/esp-idf.git esp-idf-v5.1.6
   cd esp-idf-v5.1.6
   git checkout v5.1.6
   git submodule update --init --recursive
   ```

2. **Install ESP-IDF Tools**
   
   ```bash
   install.bat esp32c3
   ```
   
    This downloads compiler, flash tools, and utilities (~2GB, 10-20 minutes)

3. **Set Environment Variables**
   
    **Temporary (current session):**
   
   ```bash
   export.bat
   ```
   
    **Permanent (add to PATH):**
    Add to your system PATH:
   
   ```
   C:\Users\jaakk\esp\esp-idf-v5.1.6\esp-idf\tools
   ```

### Step 2: Clone This Project

```bash
cd "C:\Users\jaakk\DIY projects\Microcontroller codes"
git clone https://github.com/JaakkolaM/ESP32-modbus-reader.git
cd ESP32-modbus-reader
```

### Step 3: Configure Project

1. **Set Target to ESP32-C3**
   
   ```bash
   idf.py set-target esp32c3
   ```

2. **Optional Configuration**
   
   ```bash
   idf.py menuconfig
   ```
   
    Navigate to:
   
   - Component config → WiFi
   - Component config → HTTP Server
   - Component config → UART Driver

### Step 4: Build Project

```bash
idf.py build
```

The build process compiles the firmware and creates a binary file.

### Step 5: Flash to ESP32-C3

1. **Connect ESP32-C3** via USB-C cable

2. **Identify COM Port**
   
   - Windows: Device Manager → Ports (COM & LPT)
   - Example: COM3, COM4, etc.

3. **Flash Firmware**
   
   ```bash
   idf.py -p COM3 flash monitor
   ```
   
    This flashes the firmware and opens serial monitor.

4. **Monitor Only** (if already flashed)
   
   ```bash
   idf.py -p COM3 monitor
   ```

### Step 6: Connect Modbus Devices

1. Connect RS485 adapter to ESP32-C3:
   
   - TX pin → GPIO4 (default)
   - RX pin → GPIO5 (default)
   - GND → GND

2. Connect RS485 to HVAC devices:
   
   - A+ → A+
   - B- → B-
   - GND → GND (optional)

## Usage

### First-Time Setup

#### 1. Power On ESP32-C3

- Device starts up
- Checks for saved WiFi credentials in NVS
- No credentials found → Starts in **Access Point Mode**

#### 2. Connect to ESP32-C3 AP

- WiFi network: **`ESP32-Config`**
- Password: **`config123`**
- Connect your phone, tablet, or computer to this network

#### 3. Open Configuration Page

- In your browser, navigate to: **`http://192.168.4.1`**
- You'll see the WiFi configuration page

#### 4. Enter WiFi Credentials

- Enter your home WiFi network name (SSID)
- Enter your WiFi password
- Click **"Save & Connect"**

#### 5. Device Reboots & Connects

- Credentials saved to flash memory
- ESP32-C3 reboots
- Automatically connects to your WiFi network
- Note the IP address displayed in status

#### 6. Access Web Interface

- Connect your device to your home WiFi
- Navigate to: **`http://<device-ip-address>`**
- View WiFi status, configure Modbus devices

### Modbus Configuration

#### 1. Access Modbus Configuration

- Navigate to: **`http://<device-ip-address>/modbus.html`**
- Click "Add Device"

#### 2. Configure Device

- Enter Device ID (e.g., 1 for Enervent Pingvin)
- Enter Device Name (e.g., "Enervent Pingvin")
- Enter Description
- Set Poll Interval (default: 5000ms)
- Configure UART settings if needed
- Click "Save"

#### 3. Add Register Mappings

- Select device from list
- Add register addresses (refer to device documentation in `docs/devices/`)
- Set register type (Holding, Input, Coil)
- Enter name, unit, scaling factor
- Mark writable if applicable
- Click "Save"

#### 4. Test Communication

- Click "Test Connection" on device page
- Verify device responds
- Check register values in dashboard

#### 5. View Dashboard

- Navigate to: **`http://<device-ip-address>/dashboard.html`**
- View real-time register values
- Write to writable registers using control panel
- Monitor device status

### Reconfiguration

#### WiFi Credentials

If you need to change WiFi credentials:

1. Access web interface at device IP
2. Click "Clear Credentials"
3. Device reboots and creates AP again
4. Follow first-time setup steps

#### Modbus Configuration

If you need to change Modbus configuration:

1. Access Modbus configuration page
2. Edit device settings or register mappings
3. Changes take effect immediately (no reboot needed)
4. Test communication after changes

 ### API Usage

#### WiFi Status

```bash
curl http://<device-ip>/status
```

Response:

```json
{
  "connected": true,
  "mode": "STA",
  "ip": "192.168.1.100",
  "ssid": "MyHomeWiFi",
  "rssi": -45
}
```

#### List Modbus Devices

```bash
curl http://<device-ip>/api/modbus/devices
```

Response:

```json
{
  "devices": [
    {
      "id": 1,
      "name": "Enervent Pingvin",
      "enabled": true,
      "poll_interval": 5000,
      "status": "online"
    }
  ]
}
```

#### Add Device

```bash
curl -X POST http://<device-ip>/api/modbus/devices \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": 1,
    "name": "Enervent Pingvin",
    "description": "Ventilation unit",
    "poll_interval_ms": 5000,
    "baudrate": 19200,
    "enabled": true
  }'
```

#### Delete Device

```bash
curl -X DELETE "http://<device-ip>/api/modbus/devices?device_id=1"
```

#### Add Register

```bash
curl -X POST http://<device-ip>/api/modbus/registers \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": 1,
    "address": 1,
    "type": 3,
    "name": "Room Temp",
    "unit": "°C",
    "scale": 0.1,
    "offset": 0,
    "writable": false,
    "description": "Room temperature"
  }'
```

#### Delete Register

```bash
curl -X DELETE "http://<device-ip>/api/modbus/registers?device_id=1&address=1"
```

#### Write Register

```bash
curl -X POST "http://<device-ip>/api/modbus/write?device_id=1&address=1" \
  -H "Content-Type: application/json" \
  -d '{"value": 22.5}'
```

#### Read Registers

```bash
curl http://<device-ip>/api/modbus/1/read/holding/0x100/10
```

## Project Structure

```
ESP32-modbus-reader/
├── boards/                       # NEW: Board-specific configurations
│   ├── esp32c3_board.h         # ESP32-C3 pinouts and settings
│   ├── weact_esp32_board.h     # WeAct board pinouts and settings
│   └── board.h                # Board selection wrapper
├── CMakeLists.txt                 # Main CMake configuration
├── .gitignore                     # Git ignore rules
├── build.bat                      # Build script (Windows, backward compatibility)
├── build.ps1                      # NEW: Universal build script with parameters
├── build-esp32c3.ps1             # NEW: Quick ESP32-C3 build
├── build-weact.ps1                # NEW: Quick WeAct build
├── run_install.bat                # Install script
├── sdkconfig.esp32c3.defaults    # NEW: ESP32-C3 SDK config
├── sdkconfig.weact_esp32.defaults # NEW: WeAct SDK config
├── docs/
│   ├── PLAN.md                    # Detailed implementation plan
│   ├── README.md                  # This file
│   ├── BUILD_CHECKLIST.md         # Build verification checklist
│   └── devices/                  # Device documentation
│       ├── README.md              # Device documentation template
│       ├── WeAct_CAN485DevBoardV1.md # NEW: WeAct board documentation
│       ├── MAX485_RS485_Module.md # MAX485 TTL to RS485 module docs
│       ├── XIAO_ESP32C3_Getting_Started.md # XIAO ESP32C3 guide
│       ├── eairmd.md              # eAirMD device documentation
│       ├── ewind.md               # eWind device documentation
│       ├── eAirMD-modbus-register-list-public-clean.csv
│       ├── eAirMD-modbus-register-list-public-clean.xlsx
│       ├── eWind-modbus-register-list-public-clean.csv
│       ├── eWind-modbus-register-list-public-clean.xlsx
│       └── eWind-modbus-register-list-public-clean-enumerators.csv
├── main/
│   ├── CMakeLists.txt             # Main component CMake
│   ├── main.c                     # Application entry point
│   ├── wifi_manager.c             # WiFi implementation
│   ├── wifi_manager.h             # WiFi header
│   ├── web_server.c               # HTTP server implementation
│   ├── web_server.h               # HTTP server header
│   ├── nvs_storage.c              # NVS implementation
│   ├── nvs_storage.h              # NVS header
│   ├── modbus_manager.c           # Modbus RTU manager
│   ├── modbus_manager.h           # Modbus manager header
│   ├── modbus_protocol.c          # Modbus protocol stack
│   ├── modbus_protocol.h          # Modbus protocol header
│   ├── modbus_devices.c           # Device configuration manager
│   ├── modbus_devices.h           # Device configuration header
│   └── html/
│       ├── index.html             # Web UI HTML (WiFi config)
│       ├── style.css              # Web UI CSS
│       ├── script.js              # Web UI JavaScript
│       ├── modbus.html            # Modbus configuration page
│       ├── dashboard.html         # Data dashboard
│       └── modbus.js              # Modbus UI JavaScript
├── build/                         # Build output directory
├── sdkconfig                      # Project configuration
├── sdkconfig.old                  # Previous configuration backup
└── sdkconfig.defaults             # Default configuration
```

## Architecture

### System Components

```
┌─────────────────────────────────────────────────────┐
│              ESP32-C3 Modbus RTU Gateway            │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────┐    ┌──────────────┐               │
│  │  WiFi Manager│ ◄──┤  NVS Storage │               │
│  └──────┬───────┘    └──────────────┘               │
│         │                                           │
│         ▼                                           │
│  ┌──────────────┐    ┌──────────────┐               │
│  │   Web Server │ ◄──┤ Modbus Data  │               │
│  └──────┬───────┘    └──────┬───────┘               │
│         │                   │                       │
│         ▼                   ▼                       │
│  ┌──────────────┐    ┌──────────────────┐           │
│  │  FreeRTOS    │    │  Modbus Manager  │           │
│  └──────────────┘    └────────┬─────────┘           │
│                               │                     │
│                               ▼                     │
│                      ┌──────────────┐               │
│                      │  UART Driver │               │
│                      │  (Modbus RTU)│               │
│                      └──────────────┘               │
│                              │                      │
│                              ▼                      │
│                     RS485/UART (HVAC Devices)       │
└─────────────────────────────────────────────────────┘
```

### Component Descriptions

#### WiFi Manager

- Manages WiFi mode switching (AP/Station)
- Handles connection events
- Implements connection retry logic
- Provides status to web server

#### Web Server

- HTTP server on port 80
- Serves static files (HTML, CSS, JS)
- Handles form submissions
- Provides REST API for Modbus operations

#### NVS Storage

- Non-Volatile Storage for WiFi and device configs
- Persistent across reboots
- Thread-safe operations

#### Modbus Manager

- Modbus RTU master implementation
- Device configuration management
- Register caching and polling
- Error handling and retry logic

#### Modbus Protocol

- Full Modbus RTU protocol stack
- CRC calculation and validation
- Support for functions 0x01, 0x03, 0x04, 0x05, 0x06, 0x15, 0x16

#### Web Interface

- Responsive HTML/CSS/JavaScript
- AJAX-powered status updates
- Real-time dashboard with auto-refresh
- Device configuration and control panels

## Development

### Building from Source

#### Building for Specific Boards

##### ESP32-C3 (Default)
```powershell
.\build-esp32c3.ps1
```

Or manual:
```bash
idf.py set-target esp32c3
idf.py build
```

##### WeAct ESP32-D0WD-V3
```powershell
.\build-weact.ps1
```

Or manual:
```bash
idf.py set-target esp32
idf.py build
```

#### Build Options

Using the universal build script with parameters:

```powershell
# Incremental build (fast, keeps cached files)
.\build.ps1 -Board esp32c3

# Full clean build
.\build.ps1 -Board weact -Clean

# Build and flash
.\build.ps1 -Board weact -Flash

# Build, flash and monitor
.\build.ps1 -Board weact -Flash -Monitor

# Specify COM port
.\build.ps1 -Board weact -Flash -Port COM3
```

**Universal Script Parameters:**
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

#### Manual Build Steps

If you prefer manual builds:

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

5. **Flash and Monitor**:
   ```bash
   idf.py -p COM3 flash monitor
   ```

### Debugging

Enable verbose logging:

```bash
idf.py menuconfig
```

Navigate to: Component config → Log output → Default log verbosity → Debug

### Debugging

Enable verbose logging:

```bash
idf.py menuconfig
```

Navigate to: Component config → Log output → Default log verbosity → Debug

### Code Style

This project follows ESP-IDF coding conventions:

- K&R indentation style
- 4 spaces per indentation
- CamelCase for function names
- snake_case for variables

#### NVS Data Persistence Issues

**Problem:** Device configuration lost after power cycle

**Solution:**

The v1.3.1 release fixed NVS data persistence by shortening key names:
- ESP32 NVS has maximum key length of 15 characters
- Previous keys like `device_0_poll_interval` (22 chars) failed with `ESP_ERR_NVS_KEY_TOO_LONG`
- All keys shortened to ≤15 chars: `d%d_id`, `d%d_name`, `d%d_rc`, `d%dr%da`, etc.
- All NVS read operations now check return values
- Safe default values applied when reads fail
- Detailed logging shows which data loaded successfully vs failed
- Device configurations now persist correctly across reboots

**Verification:**
1. Add a device via web interface
2. Power cycle ESP32
3. Check serial monitor for: `Loaded X device(s) from NVS successfully`
4. Verify device configuration appears correctly in web interface

If you still see data loss:
- Check for `ESP_ERR_NVS_KEY_TOO_LONG` errors in serial monitor
- Check for warning logs: `W (XXXXX) MODBUS_DEVICES: Failed to read...`
- Verify NVS partition size in sdkconfig (default 12KB should be sufficient)
- Try clearing and reconfiguring: Click "Clear Devices" button in web interface

---

#### ESP-IDF Command Not Found

**Problem:** `idf.py` command not recognized

**Solution:**

```bash
cd C:\Users\jaakk\esp\esp-idf-v5.1.6\esp-idf
export.bat
```

#### Modbus Device Not Responding

**Problem:** Cannot communicate with Modbus device

**Solutions:**

- Check RS485 wiring (A+ to A+, B- to B-)
- Verify device ID matches configuration
- Check baudrate settings
- Ensure proper grounding
- Test with Modbus simulator or known working device
- Review serial logs for error messages

#### WiFi Not Connecting

**Problem:** Device fails to connect to WiFi

**Solutions:**

- Verify SSID and password are correct
- Check WiFi signal strength
- Ensure 2.4GHz band (ESP32 doesn't support 5GHz)
- Check router security (WPA2/WPA3)
- Review serial logs for error messages

#### Dashboard Not Updating

**Problem:** Register values not refreshing

**Solutions:**

- Verify device is online in Modbus configuration
- Check polling interval is set correctly
- Refresh browser page
- Check browser console for JavaScript errors
- Review serial logs for Modbus errors

---

#### Cannot Add Registers with Same Address

**Problem:** "Register address already exists" error when trying to add DI and DO registers with same address

**Solution:**

This is normal behavior. The system now allows the same address for different register types:
- **DI (Discrete Inputs, Type 2)**: Use address 0-7 for DI1-DI8
- **DO (Coils, Type 1)**: Use address 0-7 for DO1-DO8

Both can use the same addresses because they use different Modbus function codes:
- DI uses Function Code 0x02 (Read Discrete Inputs)
- DO uses Function Code 0x01 (Read Coils)

This is valid per Modbus standard - coils and discrete inputs are separate memory areas.

---

#### Modbus Communication Debugging

**Current Status:** The system logs Modbus communication failures (timeouts, CRC errors, exceptions) but does not log successful read/write operations or frame details.

**Planned Enhancement:** Future version will add comprehensive communication logging:

- Send frame details (device ID, function code, address, quantity, byte count)
- Receive frame details (bytes received, device ID, function code, byte count)
- Hex dump of full Modbus frames for protocol analysis
- Retry attempt tracking with per-attempt results
- Timing information (TX duration, wait time, RX duration)

**Example Planned Output:**
```
I (12345) MODBUS_MANAGER: SENT: DevID=1, FC=0x02, Addr=0, Qty=1, Bytes=8
I (12345) MODBUS_MANAGER: SENDING: 01 02 00 00 01 89 CA
I (12345) MODBUS_MANAGER: RECV: 8 bytes, DevID=1, FC=0x02, ByteCount=1
I (12345) MODBUS_MANAGER: RECEIVED: 01 02 01 01 79 C8
I (12345) MODBUS_MANAGER: ATTEMPT 1/3: DevID=1, FC=0x02, Addr=0, Result=OK
```

**Debugging with Current Version:**
- Check for warning logs: `W (XXXXX) MODBUS_MANAGER: Timeout waiting for response`
- Check for error logs: `E (XXXXX) MODBUS_PROTOCOL: CRC validation failed`
- Monitor error counts in web interface (Polls vs Errors)
- Verify startup logs show: `I (0) MODBUS_MANAGER: UART initialized`

### Getting Help

- Check [docs/PLAN.md](docs/PLAN.md) for detailed implementation notes
- Check [docs/devices/](docs/devices/) for device-specific documentation
- Review ESP-IDF documentation: https://docs.espressif.com/projects/esp-idf
- Check ESP32-C3 datasheet for hardware details
- Search GitHub Issues for similar problems

## Roadmap

### Version 1.4.2 (Current - 2026-02-17)

- ✅ Fixed MQTT write functionality with proper subscription to set topics
- ✅ Added FreeRTOS mutex to prevent Modbus bus contention between polling and MQTT writes
- ✅ Implemented mqtt_write_callback() to handle Home Assistant write commands
- ✅ Fixed ON/OFF string parsing for coil switches in MQTT messages
- ✅ Fixed MQTT client start when first enabled via web UI
- ✅ Fixed form values being overwritten by periodic status refresh
- ✅ MQTT support with Home Assistant auto-discovery
- ✅ MQTT configuration page at `/mqtt`
- ✅ Configurable broker, port, username, password, topic prefix
- ✅ Automatic device discovery for sensors, switches, and numbers
- ✅ Publish on polling intervals and on value changes
- ✅ Write capability via MQTT set commands
- ✅ Auto-reconnect on broker disconnect
- ✅ MQTT status indicator on web pages
- ✅ Custom partition table for larger app (2MB factory partition)
- ✅ Fixed HTTP server URI handler limit
- 📝 Planned: Historical data logging

### Version 1.4.1 (2026-02-17)

- ✅ Fixed MQTT client start and form refresh issues
- ✅ Separate loadMqttStatus() function for status-only updates to prevent form overwrite

### Version 1.1 (2025-01-29)

- ✅ WiFi manager functionality
- ✅ Web interface with styled CSS
- ✅ NVS credential storage
- ✅ AP mode for configuration
- ✅ Station mode for normal operation
- ✅ Modbus RTU protocol implementation
- ✅ Modbus Manager component
- ✅ Device configuration management
- ✅ Register read/write functionality
- ✅ Web dashboard for real-time data
- ✅ REST API for Modbus operations
- ✅ Support for eAirMD (Enervent Pingvin)
- ✅ Support for eWind (Kotilämpö)
- ✅ Fixed wildcard URI handler conflicts
- ✅ Added navigation between all web pages

### Version 1.3 (2026-02-16)

- ✅ Fixed NVS data persistence bug in modbus_devices_load()
- ✅ Added comprehensive error handling for all NVS read operations
- ✅ Added safe default values when NVS reads fail
- ✅ Added detailed logging for NVS load operations
- ✅ Fixed corrupted modbus_devices_load() function with duplicate code
- ✅ Fixed missing closing quotes in log messages
- ✅ Fixed undefined variables (devices_loaded, OK)
- ✅ Verified all ESP_OK constants are properly used
- [ ] MQTT integration
- [ ] Historical data logging
- [ ] Alarms and alerts
- [ ] Captive portal support
- [ ] WiFi network scanning
- [ ] Multiple network profiles

### Version 2.0 (Future)

- [ ] OTA firmware updates
- [ ] Authentication for web interface
- [ ] Dark mode toggle
- [ ] Mobile app companion
- [ ] WebSocket real-time updates
- [ ] Static IP configuration
- [ ] Custom DNS settings
- [ ] Data export (CSV/JSON)
- [ ] Trend graphs and analytics
- [ ] Modbus TCP support
- [ ] Custom automation scripts

## Device Documentation

### Ebyte M31-AXAX8080G (8DI+8DO) Configuration

The Ebyte M31-AXAX8080G is a distributed I/O module with 8 digital inputs and 8 digital outputs, supporting Modbus RTU protocol.

**Device Specifications:**
- **DI Inputs**: 8 channels (DI1-DI8)
- **DO Outputs**: 8 channels (DO1-DO8)
- **Protocol**: Modbus RTU
- **Default Baudrate**: 9600 bps (configurable via register 0x7530)
- **Default Device ID**: 1 (hardware DIP + software offset)
- **Power**: 9-36V DC

**Register Configuration for ESP32:**

**DI (Digital Inputs) - Read-Only Status:**
| Register | Type | Address Range | Function Code | Notes |
|----------|-------|---------------|---------------|--------|
| DI1-DI8 | Discrete Input (2) | 0-7 | 0x02 | 12-24V input, read status only |
| DI Filter | Holding Register (3) | 0x0DA3 | 0x03 | Filter cycles 1-16, default 6 |

**DO (Digital Outputs) - Read/Write Control:**
| Register | Type | Address Range | Function Code | Notes |
|----------|-------|---------------|---------------|--------|
| DO1-DO8 | Coil (1) | 0-7 | 0x01 | Relay control, writable |
| DO All On/Off | Coil (1) | 0x0000 | 0x0F | Control multiple DOs |

**Setup Steps:**

1. **Add M31 Device:**
   - Device ID: 1 (or configured value)
   - Baudrate: 9600
   - Poll Interval: 5000ms (5 seconds)

2. **Add DI Registers (Inputs):**
   - Type: Discrete Input (Type 2)
   - Addresses: 0, 1, 2, 3, 4, 5, 6, 7 (one per DI)
   - Names: DI1, DI2, ... DI8
   - Writable: No (read-only)

3. **Add DO Registers (Outputs):**
   - Type: Coil (Type 1)
   - Addresses: 0, 1, 2, 3, 4, 5, 6, 7 (one per DO)
   - Names: DO1, DO2, ... DO8
   - Writable: Yes (allows relay control)

**Important Notes:**
- DI and DO can use the SAME addresses (0-7) because they are different Modbus memory areas
- DI uses Function Code 0x02 (Read Discrete Inputs)
- DO uses Function Code 0x01 (Read Coils)
- DO relays are normally-open (close on write value=1)
- DI inputs accept 12-24V DC, NPN or PNP active
- DI green LEDs on M31 indicate valid input (on) vs invalid (off)
- DO green LEDs on M31 indicate relay closed (on) vs open (off)

**Full Documentation:**
See `docs/devices/M31_Series_User_Manual.md` for complete register map, configuration options, and troubleshooting.

**Known Issues:**
- Version 1.2 fixed DI/DO same address restriction - you can now add both types at address 0-7
- Modbus communication logs will be enhanced in next update for better debugging

---

### Other Device Documentation

Add your device-specific documentation to the `docs/devices/` folder:

- **Enervent Pingvin**: Add register maps, communication details to `enervent_pingvin.md`
- **Kotilämpö**: Add register maps, communication details to `kotilampo.md`

See `docs/devices/README.md` for the documentation template.

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork Repository**
   
   ```bash
   git clone https://github.com/JaakkolaM/ESP32-modbus-reader.git
   cd ESP32-modbus-reader
   ```

2. **Create Feature Branch**
   
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make Changes**
   
   - Follow existing code style
   - Add comments for complex logic
   - Update documentation as needed

4. **Test Thoroughly**
   
   ```bash
   idf.py build
   idf.py -p COM3 flash monitor
   ```

5. **Commit Changes**
   
   ```bash
   git add .
   git commit -m "Add your feature description"
   ```

6. **Push and Create Pull Request**
   
   ```bash
   git push origin feature/your-feature-name
   ```

## Documentation

- **[Implementation Plan](docs/PLAN.md)** - Detailed technical design and implementation notes
- **[Device Documentation](docs/devices/)** - HVAC device register maps and specifications
- **[ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/v5.1.6/esp32c3/)** - Official ESP-IDF docs
- **[ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)** - Hardware specifications

## License

This project is licensed under MIT License - see [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Espressif Systems** - For the ESP32-C3 chip and ESP-IDF framework
- **Seeed Studio** - For ESP32-C3 development boards
- **ESP-IDF Community** - For excellent documentation and examples

## Support & Contact

- **Issues**: [GitHub Issues](https://github.com/JaakkolaM/ESP32-modbus-reader/issues)
- **Discussions**: [GitHub Discussions](https://github.com/JaakkolaM/ESP32-modbus-reader/discussions)

## Performance

### Memory Usage

- Free Heap: ~280KB (ESP32-C3)
- WiFi Stack: ~40KB
- HTTP Server: ~20KB
- Modbus Protocol: ~15KB
- Modbus Manager: ~10KB
- Device Configs: ~5KB (4 devices)
- Register Cache: ~10KB (dynamic)
- Application: ~10KB
- Available: ~170KB

### Power Consumption

- Active (WiFi ON, Modbus ON): ~100mA
- Active (WiFi ON, Modbus IDLE): ~85mA
- Light Sleep: ~15mA
- Deep Sleep: ~10µA

### Network Performance

- Connection time: 2-5 seconds
- AP mode startup: 1-2 seconds
- HTTP request: <100ms
- Page load: <500ms
- Modbus RTU read (single): ~50ms
- Modbus RTU read (multiple): ~100-200ms
- Modbus RTU write: ~50ms
- Dashboard refresh: ~500-1000ms

  ## Changelog

### Version 1.4.2 (2026-02-17)

- Fixed MQTT write functionality by adding proper subscription to set topics
- Implemented mqtt_write_callback() to handle Home Assistant write commands from MQTT
- Added FreeRTOS mutex (modbus_transaction_mutex) to serialize Modbus transactions
- Fixed ON/OFF string parsing for coil switches (case-insensitive comparison)
- Fixed MQTT client start when first enabled via web UI
- Prevents polling and MQTT writes from colliding on RS485 bus
- Updated AGENTS.md with MQTT integration guidelines

### Version 1.4.1 (2026-02-17)

- Fixed mqtt_client_update_config to start MQTT when first enabled via web UI
- Fixed form values being overwritten by periodic status refresh in MQTT config page
- Added separate loadMqttStatus() function for status-only updates
- Note: MQTT connection fix not yet tested

### Version 1.4.0 (2026-02-17)

- Added MQTT client support with configurable broker, port, username, password
- Added Home Assistant auto-discovery for all configured registers
- Added MQTT configuration page at `/mqtt` with enable/disable toggle
- Added automatic publish on polling intervals and on value changes
- Added write capability via MQTT set commands
- Added Last Will Testament (LWT) for availability tracking
- Added MQTT status indicator on all web pages
- Renamed mqtt_client.c/h to mqtt_gateway.c/h to avoid ESP-IDF header conflict
- Added custom partition table (partitions.csv) with 2MB factory partition
- Configured 4MB flash size for ESP32-C3
- Increased HTTP server URI handlers from 20 to 24
- Added esp_wifi and esp_http_server dependencies to CMakeLists.txt

### Version 1.3.1 (2026-02-16)

- Fixed NVS key names too long causing ESP_ERR_NVS_KEY_TOO_LONG
- Shortened all NVS keys to ≤15 characters (ESP32 limit: `d%d_id`, `d%d_name`, etc.)
- Fixed all device and register persistence across reboots
- Updated AGENTS.md with NVS key length warning
- Updated documentation with key abbreviation scheme

### Version 1.2.0 (2025-02-03)

- Added comprehensive input validation to all API handlers
- Improved error messages with specific validation feedback
- Fixed REST API endpoint conflicts by switching from wildcard to query parameters
- Updated frontend JavaScript to use query parameter-based API calls
- Enhanced device creation validation (device_id range 1-247, baudrate validation, poll interval limits)
- Enhanced register creation validation (address range 0-65535, type validation, required fields)
- Enhanced write operation validation (value field required)
- Fixed DELETE device endpoint: `DELETE /api/modbus/devices?device_id=X`
- Fixed DELETE register endpoint: `DELETE /api/modbus/registers?device_id=X&address=Y`
- Fixed write register endpoint: `POST /api/modbus/write?device_id=X&address=Y`
- Removed wildcard URI matching which caused server crashes on ESP-IDF v5.1.6
- Known issue: Adding registers still experiences intermittent errors (addressed in v1.3)

### Version 1.1.0 (2025-01-29)

- Complete Modbus RTU implementation with read/write support
- Added web-based Modbus device configuration page (`/modbus.html`)
- Added real-time data dashboard page (`/dashboard.html`)
- Implemented REST API for Modbus operations
- Added support for eAirMD and eWind HVAC devices with presets
- Fixed wildcard URI handler conflicts causing server crashes
- Replaced wildcard with specific file handlers (15 total, within 16 limit)
- Added navigation buttons to WiFi config page
- Fixed navigation between all web pages
- Updated documentation to reflect completed features

### Version 1.0.1 (2025-01-27)

- Updated README for Modbus RTU gateway functionality
- Added comprehensive project plan for Modbus implementation
- Created device documentation structure

### Version 1.0.0 (2025-01-26)

- Initial release
- WiFi manager with AP/Station modes
- Web interface with styled CSS
- NVS credential storage
- Complete documentation

---

**Made with ❤️ using ESP32-C3 and ESP-IDF**

![ESP-IDF](https://img.shields.io/badge/Powered%20by-ESP--IDF-blue)
