# Enervent HVAC Device Documentation

This directory contains detailed Modbus register documentation for Enervent HVAC ventilation systems and development boards.

## Development Boards

### [WeAct CAN485DevBoardV1_ESP32](WeAct_CAN485DevBoardV1.md)
Industrial development board with dual-core ESP32-D0WD-V3, 8MB flash, 2.5kV isolated RS485 and CAN interfaces.

- **Architecture**: Xtensa dual-core (more powerful than ESP32-C3)
- **Flash**: 8MB (double ESP32-C3 capacity)
- **RS485**: 2.5kV galvanically isolated, TX=GPIO22, RX=GPIO21, DE=GPIO17
- **CAN**: 2.5kV isolated transceiver (not currently used)
- **TF Card**: SPI interface for SD card (not currently used)
- **Documentation**: [WeAct Board Guide](WeAct_CAN485DevBoardV1.md)
- **Build**: `.\build-weact.ps1`

**Comparison:**
| Feature | WeAct ESP32-D0WD | ESP32-C3 (XIAO) |
|---------|-------------------|-------------------|
| Cores | 2 (Xtensa) | 1 (RISC-V) |
| Flash | 8MB | 4MB |
| Isolation | 2.5kV on RS485 | Depends on module |
| CAN Bus | Yes | No |
| TF Card | Yes | No |
| Form Factor | Industrial (55mm x 40mm) | Compact (XIAO) |

## Available Devices

### [eWind](ewind.md)
Enervent eWind ventilation system with comprehensive Modbus RTU interface.

- **Features**: Coils, Holding registers, extensive alarm system, boost modes, analog inputs
- **Default baudrate**: 9600 bps (configurable: 9600, 19200, 38400)
- **Documentation**: [eWind Modbus Register List](ewind.md)

### [eAirMD](eairmd.md)
Enervent eAirMD ventilation system with advanced time-based automation and extensive analog input support.

- **Features**: Holding registers only, 20 week timer slots, 5 year timer slots, 6 analog inputs, detailed alarm logging
- **Default baudrate**: 19200 bps (configurable: 9600, 19200, 115200)
- **Software version**: 1.14 and later
- **Documentation**: [eAirMD Modbus Register List](eairmd.md)

## Device Comparison

| Feature | eWind | eAirMD |
|---------|--------|---------|
| **Register Types** | Coils, Holding Registers | Holding Registers only |
| **Default Speed** | 9600 bps | 19200 bps |
| **Supported Speeds** | 9600, 19200, 38400 | 9600, 19200, 115200 |
| **Analog Inputs** | AI1 (CO2/RH sensors) | AI1-AI6 (up to 6 inputs) |
| **Time Programs** | Basic boost timers | 20 week slots, 5 year slots |
| **Alarm Log** | 20 alarms | 20 alarms |
| **Circulation Fan** | Yes (specific models) | Not documented |
| **Filter Test** | Not documented | Yes (configurable) |
| **Network Status** | Not documented | Ethernet status register |
| **RTC** | Not documented | Yes (read/write) |

## Common Features

### Modbus RTU Protocol
Both devices use Modbus RTU over RS485:
- **Function codes**: 0x01 (Read Coils), 0x03 (Read Holding Registers), 0x06 (Write Single Register)
- **Data bits**: 8
- **Stop bits**: 1
- **Parity**: Configurable (None or Even)
- **Device addressing**: 1-100 (unique per device on bus)

### Temperature Scaling
All temperature registers use a **multiplier of 10**:
- Raw value ÷ 10 = Temperature in °C
- Example: Raw 215 → 21.5°C

### Alarm System
Both devices maintain an alarm log with up to 20 alarms:
- **Alarm 1** = Newest alarm
- **Alarm 20** = Oldest alarm
- Each alarm entry includes: type, state, and timestamp
- Alarms can be acknowledged via write operations

### Analog Input Configuration
Both support analog inputs with configurable sensor types:
- **Raw voltage**: 0-10V measurements
- **Calculated values**: Scaled and offset results based on voltage limits
- **Sensor types**: CO2 sensors, RH sensors, temperature sensors, pressure sensors

## Register Map Template

When adding new device documentation, use this format:

```markdown
## Device Specifications

- **Model**: [Device Name]
- **Protocol**: Modbus RTU
- **Connection**: RS485
- **Baudrate**: [Default] (configurable options)
- **Device ID**: [Default] (configurable range)
- **Register types**: [Coils, Holding, Input, Discrete Input]

## Coil Registers (if applicable)

| Address | Symbol | Description | Type | R/W | Multiplier | Range | Unit | Description | Notes |
|---------|--------|-------------|------|------|------------|-------|------|-------------|--------|

## Holding Registers

| Address | Symbol | Description | Type | R/W | Multiplier | Range | Unit | Description | Notes |
|---------|--------|-------------|------|------|------------|-------|------|-------------|--------|

## Enumerators

### [enum_name]

| Value | Label |
|-------|-------|
| 0 | Label0 |
| 1 | Label1 |

## Usage Notes

- Important notes about the device
- Special behaviors or requirements
- Register grouping recommendations
```

## Quick Reference

### Common Temperature Registers

