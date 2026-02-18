# AGENTS.md - Coding Guidelines for ESP32 Modbus Reader

This file provides build commands and code style guidelines for agentic coding agents working on this ESP32-C3 Modbus RTU Gateway project.

## Build Commands

### Universal Build (PowerShell)
```powershell
# For ESP32-C3 (default)
.\build.ps1 -Board esp32c3

# For WeAct ESP32-D0WD-V3
.\build.ps1 -Board weact

# With options
.\build.ps1 -Board esp32c3 -Clean -Flash -Monitor
```

**Parameters:**
- `-Board esp32c3|weact` - Select board (default: esp32c3)
- `-Clean` - Full clean before build
- `-Flash` - Flash after build
- `-Monitor` - Open serial monitor after flash
- `-Port COMx` - Specify serial port (auto-detect if omitted)

### Quick Access Build Scripts
```powershell
.\build-esp32c3.ps1    # ESP32-C3 incremental build
.\build-weact.ps1        # WeAct incremental build
```

### Manual Build Steps

#### For ESP32-C3
```bash
cd "C:\Users\jaakk\DIY projects\Microcontroller codes\ESP32 modbus reader"
copy sdkconfig.esp32c3.defaults sdkconfig
idf.py set-target esp32c3
idf.py build
```

#### For WeAct ESP32-D0WD-V3
```bash
cd "C:\Users\jaakk\DIY projects\Microcontroller codes\ESP32 modbus reader"
copy sdkconfig.weact_esp32.defaults sdkconfig
idf.py set-target esp32
idf.py build
```

### Clean Build
```powershell
# ESP32-C3
.\build.ps1 -Board esp32c3 -Clean

# WeAct
.\build.ps1 -Board weact -Clean
```

### Flash and Monitor
```powershell
# Auto-detect COM port
.\build.ps1 -Board esp32c3 -Flash -Monitor

# Specify COM port
.\build.ps1 -Board weact -Flash -Port COM3 -Monitor
```

### Monitor Only
```bash
idf.py -p COMX monitor
```

### Configuration
```bash
idf.py menuconfig
```

### Set Target
```bash
# ESP32-C3
idf.py set-target esp32c3

# WeAct ESP32-D0WD-V3
idf.py set-target esp32
```

## Board-Specific Guidelines

### Board Configuration System

This project supports multiple hardware boards through board-specific configuration headers:

**Supported Boards:**
- **ESP32-C3** (Seeed Studio XIAO, Espressif DevKitM-1) - Default target
- **WeAct ESP32-D0WD-V3 CAN485DevBoardV1** - Industrial dual-core board

**Board Configuration Files:**
- `boards/esp32c3_board.h` - ESP32-C3 definitions (RISC-V, single-core, 4MB flash)
- `boards/weact_esp32_board.h` - WeAct definitions (Xtensa, dual-core, 8MB flash)
- `boards/board.h` - Board selection wrapper (includes appropriate header based on BOARD define)

**Build-Time Board Selection:**
- Use CMake BOARD variable: `idf.py -D BOARD=weact build`
- Build scripts automatically set BOARD define via compile flags
- Default: `BOARD=esp32c3` if not specified

### Pin Configuration

#### ESP32-C3 Default Pins
```c
#define MODBUS_TX_PIN 21
#define MODBUS_RX_PIN 20
#define MODBUS_DE_PIN 7
#define MODBUS_RE_PIN 6
```

#### WeAct ESP32-D0WD-V3 Default Pins
```c
#define MODBUS_TX_PIN 22
#define MODBUS_RX_PIN 21
#define MODBUS_DE_PIN 17
#define MODBUS_RE_PIN 17 // Tied to DE on board
```

**Note:** All board-specific pins are defined in respective `boards/*_board.h` files and included via `boards/board.h`.

### Modbus RS485 Configuration

Both boards use UART_NUM_1 for RS485:
- Baudrate: 9600 bps (default, configurable)
- Data bits: 8
- Stop bits: 1
- Parity: None
- Timeout: 1000ms
- Retry attempts: 3

**WeAct Board Notes:**
- DE and RE pins are tied together (GPIO17)
- Set GPIO17 HIGH to transmit, LOW to receive
- RS485 transceiver is 2.5kV galvanically isolated from ESP32
- Recommended for industrial environments with electrical noise

### SDK Configuration Files

Use board-specific defaults files for clean configuration:

