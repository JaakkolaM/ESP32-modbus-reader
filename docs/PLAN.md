# ESP32-C3 Modbus RTU Gateway - Implementation Plan

> **Living Document** - Updated as project progresses
> 
> Last Updated: 2025-01-27

## Project Overview

Create a Modbus RTU gateway for ESP32-C3 from Seeed Studio that:

- Automatically creates an Access Point (AP) when no WiFi credentials are saved
- Hosts a styled web page for entering WiFi credentials
- Saves credentials to Non-Volatile Storage (NVS)
- Connects to saved WiFi network
- Communicates with Modbus RTU devices (Enervent Pingvin, Kotilämpö) via UART
- Provides web interface for monitoring and controlling Modbus registers
- Displays real-time data in browser dashboard
- Prepares data structure for future MQTT integration
- Allows reconfiguration via web interface at device IP address

## Current Status

- [x] Environment check completed
- [x] ESP-IDF v5.1.6 found at `C:\Users\jaakk\esp\v5.1.6\esp-idf`
- [x] Arduino CLI with ESP32 v3.2.0 package detected
- [x] esptool available for flashing
- [x] Python 3.13.1 installed
- [x] ESP32-C3 support confirmed
- [x] WiFi Manager implementation
- [x] Web Server implementation
- [x] NVS Storage implementation
- [x] Web UI (HTML + Styled CSS)
- [x] Build and test on ESP32-C3
- [x] Documentation files created
- [x] Project structure created
- [x] Pushed to GitHub
- [ ] Modbus Manager implementation
- [ ] Modbus RTU protocol implementation
- [ ] Modbus device configuration
- [ ] Dashboard web interface
- [ ] Register read/write functionality
- [ ] MQTT integration (future)

## Hardware Requirements

- **Microcontroller**: ESP32-C3 (Seeed Studio)
- **USB Cable**: For power and programming
- **Computer**: Windows with development tools

## Software Requirements

- **ESP-IDF**: v5.1.6+ (located at `C:\Users\jaakk\esp\v5.1.6\esp-idf`)
- **Python**: 3.13.1 (installed)
- **Git**: Installed
- **CMake**: Installed
- **esptool**: Available via Arduino IDE installation

## Architecture

### System Components

```
┌─────────────────────────────────────────────────────┐
│                 ESP32-C3 Application                │
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

### Component Details

#### 1. WiFi Manager (`wifi_manager.c/.h`)

**Responsibilities:**

- Initialize WiFi driver and event loop
- Check NVS for saved credentials
- Manage WiFi mode (AP vs Station)
- Handle connection state transitions
- Provide status to web server

**Functions:**

- `wifi_manager_init()` - Initialize WiFi driver and event handler
- `wifi_manager_start()` - Start WiFi operations based on credentials
- `wifi_manager_connect_ap(ssid, password)` - Connect to AP
- `wifi_manager_start_ap()` - Start AP mode
- `wifi_manager_get_status()` - Get current connection status
- `wifi_manager_event_handler()` - Handle WiFi events

**State Machine:**

```
INIT → Check NVS → Credentials Found? 
                     ├── Yes → Connect to Station → Success/Monitor
                     └── No → Start AP Mode → Wait for Credentials
