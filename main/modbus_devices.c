#include "modbus_devices.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MODBUS_DEVICES";
static const char *NVS_NAMESPACE = "modbus_config";

static modbus_device_t devices[MAX_MODBUS_DEVICES];
static uint8_t device_count = 0;

esp_err_t modbus_devices_init(void)
{
    memset(devices, 0, sizeof(devices));
    device_count = 0;
    ESP_LOGI(TAG, "Modbus devices manager initialized");
    return ESP_OK;
}

esp_err_t modbus_devices_save(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_u8(nvs_handle, "device_count", device_count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save device count: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    char key[16];
    for (uint8_t i = 0; i < device_count; i++) {
        snprintf(key, sizeof(key), "d%d_id", i);
        err = nvs_set_u8(nvs_handle, key, devices[i].device_id);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_id: %s", i, esp_err_to_name(err));
        }

        snprintf(key, sizeof(key), "d%d_name", i);
        err = nvs_set_str(nvs_handle, key, devices[i].name);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_name: %s", i, esp_err_to_name(err));
        }

        snprintf(key, sizeof(key), "d%d_desc", i);
        err = nvs_set_str(nvs_handle, key, devices[i].description);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_desc: %s", i, esp_err_to_name(err));
        }

        snprintf(key, sizeof(key), "d%d_poll", i);
        err = nvs_set_u32(nvs_handle, key, devices[i].poll_interval_ms);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_poll: %s", i, esp_err_to_name(err));
        }

        snprintf(key, sizeof(key), "d%d_en", i);
        err = nvs_set_u8(nvs_handle, key, devices[i].enabled);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_en: %s", i, esp_err_to_name(err));
        }

        snprintf(key, sizeof(key), "d%d_baud", i);
        err = nvs_set_u16(nvs_handle, key, devices[i].baudrate);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_baud: %s", i, esp_err_to_name(err));
        }

        snprintf(key, sizeof(key), "d%d_rc", i);
        err = nvs_set_u8(nvs_handle, key, devices[i].register_count);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save d%d_rc: %s", i, esp_err_to_name(err));
        }

        ESP_LOGI(TAG, "Saving %d register(s) for device %d", devices[i].register_count, i);
        for (uint8_t j = 0; j < devices[i].register_count; j++) {
            snprintf(key, sizeof(key), "d%dr%da", i, j);
            err = nvs_set_u16(nvs_handle, key, devices[i].registers[j].address);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%da: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%dt", i, j);
            err = nvs_set_u8(nvs_handle, key, devices[i].registers[j].type);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%dt: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%dn", i, j);
            err = nvs_set_str(nvs_handle, key, devices[i].registers[j].name);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%dn: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%du", i, j);
            err = nvs_set_str(nvs_handle, key, devices[i].registers[j].unit);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%du: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%ds", i, j);
            err = nvs_set_u32(nvs_handle, key, *(uint32_t*)&devices[i].registers[j].scale);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%ds: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%do", i, j);
            err = nvs_set_u32(nvs_handle, key, *(uint32_t*)&devices[i].registers[j].offset);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%do: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%dw", i, j);
            err = nvs_set_u8(nvs_handle, key, devices[i].registers[j].writable);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%dw: %s", i, j, esp_err_to_name(err));
            }

            snprintf(key, sizeof(key), "d%dr%dd", i, j);
            err = nvs_set_str(nvs_handle, key, devices[i].registers[j].description);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to save d%dr%dd: %s", i, j, esp_err_to_name(err));
            }
        }
    }

    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Saved %d device(s) to NVS successfully", device_count);
        for (uint8_t i = 0; i < device_count; i++) {
            ESP_LOGI(TAG, "  Device %d: ID=%d, Name='%s', Regs=%d", i, devices[i].device_id, devices[i].name, devices[i].register_count);
        }
    } else {
        ESP_LOGE(TAG, "Failed to commit NVS: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t modbus_devices_load(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No modbus configuration found in NVS");
        return ESP_OK;
    }
    
    err = nvs_get_u8(nvs_handle, "device_count", &device_count);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No devices found in NVS");
        nvs_close(nvs_handle);
        return ESP_OK;
    }
    
    if (device_count > MAX_MODBUS_DEVICES) {
        device_count = MAX_MODBUS_DEVICES;
        ESP_LOGW(TAG, "Device count exceeds maximum, limiting to %d", MAX_MODBUS_DEVICES);
    }
    
    char key[16];
    bool load_success = true;
    
    for (uint8_t i = 0; i < device_count; i++) {
        snprintf(key, sizeof(key), "d%d_id", i);
        err = nvs_get_u8(nvs_handle, key, &devices[i].device_id);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read d%d_id: %s", i, esp_err_to_name(err));
            devices[i].device_id = i;
            load_success = false;
        }
        
        snprintf(key, sizeof(key), "d%d_name", i);
        size_t len = sizeof(devices[i].name);
        err = nvs_get_str(nvs_handle, key, devices[i].name, &len);
        if (err != ESP_OK || len == 0) {
            ESP_LOGE(TAG, "Failed to read d%d_name: %s", i, esp_err_to_name(err));
            memset(devices[i].name, 0, sizeof(devices[i].name));
            strcpy(devices[i].name, "Unnamed Device");
            load_success = false;
        }
        
        snprintf(key, sizeof(key), "d%d_desc", i);
        len = sizeof(devices[i].description);
        err = nvs_get_str(nvs_handle, key, devices[i].description, &len);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read d%d_desc: %s", i, esp_err_to_name(err));
            memset(devices[i].description, 0, sizeof(devices[i].description));
            load_success = false;
        }
        
        snprintf(key, sizeof(key), "d%d_poll", i);
        err = nvs_get_u32(nvs_handle, key, &devices[i].poll_interval_ms);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read d%d_poll: %s", i, esp_err_to_name(err));
            devices[i].poll_interval_ms = 5000;
            load_success = false;
        }
        
        snprintf(key, sizeof(key), "d%d_en", i);
        err = nvs_get_u8(nvs_handle, key, (uint8_t*)&devices[i].enabled);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read d%d_en: %s", i, esp_err_to_name(err));
            devices[i].enabled = 1;
            load_success = false;
        }
        
        snprintf(key, sizeof(key), "d%d_baud", i);
        err = nvs_get_u16(nvs_handle, key, &devices[i].baudrate);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read d%d_baud: %s", i, esp_err_to_name(err));
            devices[i].baudrate = 9600;
            load_success = false;
        }
        
        snprintf(key, sizeof(key), "d%d_rc", i);
        err = nvs_get_u8(nvs_handle, key, &devices[i].register_count);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read d%d_rc: %s", i, esp_err_to_name(err));
            devices[i].register_count = 0;
            load_success = false;
        }
        
        if (devices[i].register_count > MAX_REGISTERS_PER_DEVICE) {
            ESP_LOGW(TAG, "Device_%d has %d registers, limiting to %d", i, devices[i].register_count, MAX_REGISTERS_PER_DEVICE);
            devices[i].register_count = MAX_REGISTERS_PER_DEVICE;
        }
        
        ESP_LOGI(TAG, "Loading %d register(s) for device %d", devices[i].register_count, i);
        
        for (uint8_t j = 0; j < devices[i].register_count; j++) {
            snprintf(key, sizeof(key), "d%dr%da", i, j);
            err = nvs_get_u16(nvs_handle, key, &devices[i].registers[j].address);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to read d%dr%da: %s", i, j, esp_err_to_name(err));
                load_success = false;
            }
            
            snprintf(key, sizeof(key), "d%dr%dt", i, j);
            uint8_t type;
            err = nvs_get_u8(nvs_handle, key, &type);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to read d%dr%dt: %s", i, j, esp_err_to_name(err));
                devices[i].registers[j].type = REGISTER_TYPE_HOLDING;
                load_success = false;
            } else {
                devices[i].registers[j].type = (register_type_t)type;
            }
            
            snprintf(key, sizeof(key), "d%dr%dn", i, j);
            len = sizeof(devices[i].registers[j].name);
            err = nvs_get_str(nvs_handle, key, devices[i].registers[j].name, &len);
            if (err != ESP_OK || len == 0) {
                ESP_LOGE(TAG, "Failed to read d%dr%dn: %s", i, j, esp_err_to_name(err));
                memset(devices[i].registers[j].name, 0, sizeof(devices[i].registers[j].name));
                strcpy(devices[i].registers[j].name, "Unnamed");
                load_success = false;
            }
            
            snprintf(key, sizeof(key), "d%dr%du", i, j);
            len = sizeof(devices[i].registers[j].unit);
            err = nvs_get_str(nvs_handle, key, devices[i].registers[j].unit, &len);
            if (err != ESP_OK) {
                memset(devices[i].registers[j].unit, 0, sizeof(devices[i].registers[j].unit));
            }
            
            snprintf(key, sizeof(key), "d%dr%ds", i, j);
            uint32_t scale_val;
            err = nvs_get_u32(nvs_handle, key, &scale_val);
            if (err == ESP_OK) {
                devices[i].registers[j].scale = *(float*)&scale_val;
            } else {
                devices[i].registers[j].scale = 1.0;
            }
            
            snprintf(key, sizeof(key), "d%dr%do", i, j);
            uint32_t offset_val;
            err = nvs_get_u32(nvs_handle, key, &offset_val);
            if (err == ESP_OK) {
                devices[i].registers[j].offset = *(float*)&offset_val;
            } else {
                devices[i].registers[j].offset = 0.0;
            }
            
            snprintf(key, sizeof(key), "d%dr%dw", i, j);
            err = nvs_get_u8(nvs_handle, key, (uint8_t*)&devices[i].registers[j].writable);
            if (err != ESP_OK) {
                devices[i].registers[j].writable = 0;
            }
            
            snprintf(key, sizeof(key), "d%dr%dd", i, j);
            len = sizeof(devices[i].registers[j].description);
            err = nvs_get_str(nvs_handle, key, devices[i].registers[j].description, &len);
            if (err != ESP_OK) {
                memset(devices[i].registers[j].description, 0, sizeof(devices[i].registers[j].description));
            }
            
            devices[i].registers[j].last_value = 0;
            devices[i].registers[j].last_update = 0;
            
            ESP_LOGI(TAG, "  Loaded reg_%d: Addr=%d, Type=%d, Name='%s'", j, devices[i].registers[j].address, devices[i].registers[j].type, devices[i].registers[j].name);
        }
        
        devices[i].last_error = 0;
        devices[i].last_seen = 0;
        devices[i].status = DEVICE_STATUS_UNKNOWN;
        devices[i].poll_count = 0;
        devices[i].error_count = 0;
    }
    
    nvs_close(nvs_handle);
    
    if (!load_success) {
        ESP_LOGW(TAG, "Loaded %d device(s) from NVS with some errors", device_count);
    } else {
        ESP_LOGI(TAG, "Loaded %d device(s) from NVS successfully", device_count);
    }
    
    for (uint8_t i = 0; i < device_count; i++) {
        ESP_LOGI(TAG, "Device %d: ID=%d, Name='%s', Baud=%d, Poll=%dms, Regs=%d",
                 i, devices[i].device_id, devices[i].name, devices[i].baudrate,
                 devices[i].poll_interval_ms, devices[i].register_count);
    }
    
    return ESP_OK;
}