- **ESP32-C3:** `sdkconfig.esp32c3.defaults`
  - Target: ESP32-C3
  - Flash: 4MB
  - FreeRTOS: Single-core mode
  - Architecture: RISC-V

- **WeAct ESP32:** `sdkconfig.weact_esp32.defaults`
  - Target: ESP32
  - Flash: 8MB
  - FreeRTOS: Dual-core mode
  - Architecture: Xtensa

**Build scripts automatically copy appropriate defaults to `sdkconfig` before building.**

### Adding New Boards

To add support for a new hardware board:

1. **Create board header:**
   ```c
   // boards/newboard_board.h
   #ifndef NEWBOARD_BOARD_H
   #define NEWBOARD_BOARD_H

   #define BOARD_NAME "New Board Name"
   #define BOARD_TARGET "esp32"  // or "esp32c3", "esp32s3"
   #define BOARD_MCU "ESP32-..."
   #define BOARD_ARCH "Xtensa"  // or "RISC-V"
   #define BOARD_CORES 2

   #define BOARD_FLASH_SIZE_4MB  // or 8MB, 16MB

   // Pin definitions
   #define MODBUS_TX_PIN X
   #define MODBUS_RX_PIN X
   #define MODBUS_DE_PIN X
   #define MODBUS_RE_PIN X

   #define DEFAULT_BAUDRATE 9600

   #endif
   ```

2. **Add to `boards/board.h`:**
   ```c
   #ifdef BOARD_NEWBOARD
   #include "newboard_board.h"
   #elif defined(BOARD_ESP32C3)
   ...
   ```

3. **Create SDK defaults:** `sdkconfig.newboard.defaults`

4. **Update `main/CMakeLists.txt`:**
   ```cmake
   set_property(CACHE BOARD PROPERTY STRINGS "esp32c3" "weact_esp32" "newboard")

   if(BOARD STREQUAL "newboard")
       target_compile_definitions(${COMPONENT_LIB} PRIVATE BOARD_NEWBOARD)
   endif()
   ```

5. **Update build scripts** to recognize new board

6. **Add documentation** in `docs/devices/` with pinouts, features, integration guide

## Git Workflow Rules

### Commit and Push Guidelines
- **NEVER commit or push to GitHub unless explicitly asked by the user**
- When committing, **ALWAYS push to GitHub immediately after** - do not wait for separate push command
- Always read existing documentation before making changes
- Update relevant documentation files BEFORE any commit:
  - `docs/README.md` - Main documentation
  - `docs/IMPLEMENTATION_SUMMARY.md` - Implementation notes
  - `AGENTS.md` - Agent guidelines (if updating this file)
  - `README.md` - User-facing documentation
- When asked to commit:
  1. Run `git status` to review changes
  2. Run `git diff` to verify changes
  3. Stage only relevant files with `git add`
  4. Check `git status` to verify staged files
  5. Create meaningful commit message following project history style
  6. Commit with `git commit`
  7. **Automatically push with `git push origin main`** (or requested branch)
- Check for `*.backup` files and `.backup` patterns in `.gitignore`

### Example Commit Message Style
```
v1.x.y: Feature description

- Fixed specific bug in modbus_devices.c
- Added new function with error handling
- Updated documentation in docs/README.md
```

## Code Style Guidelines

### Imports and Includes
- Project headers first, then standard library, then ESP-IDF components
- Always include `esp_err.h` for error handling
- Always include `esp_log.h` for logging
- Use angle brackets for ESP-IDF: `<esp_log.h>`, `<freertos/FreeRTOS.h>`
- Use quotes for project headers: `"modbus_devices.h"`

```c
#include "modbus_devices.h"      // Project headers
#include "modbus_manager.h"
#include <stdint.h>               // Standard library
#include "esp_err.h"             // ESP-IDF core
#include "esp_log.h"             // Logging
#include "freertos/FreeRTOS.h"   // FreeRTOS
```

### Formatting and Indentation
- K&R style (opening brace on same line)
- 4 spaces per indentation (NO tabs)
- Space after comma in function calls: `func(a, b, c)`
- Space around operators: `a = b + c`, `if (x == y)`
- Max line length: 120 characters preferred
- One blank line between functions

