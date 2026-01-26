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

### Supported Devices
- Seeed Studio ESP32-C3 series
- Espressif ESP32-C3-DevKitM-1
- Any ESP32-C3 based development board
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

#### Read Registers
```bash
curl http://<device-ip>/api/modbus/1/read/holding/0x100/10
```

#### Write Register
```bash
curl -X POST http://<device-ip>/api/modbus/1/write/holding/0x100 -d "value=20.5"
```

## Project Structure

```
ESP32-modbus-reader/
├── CMakeLists.txt                 # Main CMake configuration
├── docs/
│   ├── PLAN.md                    # Detailed implementation plan
│   ├── README.md                  # This file
│   └── devices/                  # Device documentation
│       ├── README.md              # Device docs template
│       ├── enervent_pingvin.md    # Enervent Pingvin docs
│       └── kotilampo.md          # Kotilämpö docs
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
├── sdkconfig                      # Project configuration
├── sdkconfig.defaults             # Default configuration
└── .gitignore                     # Git ignore rules
```

## Architecture

### System Components

```
┌─────────────────────────────────────────────────────┐
│              ESP32-C3 Modbus RTU Gateway          │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────┐    ┌──────────────┐               │
│  │  WiFi Manager │◄──┤  NVS Storage │               │
│  └──────┬───────┘    └──────────────┘               │
│         │                                           │
│         ▼                                           │
│  ┌──────────────┐    ┌──────────────┐               │
│  │   Web Server │◄──┤ Modbus Data  │               │
│  └──────┬───────┘    └──────┬───────┘               │
│         │                   │                       │
│         ▼                   ▼                       │
│  ┌──────────────┐    ┌──────────────────┐          │
│  │  FreeRTOS    │    │  Modbus Manager  │          │
│  └──────────────┘    └────────┬─────────┘          │
│                              │                      │
│                              ▼                      │
│                      ┌──────────────┐               │
│                      │  UART Driver │               │
│                      │  (Modbus RTU)│               │
│                      └──────────────┘               │
│                              │                      │
│                              ▼                      │
│                     RS485/UART (HVAC Devices)        │
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

```bash
# Clean previous build
idf.py fullclean

# Set target
idf.py set-target esp32c3

# Build
idf.py build

# Flash and monitor
idf.py -p COM3 flash monitor
```

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

## Troubleshooting

### Common Issues

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

### Getting Help

- Check [docs/PLAN.md](docs/PLAN.md) for detailed implementation notes
- Check [docs/devices/](docs/devices/) for device-specific documentation
- Review ESP-IDF documentation: https://docs.espressif.com/projects/esp-idf
- Check ESP32-C3 datasheet for hardware details
- Search GitHub Issues for similar problems

## Roadmap

### Version 1.0 (Current)
- ✅ WiFi manager functionality
- ✅ Web interface with styled CSS
- ✅ NVS credential storage
- ✅ AP mode for configuration
- ✅ Station mode for normal operation
- ✅ Basic web interface

### Version 1.1 (In Development)
- [ ] Modbus RTU protocol implementation
- [ ] Modbus Manager component
- [ ] Device configuration management
- [ ] Register read/write functionality
- [ ] Web dashboard for real-time data
- [ ] REST API for Modbus operations
- [ ] Support for Enervent Pingvin
- [ ] Support for Kotilämpö

### Version 1.2 (Planned)
- [ ] MQTT integration
- [ ] Historical data logging
- [ ] Alarms and alerts
- [ ] Captive portal support
- [ ] WiFi network scanning
- [ ] Multiple network profiles
- [ ] Enhanced error handling

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