esp_err_t modbus_add_device(const modbus_device_t *device)
{
    if (device_count >= MAX_MODBUS_DEVICES) {
        ESP_LOGE(TAG, "Maximum number of devices reached");
        return ESP_ERR_NO_MEM;
    }

    if (modbus_device_exists(device->device_id)) {
        ESP_LOGE(TAG, "Device ID %d already exists", device->device_id);
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&devices[device_count], device, sizeof(modbus_device_t));
    devices[device_count].last_error = 0;
    devices[device_count].last_seen = 0;
    devices[device_count].status = DEVICE_STATUS_UNKNOWN;
    devices[device_count].poll_count = 0;
    devices[device_count].error_count = 0;
    device_count++;

    ESP_LOGI(TAG, "Added device: ID=%d, Name=%s", device->device_id, device->name);
    return ESP_OK;
}

esp_err_t modbus_update_device(uint8_t device_id, const modbus_device_t *device)
{
    for (uint8_t i = 0; i < device_count; i++) {
        if (devices[i].device_id == device_id) {
            memcpy(&devices[i], device, sizeof(modbus_device_t));
            ESP_LOGI(TAG, "Updated device: ID=%d", device_id);
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

esp_err_t modbus_remove_device(uint8_t device_id)
{
    for (uint8_t i = 0; i < device_count; i++) {
        if (devices[i].device_id == device_id) {
            if (i < device_count - 1) {
                memmove(&devices[i], &devices[i + 1], (device_count - 1 - i) * sizeof(modbus_device_t));
            }
            device_count--;
            ESP_LOGI(TAG, "Removed device ID=%d", device_id);
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

modbus_device_t* modbus_get_device(uint8_t device_id)
{
    for (uint8_t i = 0; i < device_count; i++) {
        if (devices[i].device_id == device_id) {
            return &devices[i];
        }
    }
    return NULL;
}

modbus_device_t* modbus_list_devices(uint8_t *count)
{
    if (count == NULL) {
        return NULL;
    }

    if (device_count == 0) {
        *count = 0;
        return NULL;
    }

    *count = device_count;
    return devices;
}

esp_err_t modbus_add_register(uint8_t device_id, const modbus_register_t *reg)
{
    modbus_device_t *device = modbus_get_device(device_id);
    if (device == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    if (device->register_count >= MAX_REGISTERS_PER_DEVICE) {
        return ESP_ERR_NO_MEM;
    }

    for (uint8_t i = 0; i < device->register_count; i++) {
        if (device->registers[i].address == reg->address && device->registers[i].type == reg->type) {
            ESP_LOGW(TAG, "Register address %d (Type %d) already exists for device %d", 
                      reg->address, reg->type, device_id);
            return ESP_ERR_INVALID_ARG;
        }
    }

    memcpy(&device->registers[device->register_count], reg, sizeof(modbus_register_t));
    device->registers[device->register_count].last_value = 0;
    device->registers[device->register_count].last_update = 0;
    device->register_count++;

    ESP_LOGI(TAG, "Added register: Device=%d, Addr=%d, Name=%s", device_id, reg->address, reg->name);
    return ESP_OK;
}

esp_err_t modbus_update_register(uint8_t device_id, uint16_t address, const modbus_register_t *reg)
{
    modbus_device_t *device = modbus_get_device(device_id);
    if (device == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    for (uint8_t i = 0; i < device->register_count; i++) {
        if (device->registers[i].address == address) {
            uint16_t last_val = device->registers[i].last_value;
            uint32_t last_upd = device->registers[i].last_update;
            memcpy(&device->registers[i], reg, sizeof(modbus_register_t));
            device->registers[i].last_value = last_val;
            device->registers[i].last_update = last_upd;
            ESP_LOGI(TAG, "Updated register: Device=%d, Addr=%d", device_id, address);
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

esp_err_t modbus_remove_register(uint8_t device_id, uint16_t address)
{
    modbus_device_t *device = modbus_get_device(device_id);
    if (device == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    for (uint8_t i = 0; i < device->register_count; i++) {
        if (device->registers[i].address == address) {
            if (i < device->register_count - 1) {
                memmove(&device->registers[i], &device->registers[i + 1], (device->register_count - 1 - i) * sizeof(modbus_register_t));
            }
            device->register_count--;
            ESP_LOGI(TAG, "Removed register: Device=%d, Addr=%d", device_id, address);
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

modbus_register_t* modbus_get_register(uint8_t device_id, uint16_t address)
{
    modbus_device_t *device = modbus_get_device(device_id);
    if (device == NULL) {
        return NULL;
    }

    for (uint8_t i = 0; i < device->register_count; i++) {
        if (device->registers[i].address == address) {
            return &device->registers[i];
        }
    }
    return NULL;
}

esp_err_t modbus_update_register_value(uint8_t device_id, uint16_t address, uint16_t value)
{
    modbus_register_t *reg = modbus_get_register(device_id, address);
    if (reg == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    reg->last_value = value;
    reg->last_update = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return ESP_OK;
}

float modbus_get_scaled_value(uint8_t device_id, uint16_t address)
{
    modbus_register_t *reg = modbus_get_register(device_id, address);
    if (reg == NULL) {
        return 0.0f;
    }

    return (float)reg->last_value * reg->scale + reg->offset;
}

uint16_t modbus_get_raw_value(uint8_t device_id, uint16_t address)
{
    modbus_register_t *reg = modbus_get_register(device_id, address);
    if (reg == NULL) {
        return 0;
    }

    return reg->last_value;
}

uint8_t modbus_get_device_count(void)
{
    return device_count;
}

bool modbus_device_exists(uint8_t device_id)
{
    return modbus_get_device(device_id) != NULL;
}

esp_err_t modbus_clear_all_devices(void)
{
    memset(devices, 0, sizeof(devices));
    device_count = 0;
    
    nvs_handle_t nvs_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle) == ESP_OK) {
        nvs_erase_all(nvs_handle);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }
    
    ESP_LOGI(TAG, "Cleared all devices");
    return ESP_OK;
}