```c
esp_err_t my_function(uint8_t device_id, const char *name) {
    if (device_id < 1 || device_id > 247) {
        ESP_LOGE(TAG, "Invalid device ID: %d", device_id);
        return ESP_ERR_INVALID_ARG;
    }
    
    strcpy(devices[device_id].name, name);
    return ESP_OK;
}
```

### Type Conventions
- Use standard types: `uint8_t`, `uint16_t`, `uint32_t`, `int8_t`
- Use `bool` for boolean values (from `<stdbool.h>`)
- Use `esp_err_t` for all function returns
- Define enums in CAPS_SNAKE_CASE: `DEVICE_STATUS_ONLINE`
- Define structs in snake_case: `modbus_device_t`, `wifi_status_t`
- Define constants in CAPS_SNAKE_CASE: `MAX_DEVICES`, `DEFAULT_TIMEOUT`

```c
typedef enum {
    DEVICE_STATUS_UNKNOWN = 0,
    DEVICE_STATUS_ONLINE = 1
} device_status_t;

typedef struct {
    uint8_t device_id;
    char name[32];
    bool enabled;
} modbus_device_t;

#define MAX_DEVICES 4
#define DEFAULT_TIMEOUT_MS 1000
```

### Naming Conventions
- Functions: CamelCase with module prefix: `modbus_add_device()`, `wifi_connect()`
- Variables: snake_case: `device_count`, `poll_interval_ms`
- Global static: snake_case with underscore prefix: `static uint8_t _last_error;`
- Constants: CAPS_SNAKE_CASE: `MAX_REGISTERS`, `DEFAULT_BAUDRATE`
- Enums: CAPS_SNAKE_CASE: `REGISTER_TYPE_HOLDING`, `MODBUS_RESULT_OK`
- Struct members: snake_case: `device_id`, `register_count`
- TAG: Static const in CAPS: `static const char *TAG = "MODULE";`

```c
static const char *TAG = "MY_MODULE";

esp_err_t my_function(uint8_t input_param, char *output_buffer) {
    uint8_t local_var = 0;
    const uint32_t CONSTANT_VAL = 100;
    // ...
}
```

### Error Handling
- Return `esp_err_t` from all functions
- Use ESP_ERROR_CHECK() for fatal errors in initialization
- Check return values with `if (err != ESP_OK)` for non-fatal errors
- Use standard ESP-IDF error codes: `ESP_OK`, `ESP_ERR_INVALID_ARG`, `ESP_ERR_NO_MEM`, `ESP_ERR_NOT_FOUND`, `ESP_FAIL`
- Always log errors with `ESP_LOGE(TAG, "Message: %s", esp_err_to_name(err))`
- Return early on errors, clean up resources before returning

```c
esp_err_t save_data(uint8_t device_id) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }
    
    err = nvs_set_u8(handle, "value", device_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save value: %s", esp_err_to_name(err));
        nvs_close(handle);  // Clean up before return
        return err;
    }
    
    nvs_close(handle);
    return ESP_OK;
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_storage_init());  // Fatal - abort on error
    ESP_ERROR_CHECK(modbus_init());
}
```

### Logging
- Define TAG at top of file: `static const char *TAG = "MODULE_NAME";`
- Use ESP_LOGI for informational messages
- Use ESP_LOGW for warnings (non-fatal issues)
- Use ESP_LOGE for errors (with context)
- Use ESP_LOGD for debug (use sparingly)
- Include relevant values in logs
- Don't log sensitive data (passwords, credentials)

```c
static const char *TAG = "MY_MODULE";

ESP_LOGI(TAG, "Starting initialization");
ESP_LOGI(TAG, "Device %d configured successfully", device_id);
ESP_LOGW(TAG, "Register count exceeds maximum, limiting to %d", MAX_REGS);
ESP_LOGE(TAG, "Failed to read device: %s", esp_err_to_name(err));
```

### Memory Management
- Free dynamically allocated memory: `free(buffer)`
- Check malloc return: `if (ptr == NULL) return ESP_ERR_NO_MEM;`
- Use ESP-IDF heap macros for debugging: `ESP_LOGI(TAG, "Free heap: %d", esp_get_free_heap_size());`
- Prefer stack allocation for small, short-lived buffers
- Use heap for large or long-lived data
- Set pointers to NULL after freeing

```c
char *buffer = malloc(256);
if (buffer == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory");
    return ESP_ERR_NO_MEM;
}

// Use buffer
free(buffer);
buffer = NULL;
```

