#ifndef MQTT_GATEWAY_H
#define MQTT_GATEWAY_H

#include <stdbool.h>
#include "esp_err.h"
#include <mqtt_client.h>
#include "modbus_devices.h"
#include "nvs_storage.h"

#define MQTT_CLIENT_TAG "MQTT_CLIENT"

typedef enum {
    MQTT_STATE_DISCONNECTED = 0,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_ERROR
} mqtt_connection_state_t;

typedef void (*mqtt_register_write_cb_t)(uint8_t device_id, uint16_t address, uint16_t value);

esp_err_t mqtt_client_init(void);
esp_err_t mqtt_client_start(const mqtt_config_t *config);
esp_err_t mqtt_client_stop(void);
bool mqtt_client_is_connected(void);
mqtt_connection_state_t mqtt_client_get_state(void);

esp_err_t mqtt_client_publish_register(uint8_t device_id, const char *device_name, 
                                       const modbus_register_t *reg);
esp_err_t mqtt_client_publish_all_registers(void);
esp_err_t mqtt_client_publish_discovery(void);
esp_err_t mqtt_client_publish_lwt(bool online);

void mqtt_client_set_register_write_callback(mqtt_register_write_cb_t callback);

esp_err_t mqtt_client_update_config(const mqtt_config_t *config);

#endif