| Register | eWind | eAirMD | Description | Multiplier | Unit |
|----------|--------|---------|-------------|------------|------|
| Outside Air | 6 (HREG_T_FRS) | 6 (HREG_T_FRS) | Fresh air temperature | 10 | °C |
| Supply Air (Post-HRC) | 7 (HREG_T_SPLY_LTO) | 7 (HREG_T_SPLY_LTO) | Supply air after heat recovery | 10 | °C |
| Supply Air | 8 (HREG_T_SPLY) | 8 (HREG_T_SPLY) | Supply air temperature | 10 | °C |
| Extract Air | 10 (HREG_T_EXT) | 10 (HREG_T_EXT) | Extract air from building | 10 | °C |
| Exhaust Air | 9 (HREG_T_WST) | 9 (HREG_T_WST) | Exhaust air temperature | 10 | °C |

### Common Fan Speed Registers

| Register | eWind | eAirMD | Description | Type | Range | Unit |
|----------|--------|---------|-------------|------|-------|------|
| Supply Fan Speed | 3 (HREG_EFFECTIVE_TF) | 3 (HREG_EFFECTIVE_TF) | Current supply fan speed | uint16 | % | - |
| Extract Fan Speed | 4 (HREG_EFFECTIVE_PF) | 4 (HREG_EFFECTIVE_PF) | Current exhaust fan speed | uint16 | % | - |
| Supply Fan Level 2 | 51 (HREG_FAN_TF_BASIC) | - | TF fan speed level 2 | uint16 | 20-100 | % |
| Extract Fan Level 2 | 52 (HREG_FAN_PF_BASIC) | - | PF fan speed level 2 | uint16 | 20-100 | % |

### Common Configuration Registers

| Register | eWind | eAirMD | Description | Type | Range | Unit |
|----------|--------|---------|-------------|------|-------|------|
| Supply Air Setpoint | 135 (HREG_T_SETPOINT) | 135 (HREG_T_SETPOINT) | User-defined setpoint | int16 | 0-500 | °C |
| Modbus Speed | 733 (HREG_MODBUS_SPEED) | - | Modbus serial line speed | enumeration | - | bps |
| Modbus Parity | 734 (HREG_MODBUS_PARITY) | - | Modbus serial line parity | enumeration | - | - |
| Modbus Address | - | 640 (HREG_MBADDR) | Device ID | uint16 | 1-100 | - |
| System Uptime | 343 (HREG_UPTIME) | 343 (HREG_UPTIME) | Minutes since startup | uint16 | - | min |

### Common Alarm Registers

| Register Range | eWind | eAirMD | Description |
|-------------|--------|---------|-------------|
| 385-391 | Yes | Yes | Alarm 1 (newest) |
| 392-398 | Yes | Yes | Alarm 2 |
| 518-524 | Yes | Yes | Alarm 20 (oldest) |
| 581 | Yes (HREG_N_O_ALARMS) | Yes (HREG_N_O_ALARMS) | Number of alarms in log |
| 538 | Yes (HREG_ALARM_SERVICE_TIME) | Yes (HREG_ALARM_SERVICE_TIME) | Days until service reminder |

## ESP32 Integration Guide

### Basic Setup

1. **Configure Modbus Manager** in your ESP32 project
2. **Set communication parameters** matching device defaults
3. **Add device** with appropriate Device ID
4. **Register device-specific mappings** using registers documented above

### Recommended Registers to Monitor

**For basic HVAC monitoring:**
- Temperature sensors: 6, 7, 8, 9, 10
- Fan speeds: 3, 4 (effective speeds)
- Humidity: 13 (HREG_HUM_EXT)
- Status: 44 (HREG_MODE), 45 (HREG_EXTMODE)
- Alarms: 385-524 (alarm log)

**For advanced control:**
- Setpoint control: 135 (HREG_T_SETPOINT)
- Fan configuration: 51-52, 54-55 (fan speeds)
- Boost modes: Coil 8-11 (enable boosts), 66-76 (boost parameters)
- Time programs: eAirMD only, registers 210-384

### Reading Multiple Registers

For efficiency, read consecutive registers where possible:
- **Temperatures**: Read 6-12 (7 registers)
- **Fan speeds**: Read 3-4 (2 registers)
- **Alarms**: Read in batches or single as needed

### Writing Registers

- **Setpoints**: Temperature values must be multiplied by 10
  - Example: To set 21.5°C, write 215 to HREG_T_SETPOINT
- **Fan speeds**: Use percentage (20-100)
- **Coils**: Write 0 or 1 to enable/disable functions

### Error Handling

- **Timeout**: Default 1000ms per request
- **Retry attempts**: Recommended 3 attempts
- **Check HREG_MODE register**: Verify device is operational
- **Monitor alarm count**: HREG_N_O_ALARMS indicates active issues

## Additional Resources

- **ESP32 Modbus Reader Implementation**: [../PLAN.md](../PLAN.md)
- **Project README**: [../README.md](../README.md)
- **Modbus Protocol Reference**: See project documentation for ESP32 implementation details

## Contributing

To add documentation for a new Enervent device:

1. Create a new `.md` file in this directory
2. Follow the register map template above
3. Include device specifications, register tables, enumerators, and usage notes
4. Update this `README.md` to link to the new documentation
5. Test register mappings with actual hardware when possible

## Notes

- **Device IDs**: Must be unique on the RS485 bus (recommended range: 1-100)
- **Baudrate matching**: Ensure ESP32 UART configuration matches device settings
- **Register addressing**: Some devices use 0-based addressing, verify with documentation
- **Temperature multipliers**: Most Enervent devices use ×10, but always verify per-register

---

**Documentation maintained by:** ESP32 Modbus Reader Project
**Last updated:** 2025-01-27