### Constants and Macros
- Use `#define` for compile-time constants
- Use `enum` for related constants
- Don't use macros for functions (use inline functions instead)
- Macros in CAPS_SNAKE_CASE
- Parenthesize macro parameters

### Format Specifiers (Critical for ESP32)
- **NEVER use `PRIu32` or `PRIu16` with `int` type variables**
- **ALWAYS match format specifier to actual data type:**
  - `int` → `%d`
  - `uint32_t` → `PRIu32`
  - `int32_t` → `PRId32`
  - `uint16_t` → `PRIu16`
- **Common pitfall:** Board configuration pins stored as `int` in structs, logged with `PRIu32` → COMPILER ERROR
- **Build flag `-Werror=format`** treats format warnings as errors
- **Solution:** Use `%d` for `int` types, `PRIu32` for `uint32_t` types
- **ALWAYS include `<inttypes.h>`** when using `PRI` macros (but they're auto-included by ESP-IDF logging)

**Example:**
```c
typedef struct {
    int pin;           // int type
    uint32_t value;   // uint32_t type
} config_t;

ESP_LOGI(TAG, "Pin=%d", config.pin);           // CORRECT - uses %d for int
ESP_LOGI(TAG, "Value=%" PRIu32, config.value); // CORRECT - uses PRIu32 for uint32_t

// WRONG - will fail with -Werror=format
ESP_LOGI(TAG, "Pin=%" PRIu32, config.pin); // ERROR: PRIu32 expects uint32_t, got int
```

```c
#define MAX_DEVICES 4
#define DEFAULT_TIMEOUT_MS 1000
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef enum {
    MODE_AP = 0,
    MODE_STA = 1
} wifi_mode_t;
```

### Struct Initialization
- Use designated initializers: `.field = value`
- Zero-initialize with `memset()` for larger structs
- Initialize all fields to avoid undefined behavior

```c
modbus_device_t device = {
    .device_id = 1,
    .poll_interval_ms = 5000,
    .enabled = true,
    .register_count = 0
};

// Or for larger structs:
modbus_device_t device;
memset(&device, 0, sizeof(device));
device.device_id = 1;
```

### String Handling
- Use `strncpy()` with buffer size - 1 for safety
- Always null-terminate strings
- Check string lengths before operations
- Use `snprintf()` for formatted strings with buffer size

```c
strncpy(device.name, input_name, sizeof(device.name) - 1);
device.name[sizeof(device.name) - 1] = '\0';

snprintf(buffer, sizeof(buffer), "Device %d: %s", id, name);
```

### FreeRTOS Tasks
- Use `xTaskCreate()` to create tasks
- Specify stack size (in words, typically 4096 for simple tasks)
- Use `pdMS_TO_TICKS()` for time delays
- Never block in tasks (use vTaskDelay instead of delays)
- Delete tasks when no longer needed

```c
void polling_task(void *pvParameters) {
    while (1) {
        // Do work
        vTaskDelay(pdMS_TO_TICKS(1000));  // Delay 1 second
    }
}

xTaskCreate(polling_task, "polling", 4096, NULL, 5, NULL);
```

### Project-Specific Guidelines

#### Partition Table Issues

**ERROR: app partition is too small for binary**
```
Error: app partition is too small for binary esp32-modbus-reader.bin size 0x107a00
```

**Root Cause:**
- `idf.py set-target` regenerates sdkconfig with default partition (1MB)
- Binary size (0x107a00 = 1,078,016 bytes ≈ 1.04MB) exceeds 1MB partition
- Custom partition configuration may be overridden by target defaults

**Solution:**
1. Use custom partitions.csv with 2MB factory partition (see above)
2. Build.ps1 automatically re-applies board config after `idf.py set-target`
   - `idf.py set-target` overwrites sdkconfig with default target settings
   - Build.ps1 re-copies board-specific config after set-target
3. For WeAct board (8MB flash), force 8MB flash size config
4. Manual fix: Edit sdkconfig to set `CONFIG_PARTITION_TABLE_CUSTOM=y`

**Verification:**
```bash
grep "PARTITION" sdkconfig
# Should show: CONFIG_PARTITION_TABLE_CUSTOM=y
# Should NOT show: CONFIG_PARTITION_TABLE_SINGLE_APP=y

grep "FLASHSIZE" sdkconfig
# For WeAct: CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y
# For ESP32-C3: CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
```

**IMPORTANT: `idf.py set-target` MUST be followed by board config re-apply**
- The set-target command overwrites sdkconfig with default target settings
- Build.ps1 automatically re-applies board-specific config after set-target
- If implementing custom build scripts, always copy board defaults AFTER set-target

### NVS Storage

### Format Specifier Issues (ESP32 Specific)

**ERROR Pattern:**
```
HINT: The issue is better to resolve by replacing format specifiers to 'PRI'-family macros (include <inttypes.h> header file).
```

**Common Causes:**

1. **Using PRIu32 with int type:**
   ```c
   int pin = 21;
   ESP_LOGI(TAG, "Pin=%" PRIu32, pin); // ERROR: expects uint32_t, got int
   ```

2. **Using %d with uint32_t type:**
   ```c
   uint32_t value = 100;
   ESP_LOGI(TAG, "Value=%d", value); // ERROR on some platforms
   ```

**Solution: Match specifier to type**
```c
// For int types
int pin;
ESP_LOGI(TAG, "Pin=%d", pin);

// For uint32_t types
uint32_t value;
ESP_LOGI(TAG, "Value=%" PRIu32, value);
```

**ESP-IDF Logging:**
- Always includes `<inttypes.h>` automatically
- Use `PRI` macros only for fixed-width integer types (uint32_t, int32_t, etc.)
- Use `%d` for regular `int` types

**Note:** `idf.py set-target` may change architecture (RISC-V ↔ Xtensa), requiring format specifier fixes.
- Namespace: `"modbus_config"` for devices, `"wifi_config"` for WiFi
- **Critical: NVS key names MUST be ≤15 characters** (ESP32 limitation)
- Use abbreviated key names: `d%d_id` (device 0 ID), `d%dr%da` (device 0 register 0 address)
- Key format: `d{device}_{field}` or `d{device}r{register}{field}` where field is abbreviated (id, name, desc, poll, en, baud, rc, a, t, n, u, s, o, w, d)
- Always check `nvs_get_*()` return values
- Call `nvs_commit()` before `nvs_close()` for writes
- Provide safe defaults when reads fail

#### Modbus Protocol
- Function codes: Use enum from `modbus_protocol.h`
- UART: UART_NUM_1 for RS485
- Timeout: 1000ms default, 3 retry attempts
- CRC: Calculate on send, validate on receive

#### Web Server
- Use `httpd_req_get_url_query_str()` for query parameters
- Return JSON with cJSON: `cJSON_PrintUnformatted()`
- Free cJSON with `cJSON_Delete()`
- Free JSON strings with `free()`

#### JSON Handling
- Always check `cJSON_Parse()` result for NULL
- Always check `cJSON_GetObjectItem()` result before use
- Check types: `cJSON_IsNumber()`, `cJSON_IsString()`, `cJSON_IsBool()`
- Free root object: `cJSON_Delete(root)`

```c
cJSON *root = cJSON_Parse(buf);
if (root == NULL) {
    return ESP_FAIL;
}

cJSON *id = cJSON_GetObjectItem(root, "device_id");
if (!id || !cJSON_IsNumber(id)) {
    cJSON_Delete(root);
    return ESP_ERR_INVALID_ARG;
}

uint8_t device_id = id->valueint;
cJSON_Delete(root);
```

## File Organization
- Headers in `main/*.h`
- Implementation in `main/*.c`
- Web files in `main/html/`
- One module per file pair (e.g., `modbus_devices.c/h`)
- Keep functions cohesive and small (<100 lines preferred)

## ESP-IDF Specific Guidelines

### Component Dependencies (CMakeLists.txt)
- ESP-IDF v5.x requires explicit `REQUIRES` for all component dependencies
- Common components to include: `driver`, `nvs_flash`, `esp_netif`, `esp_event`, `esp_wifi`, `esp_http_server`, `json`, `mqtt`
- Example:
```cmake
idf_component_register(SRCS "main.c" "wifi_manager.c" ...
                       REQUIRES driver nvs_flash esp_netif esp_event esp_wifi esp_http_server json mqtt)
```

### Header File Naming
- **Avoid naming project headers the same as ESP-IDF component headers**
- Example: Don't create `mqtt_client.h` - conflicts with ESP-IDF's `<mqtt_client.h>`
- Use unique names like `mqtt_gateway.h` or `my_mqtt.h`

### ESP-IDF v5.x MQTT Include
- Use `#include <mqtt_client.h>` (angle brackets, mqtt_client.h)
- NOT `#include "esp_mqtt_client.h"` (older ESP-IDF versions)

### Modbus Transaction Mutex
- **Critical: All Modbus operations must acquire the mutex before accessing the UART**
- The `modbus_mutex` (SemaphoreHandle_t) prevents bus contention on RS485
- Background polling task and external writes (e.g., from MQTT) must not collide
- Mutex is automatically acquired/released by all modbus_manager functions
- **When implementing external Modbus operations:**
  - Call modbus_manager functions (they handle the mutex internally)
  - Do NOT directly access UART driver - always use modbus_manager API
  - Example: MQTT writes use `modbus_write_single_coil()` which acquires mutex
- **Mutex API in modbus_manager.c:**
  - `xSemaphoreTake(modbus_mutex, portMAX_DELAY)` - Acquire before UART access
  - `xSemaphoreGive(modbus_mutex)` - Release after UART operation completes
  - Mutex created in `modbus_manager_init()`, destroyed in `modbus_manager_deinit()`

### MQTT Write Callback Pattern
- Use callback pattern for external write requests (e.g., from MQTT, web API)
- Register callback with `mqtt_client_set_register_write_callback()`
- Callback signature: `void callback(uint8_t device_id, uint16_t address, uint16_t value)`
- In callback:
  1. Validate device exists: `modbus_get_device(device_id)`
  2. Validate register exists: `modbus_get_register(device_id, address)`
  3. Perform write with appropriate function (handles mutex internally):
     - Coil: `modbus_write_single_coil(device_id, address, bool_value)`
     - Holding register: `modbus_write_single_register(device_id, address, value)`
  4. Update local cache: `modbus_update_register_value(device_id, address, value)`
  5. Publish new value (if applicable): `mqtt_client_publish_register(...)`
- **Case-insensitive string comparison for ON/OFF:**
  - Use `strcasecmp(payload, "ON")` and `strcasecmp(payload, "OFF")`
  - Convert non-zero values to ON, zero to OFF

### Partition Tables

**CRITICAL ERROR PATTERN:**
```
Error: app partition is too small for binary esp32-modbus-reader.bin size 0x107a00
```
**Solution:** See "Partition Table Configuration" section below.

#### Default vs Custom Partition Tables
- **Default factory partition:** 1MB - too small for this project
- **Custom factory partition:** 2MB - required for this project's firmware size
- **Error trigger:** Binary size exceeds 1MB (0x100000 bytes)

#### Partition Table Configuration
Create `partitions.csv` for custom 2MB partition layout:
```csv
# Name,   Type, SubType, Offset,   Size, Flags
nvs,      data, nvs,     0x9000,   0x6000,
phy_init, data, phy,     0xf000,   0x1000,
factory,  app,  factory, 0x10000,  0x200000,
```

**Configure in sdkconfig:**
```
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_SINGLE_APP=n
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
```

**Build Script Behavior:**
- Build.ps1 automatically applies custom partition config after `idf.py set-target`
- For WeAct board, script edits sdkconfig to force CUSTOM partition
- If error persists, manually set: `idf.py menuconfig` → Partition Table → Custom

**Troubleshooting:**
1. Check `sdkconfig` for: `CONFIG_PARTITION_TABLE_SINGLE_APP=n`
2. Check `sdkconfig` for: `CONFIG_PARTITION_TABLE_CUSTOM=y`
3. Verify `partitions.csv` exists in project root
4. Run `idf.py fullclean` if config won't apply
- Add to `sdkconfig.defaults`:
```
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
```

### Flash Size Configuration
- ESP32-C3 typically has 4MB flash - configure in `sdkconfig.defaults`:
```
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="4MB"
```

### Buffer Size Warnings
- ESP-IDF treats format truncation warnings as errors
- Ensure `snprintf()` buffers are large enough for the content
- Include `<inttypes.h>` and use `PRIu32`, `PRId32` etc. for format specifiers

### HTTP Server URI Handler Limit
- Default max URI handlers is 8 - easily exceeded when adding API endpoints
- Set in code when creating httpd config:
```c
httpd_config_t config = HTTPD_DEFAULT_CONFIG();
config.max_uri_handlers = 24;  // Increase from default 8
```
- Error message: `httpd_register_uri_handler: no slots left for registering handler`