```

**AP Mode Configuration:**

- SSID: `ESP32-Config`
- Password: `config123`
- IP: `192.168.4.1`
- Channel: 1
- Max connections: 4

**Station Mode:**

- Auto-connect on startup
- Connection timeout: 30 seconds
- Fallback to AP mode on failure

#### 2. Web Server (`web_server.c/.h`)

**Responsibilities:**

- Initialize HTTP server on port 80
- Handle HTTP requests
- Serve static files (HTML, CSS, JS)
- Process form submissions
- Provide API endpoints

**Endpoints:**

##### WiFi Management
- `GET /` - Serve WiFi configuration page
- `GET /status` - JSON: connection status, IP, SSID
- `POST /save` - Save credentials and reboot
- `POST /clear` - Clear credentials and reboot
- `GET /style.css` - Serve CSS file
- `GET /script.js` - Serve JavaScript

##### Modbus Management
- `GET /modbus.html` - Serve Modbus configuration page
- `GET /dashboard.html` - Serve dashboard page
- `GET /modbus.js` - Serve Modbus JavaScript

##### Modbus API
- `GET /api/modbus/devices` - List all configured devices
- `POST /api/modbus/devices` - Add new device (JSON: device_id, name, poll_interval)
- `DELETE /api/modbus/devices/{id}` - Remove device
- `GET /api/modbus/scan` - Scan for devices (returns active IDs)
- `GET /api/modbus/{device_id}/registers` - Read all cached registers
- `GET /api/modbus/{device_id}/read/holding/{address}/{count}` - Read holding registers
- `GET /api/modbus/{device_id}/read/input/{address}/{count}` - Read input registers
- `GET /api/modbus/{device_id}/read/coil/{address}/{count}` - Read coils
- `POST /api/modbus/{device_id}/write/holding/{address}` - Write single holding register
- `POST /api/modbus/{device_id}/write/coil/{address}` - Write single coil
- `POST /api/modbus/{device_id}/write/multiple/{address}` - Write multiple registers
- `GET /api/modbus/status` - Get Modbus connection status (all devices)

**Response Types:**

- HTML: `text/html`
- CSS: `text/css`
- JSON: `application/json`

**Error Handling:**

- 404 for unknown routes
- 500 for server errors
- Validation of SSID/password format

#### 3. NVS Storage (`nvs_storage.c/.h`)

**Responsibilities:**

- Initialize NVS flash storage
- Save/retrieve WiFi credentials
- Clear credentials
- Handle storage errors

**Functions:**

- `nvs_storage_init()` - Initialize NVS
- `nvs_save_wifi_credentials(ssid, password)` - Save credentials
- `nvs_load_wifi_credentials(ssid, password)` - Load credentials
- `nvs_clear_wifi_credentials()` - Delete credentials
- `nvs_has_credentials()` - Check if credentials exist

**Storage Details:**

- Namespace: `wifi_config`
- Keys: `ssid`, `password`
- Type: String
- Max length: SSID (32 chars), Password (64 chars)

#### 4. Web UI (`html/`)

**HTML Structure (`index.html`):**

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Configuration</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>ESP32 WiFi Manager</h1>
            <div class="status-indicator" id="status">
                <span class="status-dot"></span>
                <span class="status-text">Disconnected</span>
            </div>
        </header>

        <main>
            <section class="card">
                <h2>WiFi Configuration</h2>
                <form id="wifi-form">
                    <div class="form-group">
                        <label for="ssid">SSID</label>
                        <input type="text" id="ssid" name="ssid" required
                               placeholder="Enter network name" maxlength="32">
                    </div>
                    <div class="form-group">
                        <label for="password">Password</label>
                        <input type="password" id="password" name="password"
                               placeholder="Enter password" maxlength="64">
                    </div>
                    <button type="submit" class="btn btn-primary">Save & Connect</button>
                    <button type="button" class="btn btn-secondary" id="clear-btn">Clear Credentials</button>
                </form>
            </section>

            <section class="card">
                <h2>Network Information</h2>
                <div class="info-item">
                    <span class="label">Current Mode:</span>
                    <span class="value" id="mode">Unknown</span>
                </div>
                <div class="info-item">
                    <span class="label">IP Address:</span>
                    <span class="value" id="ip">--</span>
                </div>
                <div class="info-item">
                    <span class="label">Connected SSID:</span>
                    <span class="value" id="current-ssid">--</span>
                </div>
            </section>
        </main>

        <footer>
            <p>ESP32-C3 WiFi Manager</p>
        </footer>
    </div>
    <script src="/script.js"></script>
</body>
</html>
```

