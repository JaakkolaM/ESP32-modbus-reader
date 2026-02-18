# WeAct Studio CAN485DevBoardV1_ESP32

The WeAct CAN485DevBoardV1_ESP32 is an industrial development board featuring 2.5kV isolated CAN + RS485 communication, designed for ESP32-D0WD-V3 microcontroller with 8MB flash and CH343P USB interface.

![WeAct CAN485 Board](https://raw.githubusercontent.com/WeActStudio/WeActStudio.CAN485DevBoardV1_ESP32/master/Images/1.png)

## Board Specifications

| Feature | Specification |
|---------|---------------|
| **MCU** | ESP32-D0WD-V3 (dual-core Xtensa) |
| **Flash** | 8MB |
| **RAM** | 520KB SRAM |
| **USB Interface** | CH343P USB-UART bridge |
| **RS485 Isolation** | 2.5kV galvanic isolation |
| **CAN Interface** | 2.5kV galvanic isolation |
| **Operating Voltage** | 5V USB, 7-36V external |
| **Operating Temperature** | -40°C to +85°C |

## Pin Configuration

### RS485 Interface (Modbus RTU)

| Signal | GPIO | Description |
|--------|-------|-------------|
| DI (TX) | GPIO22 | RS485 data input (from ESP32) |
| RO (RX) | GPIO21 | RS485 data output (to ESP32) |
| DE | GPIO17 | Driver enable (transmit mode) |
| RE | GPIO17 | Receiver enable (tied to DE) |
| B+ | - | RS485 bus positive |
| B- | - | RS485 bus negative |

**Default Modbus Configuration for ESP32 Modbus Reader:**
- TX Pin: GPIO22
- RX Pin: GPIO21
- DE/RE Pin: GPIO17
- Baudrate: 9600 bps (configurable)
- UART: UART_NUM_1 (shared with RS485)

**Important Notes:**
- DE and RE are tied together on the board
- Set DE high to transmit, low to receive
- RS485 transceiver is isolated (2.5kV) from ESP32
- Supports half-duplex communication (Modbus RTU)

### CAN Interface

| Signal | GPIO | Description |
|--------|-------|-------------|
| TX | GPIO27 | CAN transceiver TX |
| RX | GPIO26 | CAN transceiver RX |
| H | - | CAN high |
| L | - | CAN low |

**Notes:**
- Not currently used by ESP32 Modbus Reader
- Available for future CAN bus implementation
- Uses ESP32 TWAI (Two-Wire Automotive Interface) peripheral

### TF Card (SD Card) Interface

| Signal | GPIO | Description |
|--------|-------|-------------|
| CS | GPIO13 | Chip select |
| SCK | GPIO14 | SPI clock |
| MOSI | GPIO15 | Master out slave in |
| MISO | GPIO2 | Master in slave out |

**Notes:**
- Supports SD/SDHC cards
- SPI interface (VSPI)
- Not currently used by ESP32 Modbus Reader
- Available for future data logging features

### User Interface

| Feature | GPIO | Description |
|---------|-------|-------------|
| WS2812 LED | GPIO4 | RGB LED for status indication |
| KEY | GPIO0 | Button (GPIO0 for boot) |
| VIN Monitor | GPIO36 | Analog input voltage monitoring |

**Important Boot Behavior:**
- **Pressing GPIO0 while powering on** keeps device in Boot mode
- Must power cycle again to run normally
- **Remove TF card before flashing** - otherwise fails

## Hardware Connections

### RS485 Modbus Connection

```
ESP32 (WeAct Board)            HVAC Modbus Device
+------------------+          +-------------------+
| GPIO22 (DI)     |  A+ ---> | A+                 |
| GPIO21 (RO)     |  B- ---> | B-                 |
| GPIO17 (DE/RE)  |          |                    |
| GND              |  ---->   | GND (optional)      |
+------------------+          +-------------------+
```

**Wiring Notes:**
- Use twisted pair for A+ and B-
- Add 120Ω termination resistor at both ends of bus
- Ground connection optional but recommended for noise immunity
- RS485 transceiver is isolated from ESP32

### Power Supply

- **USB Power:** 5V via USB-C (programming + operation)
- **External Power:** 7-36V DC via VIN pin
- **Voltage Monitoring:** GPIO36 reads VIN voltage (see code notes)

**Voltage Reading Calculation:**
```c
// WeAct board voltage divider ratio
int analogVolts = analogReadMilliVolts(36);
int vin_Volts = analogVolts * (51 + 510 + 51) / 51;
// vin_Volts = actual input voltage in millivolts
```

## ESP32 Modbus Reader Integration

### Build Configuration

To build ESP32 Modbus Reader for WeAct board:

```powershell
.\build-weact.ps1
```

Or using universal build script:
```powershell
.\build.ps1 -Board weact
```

### Pin Mapping

The WeAct board requires different RS485 pins than ESP32-C3:

| Component | ESP32-C3 | WeAct ESP32-D0WD |
|-----------|------------|-------------------|
| TX (UART) | GPIO21 | GPIO22 |
| RX (UART) | GPIO20 | GPIO21 |
| DE/RE | GPIO7 / GPIO6 | GPIO17 |

### Additional Features

The WeAct board provides these **unused but available** features:

1. **CAN Bus** (GPIO26/27)
   - TWAI peripheral available
   - 2.5kV isolated transceiver
   - Future: Add CAN device support

2. **TF Card** (GPIO13/14/15/2)
   - VSPI interface
   - Future: Historical data logging to SD card

3. **Status LED** (GPIO4)
   - WS2812 RGB LED
   - Future: Visual status indication (WiFi, Modbus, etc.)

4. **Button** (GPIO0)
   - Boot button
   - Future: User interaction (reset, mode switch)

## Comparison with ESP32-C3

| Feature | ESP32-C3 (Seeed XIAO) | WeAct ESP32-D0WD-V3 |
|---------|-------------------------|----------------------|
| **Architecture** | RISC-V (single-core) | Xtensa (dual-core) |
| **Flash Size** | 4MB | 8MB |
| **RS485 Pins** | TX=21, RX=20, DE=7, RE=6 | TX=22, RX=21, DE=17 |
| **Isolation** | No (depending on module) | Yes (2.5kV galvanic) |
| **USB Interface** | Native USB | CH343P USB-UART |
| **CAN Bus** | No | Yes (isolated) |
| **TF Card** | No | Yes |
| **Form Factor** | Small (XIAO) | Standard (40mm x 55mm) |
| **Industrial Grade** | Hobbyist/prototyping | Industrial (temp range, isolation) |
| **Price** | Lower | Higher |

## Advantages

### Why Use WeAct Board?

1. **Industrial Reliability**
   - 2.5kV isolation on RS485 and CAN
   - Wide operating temperature (-40°C to +85°C)
   - Galvanic isolation protects ESP32 from transients

2. **More Resources**
   - Dual-core CPU (better multitasking)
   - 8MB flash (more room for features)
   - More GPIO pins

3. **Additional Interfaces**
   - CAN bus for automotive/industrial protocols
   - TF card for data logging
   - RGB LED for status indication

4. **Standard Form Factor**
   - Easier to mount in enclosures
   - Standard connector spacing
   - Better RF shielding

## Disadvantages

### When to Use ESP32-C3 Instead?

1. **Lower Cost**
   - ESP32-C3 boards generally cheaper
   - Sufficient for basic Modbus gateway

2. **Smaller Form Factor**
   - XIAO ESP32C3 is very compact
   - Better for tight spaces

3. **Native USB**
   - No external USB-UART bridge
   - Direct USB-C connection
   - Easier debugging

## Resources

- **Repository:** [WeActStudio/WeActStudio.CAN485DevBoardV1_ESP32](https://github.com/WeActStudio/WeActStudio.CAN485DevBoardV1_ESP32)
- **Examples:** See `Examples/` folder in WeAct repository
- **Hardware:** [Hardware Development Kit](https://github.com/WeActStudio/WeActStudio.CAN485DevBoardV1_ESP32/tree/master/Hardware)
- **Documentation:** [Doc folder](https://github.com/WeActStudio/WeActStudio.CAN485DevBoardV1_ESP32/tree/master/Doc)

## Troubleshooting

### Flashing Fails

**Problem:** Cannot flash firmware
**Solution:**
- Remove TF card before flashing (WeAct board specific)
- Check USB-C cable supports data transfer
- Try pressing BOOT button (GPIO0) while connecting USB

### Modbus Communication Issues

**Problem:** Cannot communicate with HVAC devices
**Solutions:**
- Verify A+/B- wiring polarity
- Check termination resistor (120Ω at both ends)
- Verify DE/RE pin configured as GPIO17
- Check baudrate matches HVAC device
- Use `idf.py monitor` to see Modbus logs

### Device Won't Boot

**Problem:** Stuck in Boot mode
**Solution:**
- GPIO0 button may be pressed
- Disconnect any connections to GPIO0
- Power cycle without pressing GPIO0

## ESP32 Modbus Reader Compatibility

**Current Version:** 1.4.2
**Status:** ✅ Fully Compatible
**Pin Mapping:** Automatically configured via board selection
**Flash Space:** 8MB provides ample room for future features

---

**Documentation:** WeAct CAN485DevBoardV1_ESP32
**Last Updated:** 2026-02-18
