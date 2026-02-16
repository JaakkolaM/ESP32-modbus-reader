# AGENTS.md - Coding Guidelines for ESP32 Modbus Reader

This file provides build commands and code style guidelines for agentic coding agents working on this ESP32-C3 Modbus RTU Gateway project.

## Build Commands

### Full Build
```bash
cd "C:\Users\jaakk\DIY projects\Microcontroller codes\ESP32 modbus reader"
idf.py build
```

### Clean Build
```bash
idf.py fullclean
idf.py build
```

### Flash and Monitor
```bash
idf.py -p COMX flash monitor
```

### Monitor Only
```bash
idf.py -p COMX monitor
```

### Set Target
```bash
idf.py set-target esp32c3
```

### Configuration
```bash
idf.py menuconfig
```

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

#### NVS Storage
- Namespace: `"modbus_config"` for devices, `"wifi_config"` for WiFi
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
