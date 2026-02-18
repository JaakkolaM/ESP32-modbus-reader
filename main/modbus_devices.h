#ifndef MODBUS_DEVICES_H
#define MODBUS_DEVICES_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define MAX_MODBUS_DEVICES 1
#define MAX_REGISTERS_PER_DEVICE 20
#define DEVICE_NAME_MAX_LEN 32
#define DEVICE_DESC_MAX_LEN 64

typedef enum {
    REGISTER_TYPE_COIL = 0x01,
    REGISTER_TYPE_DISCRETE = 0x02,
    REGISTER_TYPE_HOLDING = 0x03,
    REGISTER_TYPE_INPUT = 0x04
} register_type_t;

typedef enum {
    DEVICE_STATUS_UNKNOWN = 0,
    DEVICE_STATUS_ONLINE = 1,
    DEVICE_STATUS_OFFLINE = 2,
    DEVICE_STATUS_ERROR = 3
} device_status_t;

typedef struct {
    uint16_t address;
    register_type_t type;
    char name[32];
    char unit[16];
    float scale;
    float offset;
    bool writable;
    char description[64];
    uint16_t last_value;
    uint32_t last_update;
} modbus_register_t;

typedef struct {
    uint8_t device_id;
    char name[DEVICE_NAME_MAX_LEN];
    char description[DEVICE_DESC_MAX_LEN];
    uint32_t poll_interval_ms;
    bool enabled;
    uint8_t last_error;
    uint64_t last_seen;
    device_status_t status;
    uint32_t poll_count;
    uint32_t error_count;
    uint16_t baudrate;
    uint8_t register_count;
    modbus_register_t registers[MAX_REGISTERS_PER_DEVICE];
} modbus_device_t;

esp_err_t modbus_devices_init(void);
esp_err_t modbus_devices_save(void);
esp_err_t modbus_devices_load(void);

esp_err_t modbus_add_device(const modbus_device_t *device);
esp_err_t modbus_update_device(uint8_t device_id, const modbus_device_t *device);
esp_err_t modbus_remove_device(uint8_t device_id);
modbus_device_t* modbus_get_device(uint8_t device_id);
modbus_device_t* modbus_list_devices(uint8_t *count);

esp_err_t modbus_add_register(uint8_t device_id, const modbus_register_t *reg);
esp_err_t modbus_update_register(uint8_t device_id, uint16_t address, const modbus_register_t *reg);
esp_err_t modbus_remove_register(uint8_t device_id, uint16_t address);
modbus_register_t* modbus_get_register(uint8_t device_id, uint16_t address);
esp_err_t modbus_update_register_value(uint8_t device_id, uint16_t address, uint16_t value);
float modbus_get_scaled_value(uint8_t device_id, uint16_t address);
uint16_t modbus_get_raw_value(uint8_t device_id, uint16_t address);

uint8_t modbus_get_device_count(void);
bool modbus_device_exists(uint8_t device_id);
esp_err_t modbus_clear_all_devices(void);

#endif