**CSS Styling (`style.css`):**

- Modern, responsive design
- Color scheme: Blue (#2563eb), White (#ffffff), Gray (#f3f4f6)
- Card-based layout with shadows
- Smooth transitions and animations
- Mobile-first approach
- Status indicators (green/red)
- Form styling with focus states

#### 5. Modbus Manager (`modbus_manager.c/.h`)

**Responsibilities:**

- Initialize UART for Modbus RTU communication
- Implement Modbus master protocol (functions 0x01, 0x03, 0x04, 0x05, 0x06, 0x15, 0x16)
- Read/write registers from HVAC devices
- Manage multiple Modbus device configurations
- Store device configs and register mappings in NVS
- Provide real-time data to web server
- Prepare data structures for MQTT integration

**Configuration:**

- UART: TX/RX pins (configurable, default: GPIO4=TX, GPIO5=RX)
- Baudrate: 9600, 19200, 38400 (selectable, default: 9600)
- Data bits: 8
- Stop bits: 1
- Parity: None
- Max devices: 4
- Timeout: 1000ms per request
- Retry attempts: 3

**Supported Modbus Functions:**

- 0x01: Read Coils
- 0x03: Read Holding Registers
- 0x04: Read Input Registers
- 0x05: Write Single Coil
- 0x06: Write Single Register
- 0x15: Write Multiple Coils
- 0x16: Write Multiple Registers

**Functions:**

- `modbus_manager_init()` - Initialize Modbus stack and UART
- `modbus_read_holding_registers(device_id, address, count, values)` - Read holding registers
- `modbus_write_holding_register(device_id, address, value)` - Write single register
- `modbus_write_multiple_registers(device_id, address, values, count)` - Write multiple
- `modbus_read_input_registers(device_id, address, count, values)` - Read input registers
- `modbus_read_coils(device_id, address, count, values)` - Read coils
- `modbus_write_coil(device_id, address, value)` - Write coil
- `modbus_scan_devices()` - Auto-detect connected devices
- `modbus_get_device_status(device_id)` - Get device connection status

**Supported Devices:**

- Enervent Pingvin (HVAC ventilation system)
- Kotilämpö (Heating system)
- Any standard Modbus RTU slave device

**Data Flow:**

```
Web Server → Modbus Manager → UART Driver → RS485 Bus → HVAC Devices
    ↓              ↓
   API          Register Cache
                (Shared Memory)
```

#### 6. Modbus Protocol (`modbus_protocol.c/.h`)

**Responsibilities:**

- Implement Modbus RTU frame format
- CRC calculation and validation
- Frame parsing and generation
- Timeout handling
- Retry logic
- Error code mapping

**Functions:**

- `modbus_build_request_frame()` - Build Modbus request frame
- `modbus_build_response_frame()` - Build Modbus response frame
- `modbus_parse_response()` - Parse received response
- `modbus_calculate_crc()` - Calculate CRC16 checksum
- `modbus_validate_crc()` - Validate CRC in received frame
- `modbus_exception_to_string()` - Convert exception codes to strings

#### 7. Modbus Devices (`modbus_devices.c/.h`)

**Responsibilities:**

- Manage device configuration
- Store device settings in NVS
- Maintain register mappings
- Cache register values
- Handle device status and health monitoring

**Device Configuration Structure:**

```c
typedef struct {
    uint8_t device_id;              // Modbus device ID
    char name[32];                  // Device name
    char description[64];           // Description
    uint32_t poll_interval;         // Poll interval in milliseconds
    bool enabled;                   // Device enabled/disabled
    uint8_t last_error;             // Last error code
    time_t last_seen;               // Last successful communication
} modbus_device_config_t;
```

**Register Mapping:**

```c
typedef struct {
    uint16_t address;               // Register address
    uint8_t type;                   // 0x01=coil, 0x03=holding, 0x04=input
    char name[64];                  // Register name
    char unit[16];                  // Unit (e.g., °C, %, bar)
    float scale;                    // Scaling factor
    float offset;                   // Offset value
    bool writable;                  // Register is writable
    char description[128];          // Description
} modbus_register_map_t;
```

**Functions:**

- `modbus_devices_init()` - Initialize device manager
- `modbus_add_device()` - Add new device configuration
- `modbus_remove_device()` - Remove device
- `modbus_get_device()` - Get device configuration
- `modbus_list_devices()` - List all devices
- `modbus_save_devices()` - Save to NVS
- `modbus_load_devices()` - Load from NVS
- `modbus_add_register_map()` - Add register mapping
- `modbus_get_register_value()` - Get cached register value
- `modbus_update_register_cache()` - Update register cache from device

#### 8. Web UI (`html/`)

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Configuration</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>ESP32 WiFi Manager</h1>
            <div class="status-indicator" id="status">
                <span class="status-dot"></span>
                <span class="status-text">Disconnected</span>
            </div>
        </header>

        <main>
            <section class="card">
                <h2>WiFi Configuration</h2>
                <form id="wifi-form">
                    <div class="form-group">
                        <label for="ssid">SSID</label>
                        <input type="text" id="ssid" name="ssid" required
                               placeholder="Enter network name" maxlength="32">
                    </div>
                    <div class="form-group">
                        <label for="password">Password</label>
                        <input type="password" id="password" name="password"
                               placeholder="Enter password" maxlength="64">
                    </div>
                    <button type="submit" class="btn btn-primary">Save & Connect</button>
                    <button type="button" class="btn btn-secondary" id="clear-btn">Clear Credentials</button>
                </form>
            </section>

            <section class="card">
                <h2>Network Information</h2>
                <div class="info-item">
                    <span class="label">Current Mode:</span>
                    <span class="value" id="mode">Unknown</span>
                </div>
                <div class="info-item">
                    <span class="label">IP Address:</span>
                    <span class="value" id="ip">--</span>
                </div>
                <div class="info-item">
                    <span class="label">Connected SSID:</span>
                    <span class="value" id="current-ssid">--</span>
                </div>
            </section>
        </main>

        <footer>
            <p>ESP32-C3 WiFi Manager</p>
        </footer>
    </div>
    <script src="/script.js"></script>
</body>
</html>
```

**CSS Styling (`style.css`):**

- Modern, responsive design
- Color scheme: Blue (#2563eb), White (#ffffff), Gray (#f3f4f6)
- Card-based layout with shadows
- Smooth transitions and animations
- Mobile-first approach
- Status indicators (green/red)
- Form styling with focus states

#### 9. Main Application (`main.c`)

**Responsibilities:**

- Application entry point
- Initialize all components
- Main event loop
- Error handling

**Flow:**

1. Initialize NVS storage
2. Check for WiFi credentials
3. Initialize WiFi Manager
4. Initialize Modbus Manager
5. Load Modbus device configurations
6. Start Web Server
7. Start Modbus polling task
8. Enter FreeRTOS main loop

**Tasks:**

- WiFi Manager Task - Handle WiFi events
- Web Server Task - Handle HTTP requests
- Modbus Polling Task - Periodically read registers from devices
- Modbus Write Task - Handle write requests asynchronously

## Implementation Phases

### Phase 1: Documentation & Planning (Current)

- [x] Analyze requirements
- [x] Check environment and tools
- [x] Design architecture
- [x] Create PLAN.md (this file)
- [ ] Create README.md

### Phase 2: Project Setup

- [ ] Create project structure
- [ ] Set up CMakeLists.txt files
- [ ] Create main application skeleton
- [ ] Create empty component files

### Phase 3: ESP-IDF Installation & Configuration

- [ ] Run `install.bat` for ESP32-C3 tools
- [ ] Source ESP-IDF environment
- [ ] Configure project (menuconfig if needed)
- [ ] Verify build system

### Phase 4: NVS Storage Implementation

- [ ] Implement `nvs_storage_init()`
- [ ] Implement `nvs_save_wifi_credentials()`
- [ ] Implement `nvs_load_wifi_credentials()`
- [ ] Implement `nvs_clear_wifi_credentials()`
- [ ] Add error handling
- [ ] Test NVS operations

### Phase 5: WiFi Manager Implementation

- [ ] Implement WiFi event handler
- [ ] Implement `wifi_manager_init()`
- [ ] Implement AP mode setup
- [ ] Implement Station mode setup
- [ ] Add connection retry logic
- [ ] Test WiFi operations

### Phase 6: Web Server Implementation

- [ ] Initialize HTTP server
- [ ] Implement `/` route (HTML)
- [ ] Implement `/style.css` route
- [ ] Implement `/save` POST handler
- [ ] Implement `/clear` POST handler
- [ ] Implement `/status` API endpoint
- [ ] Test web server

### Phase 7: Web UI Development

- [ ] Create HTML file
- [ ] Create CSS with styling
- [ ] Create JavaScript for form handling
- [ ] Add AJAX for status updates
- [ ] Test UI on different devices
- [ ] Refine styling and animations

### Phase 8: Integration & Testing

- [ ] Integrate all components
- [ ] Test AP mode workflow
- [ ] Test Station mode workflow
- [ ] Test credential persistence
- [ ] Test web interface functionality
- [ ] Verify error handling

### Phase 9: Build & Flash

- [ ] Build project successfully
- [ ] Flash to ESP32-C3
- [ ] Monitor serial output
- [ ] Test complete workflow

### Phase 10: Documentation Finalization

- [x] Update PLAN.md with actual implementation details
- [x] Update README.md with usage instructions
- [x] Add troubleshooting section
- [x] Create GitHub repository

### Phase 11: Modbus Manager Implementation

- [ ] Implement UART initialization and configuration
- [ ] Implement Modbus RTU protocol stack
- [ ] Add register read functions (0x01, 0x03, 0x04)
- [ ] Add register write functions (0x05, 0x06, 0x15, 0x16)
- [ ] Add CRC calculation and validation
- [ ] Add error handling and timeouts
- [ ] Implement retry logic
- [ ] Test with dummy Modbus slave or loopback

### Phase 12: Modbus Device Configuration

- [ ] Implement NVS storage for Modbus devices
- [ ] Add device configuration structure
- [ ] Implement register mapping system
- [ ] Add register caching mechanism
- [ ] Add UART configuration management
- [ ] Implement device health monitoring
- [ ] Add auto-discovery/scanning functionality

### Phase 13: Web Interface Extensions

- [ ] Create Modbus configuration page
- [ ] Create dashboard/display window for HVAC data
- [ ] Implement real-time data refresh (AJAX/WebSockets)
- [ ] Add register control panel (read/write)
- [ ] Add auto-scan functionality
- [ ] Add device status indicators
- [ ] Implement value formatting and unit display
- [ ] Add color-coded status (normal/warning/error)

### Phase 14: Modbus Integration & Testing

- [ ] Test UART communication with RS485 adapter
- [ ] Test with Enervent Pingvin device
- [ ] Test with Kotilämpö device
- [ ] Verify register mappings (add documentation later)
- [ ] Test read operations
- [ ] Test write operations
- [ ] Test error recovery and reconnection
- [ ] Test multiple devices simultaneously
- [ ] Performance testing (polling intervals)
- [ ] Stress testing (continuous operation)

### Phase 15: MQTT Preparation (Future)

- [ ] Design MQTT topic structure
- [ ] Implement JSON serialization for Modbus data
- [ ] Prepare MQTT client integration
- [ ] Add configuration for MQTT broker
- [ ] Implement publishing of register values
- [ ] Add subscription for control commands

## Build & Flash Instructions

### Initial Setup

1. **Install ESP-IDF Tools** (if not already installed)
   
   ```bash
   cd C:\Users\jaakk\esp\v5.1.6\esp-idf
   install.bat esp32c3
   ```
   
   This downloads compiler, flash tools, and other utilities (~2GB)

2. **Set Environment Variables**
   
   ```bash
   C:\Users\jaakk\esp\v5.1.6\esp-idf\export.bat
   ```
   
   Or add to your PATH permanently

3. **Create Project**
   
   ```bash
   cd "C:\Users\jaakk\DIY projects\Microcontroller codes\ESP32 modbus reader"
   idf.py create-project --path . wifi-manager
   ```

4. **Set Target**
   
   ```bash
   idf.py set-target esp32c3
   ```

### Building

```bash
idf.py build
```

### Flashing

1. Connect ESP32-C3 via USB

2. Identify COM port (e.g., COM3)

3. Flash and monitor:
   
   ```bash
   idf.py -p COM3 flash monitor
   ```

Or separately:

```bash
idf.py -p COM3 flash
idf.py -p COM3 monitor
```

### Configuration (Optional)

```bash
idf.py menuconfig
```

Navigate to:

- Component config → WiFi
- Component config → HTTP Server

## Usage Workflow

### First Time Setup

1. **Power on ESP32-C3**

    - Device starts in AP mode
    - Creates WiFi network: `ESP32-Config`
    - Password: `config123`

2. **Connect to AP**

    - Connect your phone/computer to `ESP32-Config`
    - Password: `config123`

3. **Access Configuration Page**

    - Open browser: `http://192.168.4.1`
    - See WiFi configuration page

4. **Enter WiFi Credentials**

    - Enter your home WiFi SSID
    - Enter password
    - Click "Save & Connect"

5. **Device Reboots**

    - ESP32-C3 saves credentials to NVS
    - Reboots and connects to your WiFi
    - Note the IP address in status

6. **Access Web Interface**

    - Connect your phone/computer to your home WiFi
    - Open browser: `http://<device-ip-address>`
    - View status, configure Modbus devices

### Modbus Configuration

1. **Connect Hardware**

    - Connect RS485 adapter to ESP32-C3 UART pins
    - Default: TX=GPIO4, RX=GPIO5 (configurable)
    - Connect RS485 to Enervent Pingvin and/or Kotilämpö devices

2. **Access Modbus Configuration**

    - Navigate to: `http://<device-ip-address>/modbus.html`
    - Click "Add Device"

3. **Configure Device**

    - Enter Device ID (e.g., 1 for Enervent Pingvin)
    - Enter Device Name (e.g., "Enervent Pingvin")
    - Enter Description
    - Set Poll Interval (default: 5000ms)
    - Configure UART settings if needed
    - Click "Save"

4. **Add Register Mappings**

    - Select device from list
    - Add register addresses (refer to device documentation)
    - Set register type (Holding, Input, Coil)
    - Enter name, unit, scaling factor
    - Mark writable if applicable
    - Click "Save"

5. **Test Communication**

    - Click "Test Connection" on device page
    - Verify device responds
    - Check register values in dashboard

6. **View Dashboard**

    - Navigate to: `http://<device-ip-address>/dashboard.html`
    - View real-time register values
    - Write to writable registers using control panel
    - Monitor device status

### Reconfiguration

If you need to change WiFi credentials:

1. Access web interface at device IP
2. Click "Clear Credentials"
3. Device reboots and creates AP again
4. Follow first-time setup steps

If you need to change Modbus configuration:

1. Access Modbus configuration page
2. Edit device settings or register mappings
3. Changes take effect immediately (no reboot needed)
4. Test communication after changes

## Troubleshooting

### Common Issues

#### ESP-IDF Tools Not Found

**Problem:** `idf.py` command not found

**Solution:**

```bash
C:\Users\jaakk\esp\v5.1.6\esp-idf\export.bat
```

#### Build Failures

**Problem:** Compilation errors

**Solutions:**

- Clean build: `idf.py fullclean && idf.py build`
- Check ESP-IDF version: `idf.py --version`
- Verify target: `idf.py show-target`

#### Flash Failures

**Problem:** Cannot flash to device

**Solutions:**

- Check COM port is correct
- Ensure USB drivers installed
- Try different USB cable
- Press BOOT button before flashing
- Check device is in bootloader mode

#### WiFi Not Connecting

**Problem:** Device fails to connect to WiFi

**Solutions:**

- Verify SSID and password are correct
- Check WiFi signal strength
- Ensure 2.4GHz band (ESP32 doesn't support 5GHz)
- Check router settings (WPA2/WPA3 compatibility)
- Review serial logs for error messages

#### Cannot Access Web Page

**Problem:** Browser cannot load configuration page

**Solutions:**

- Verify device is in AP mode or connected to WiFi
- Check IP address in serial logs
- Try different browser
- Clear browser cache
- Check firewall settings

### Debug Mode

Enable detailed logging:

```bash
idf.py menuconfig
```

Component config → Log output → Default log verbosity → Debug

## Future Enhancements

### Potential Improvements (Post-Modbus Implementation)

- [ ] **Captive Portal** - Redirect all HTTP requests to config page
- [ ] **WiFi Scanning** - Display available networks on web page
- [ ] **Multiple Network Profiles** - Save up to 5 networks, try each
- [ ] **MQTT Integration** - Publish Modbus data to MQTT broker, subscribe to control commands
- [ ] **OTA Updates** - Over-the-air firmware updates
- [ ] **Authentication** - Password protect web interface
- [ ] **Dark Mode** - Toggle dark/light theme
- [ ] **Mobile App** - Native mobile application
- [ ] **Status LED** - Visual connection indicator on device
- [ ] **WiFi Reconnect** - Auto-reconnect on connection loss
- [ ] **Network Time** - NTP time synchronization
- [ ] **WebSockets** - Real-time status updates for Modbus data
- [ ] **Data Logging** - Log historical data to flash storage
- [ ] **Alarms/Alerts** - Email/SMS alerts for threshold breaches
- [ ] **Trend Graphs** - Historical data visualization

### Advanced Features

- [ ] **Custom SSID/Password** - Configure AP credentials via web
- [ ] **Static IP** - Option for static IP assignment
- [ ] **DNS Server** - Custom DNS settings
- [ ] **Hostname** - Set device hostname
- [ ] **WiFi Power Save** - Low power mode options
- [ ] **Statistics** - Connection quality, signal strength
- [ ] **Logs** - View device logs on web page
- [ ] **Backup/Restore** - Export/import settings (JSON file)
- [ ] **Modbus TCP** - Add Modbus TCP support alongside RTU
- [ ] **Data Export** - CSV export of historical data
- [ ] **Custom Scripts** - User-defined automation scripts

## Technical References

### ESP-IDF Documentation

- [Official ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/en/v5.1.6/esp32c3/)
- [WiFi Configuration](https://docs.espressif.com/projects/esp-idf/en/v5.1.6/esp32c3/api-guides/wifi.html)
- [HTTP Server](https://docs.espressif.com/projects/esp-idf/en/v5.1.6/esp32c3/api-reference/protocols/esp_http_server.html)
- [NVS Flash Storage](https://docs.espressif.com/projects/esp-idf/en/v5.1.6/esp32c3/api-reference/storage/nvs_flash.html)

### ESP32-C3 Datasheet

- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [Seeed Studio ESP32-C3 Docs](https://wiki.seeedstudio.com/)

### Arduino vs ESP-IDF

- This project uses ESP-IDF for:
  - Full control over hardware
  - Better performance
  - Native WiFi stack
  - Complete feature access
- Benefits over Arduino:
  - Lower latency
  - More memory available
  - Better power management
  - OTA support

## Project Files

### Directory Structure

```
ESP32-modbus-reader/
├── CMakeLists.txt                 # Main CMake configuration
├── docs/
│   ├── PLAN.md                    # This file
│   ├── README.md                  # GitHub README
│   └── devices/                   # Device documentation (to be added)
│       ├── enervent_pingvin.md    # Enervent Pingvin docs
│       └── kotilampo.md           # Kotilämpö docs
├── main/
│   ├── CMakeLists.txt             # Main component CMake
│   ├── main.c                     # Application entry point
│   ├── wifi_manager.c             # WiFi implementation
│   ├── wifi_manager.h             # WiFi header
│   ├── web_server.c               # HTTP server implementation
│   ├── web_server.h               # HTTP server header
│   ├── nvs_storage.c              # NVS implementation
│   ├── nvs_storage.h              # NVS header
│   ├── modbus_manager.c           # Modbus RTU manager (to be added)
│   ├── modbus_manager.h           # Modbus manager header (to be added)
│   ├── modbus_protocol.c          # Modbus protocol stack (to be added)
│   ├── modbus_protocol.h          # Modbus protocol header (to be added)
│   ├── modbus_devices.c           # Device configuration manager (to be added)
│   ├── modbus_devices.h           # Device configuration header (to be added)
│   └── html/
│       ├── index.html             # Web UI HTML (WiFi config)
│       ├── style.css              # Web UI CSS
│       ├── script.js              # Web UI JavaScript
│       ├── modbus.html            # Modbus configuration (to be added)
│       ├── dashboard.html         # Data dashboard (to be added)
│       └── modbus.js              # Modbus UI JavaScript (to be added)
├── sdkconfig                      # Project configuration
├── sdkconfig.defaults             # Default configuration
└── .gitignore                     # Git ignore rules
```

### Key Files

| File                 | Purpose                                           |
| -------------------- | ------------------------------------------------- |
| `main.c`             | Application entry point, component initialization |
| `wifi_manager.c`     | WiFi driver, AP/Station mode management           |
| `web_server.c`       | HTTP server, request handling, static files       |
| `nvs_storage.c`      | Flash storage for WiFi credentials                |
| `modbus_manager.c`   | Modbus RTU master implementation                  |
| `modbus_protocol.c`  | Modbus RTU protocol stack                         |
| `modbus_devices.c`   | Device configuration and register mapping         |
| `index.html`         | Web interface HTML (WiFi config)                  |
| `modbus.html`        | Modbus device configuration page                   |
| `dashboard.html`     | Real-time data display dashboard                  |
| `style.css`          | Styled web interface CSS                          |
| `script.js`          | JavaScript for WiFi configuration                |
| `modbus.js`          | JavaScript for Modbus functionality               |

## Development Notes

### Memory Usage Estimates

- Free Heap: ~280KB (ESP32-C3)
- WiFi Stack: ~40KB
- HTTP Server: ~20KB
- Modbus Protocol: ~15KB
- Modbus Manager: ~10KB
- Device Configs: ~5KB (4 devices)
- Register Cache: ~10KB (dynamic)
- Application: ~10KB
- Free: ~170KB

### Power Consumption

- Active (WiFi ON, Modbus ON): ~100mA
- Active (WiFi ON, Modbus IDLE): ~85mA
- Light Sleep: ~15mA
- Deep Sleep: ~10µA

### Performance

- WiFi Connection time: 2-5 seconds
- AP mode startup: 1-2 seconds
- HTTP request: <100ms
- Page load: <500ms
- Modbus RTU read (single register): ~50ms
- Modbus RTU read (multiple registers): ~100-200ms
- Modbus RTU write: ~50ms
- Dashboard refresh: ~500-1000ms

## License

[To be determined by user]

## Contributing

1. Fork repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Create Pull Request

## Contact & Support

[To be determined by user]

---

**This document will be updated as development progresses.**
**Last updated: 2025-01-27**
