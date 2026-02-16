#include "mqtt_client.h"
#include "nvs_storage.h"
#include "esp_log.h"
#include "esp_mqtt_client.h"
#include "esp_wifi.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

static const char *TAG = "MQTT_CLIENT";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static mqtt_config_t mqtt_config;
static mqtt_connection_state_t mqtt_state = MQTT_STATE_DISCONNECTED;
static mqtt_register_write_cb_t write_callback = NULL;
static bool mqtt_initialized = false;

static void mqtt_parse_set_message(const char *topic, const char *payload);

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_state = MQTT_STATE_CONNECTED;
        mqtt_client_publish_lwt(true);
        mqtt_client_publish_discovery();
        mqtt_client_publish_all_registers();
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_state = MQTT_STATE_DISCONNECTED;
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        char topic_buf[128];
        char data_buf[256];
        int copy_len = (event->topic_len < 127) ? event->topic_len : 127;
        memcpy(topic_buf, event->topic, copy_len);
        topic_buf[copy_len] = '\0';
        
        copy_len = (event->data_len < 255) ? event->data_len : 255;
        memcpy(data_buf, event->data, copy_len);
        data_buf[copy_len] = '\0';
        
        ESP_LOGI(TAG, "Received: topic=%s, data=%s", topic_buf, data_buf);
        mqtt_parse_set_message(topic_buf, data_buf);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        mqtt_state = MQTT_STATE_ERROR;
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string: %s", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_subscribe_to_registers(void)
{
    if (mqtt_client == NULL || mqtt_state != MQTT_STATE_CONNECTED) {
        return;
    }

    uint8_t device_count = 0;
    modbus_device_t *devices = modbus_list_devices(&device_count);

    if (devices == NULL) {
        return;
    }

    for (uint8_t i = 0; i < device_count; i++) {
        for (uint8_t j = 0; j < devices[i].register_count; j++) {
            if (devices[i].registers[j].writable) {
                char topic[128];
                snprintf(topic, sizeof(topic), "%s/%d/%d/set", 
                         mqtt_config.prefix, 
                         devices[i].device_id, 
                         devices[i].registers[j].address);
                
                int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, 0);
                ESP_LOGI(TAG, "Subscribed to %s, msg_id=%d", topic, msg_id);
            }
        }
    }
}

static void mqtt_parse_set_message(const char *topic, const char *payload)
{
    char prefix_topic[64];
    snprintf(prefix_topic, sizeof(prefix_topic), "%s/", mqtt_config.prefix);
    
    char *topic_ptr = strstr(topic, prefix_topic);
    if (topic_ptr == NULL) {
        return;
    }
    
    topic_ptr += strlen(prefix_topic);
    
    uint8_t device_id;
    uint16_t address;
    if (sscanf(topic_ptr, "%hhu/%hu/set", &device_id, &address) == 2) {
        uint16_t value = atoi(payload);
        ESP_LOGI(TAG, "MQTT set: device=%d, address=%d, value=%d", device_id, address, value);
        
        if (write_callback != NULL) {
            write_callback(device_id, address, value);
        }
    }
}

esp_err_t mqtt_client_init(void)
{
    if (mqtt_initialized) {
        return ESP_OK;
    }

    memset(&mqtt_config, 0, sizeof(mqtt_config));
    mqtt_state = MQTT_STATE_DISCONNECTED;
    mqtt_initialized = true;

    ESP_LOGI(TAG, "MQTT client initialized");
    return ESP_OK;
}

esp_err_t mqtt_client_start(const mqtt_config_t *config)
{
    if (!mqtt_initialized) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_FAIL;
    }

    if (config == NULL || !config->enabled || strlen(config->broker) == 0) {
        ESP_LOGI(TAG, "MQTT disabled or not configured");
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(&mqtt_config, config, sizeof(mqtt_config_t));

    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }

    char mqtt_uri[128];
    if (strlen(mqtt_config.username) > 0) {
        snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtt://%s:%s@%s:%d", 
                 mqtt_config.username, mqtt_config.password, 
                 mqtt_config.broker, mqtt_config.port);
    } else {
        snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtt://%s:%d", 
                 mqtt_config.broker, mqtt_config.port);
    }

    ESP_LOGI(TAG, "Connecting to MQTT broker: %s", mqtt_uri);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = mqtt_uri,
        .credentials.client_id = "esp32modbus",
        .session.keepalive = 60,
        .session.last_will.topic = "esp32modbus/tele/LWT",
        .session.last_will.msg = "Offline",
        .session.last_will.qos = 0,
        .session.last_will.retain = true
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    mqtt_state = MQTT_STATE_CONNECTING;
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        mqtt_state = MQTT_STATE_ERROR;
        return err;
    }

    ESP_LOGI(TAG, "MQTT client started");
    return ESP_OK;
}

esp_err_t mqtt_client_stop(void)
{
    if (mqtt_client != NULL) {
        mqtt_client_publish_lwt(false);
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
    mqtt_state = MQTT_STATE_DISCONNECTED;
    ESP_LOGI(TAG, "MQTT client stopped");
    return ESP_OK;
}

bool mqtt_client_is_connected(void)
{
    return (mqtt_state == MQTT_STATE_CONNECTED);
}

mqtt_connection_state_t mqtt_client_get_state(void)
{
    return mqtt_state;
}

esp_err_t mqtt_client_publish_register(uint8_t device_id, const char *device_name, 
                                        const modbus_register_t *reg)
{
    if (mqtt_client == NULL || mqtt_state != MQTT_STATE_CONNECTED) {
        return ESP_FAIL;
    }

    char topic[128];
    char payload[64];

    snprintf(topic, sizeof(topic), "%s/%d/%d/state", 
             mqtt_config.prefix, device_id, reg->address);

    if (reg->type == REGISTER_TYPE_COIL || reg->type == REGISTER_TYPE_DISCRETE) {
        snprintf(payload, sizeof(payload), "%s", reg->last_value ? "ON" : "OFF");
    } else {
        float scaled = (float)reg->last_value * reg->scale + reg->offset;
        snprintf(payload, sizeof(payload), "%.2f", scaled);
    }

    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 0, 1);
    ESP_LOGI(TAG, "Published %s: %s (msg_id=%d)", topic, payload, msg_id);

    return ESP_OK;
}

esp_err_t mqtt_client_publish_all_registers(void)
{
    if (mqtt_client == NULL || mqtt_state != MQTT_STATE_CONNECTED) {
        return ESP_FAIL;
    }

    uint8_t device_count = 0;
    modbus_device_t *devices = modbus_list_devices(&device_count);

    if (devices == NULL) {
        return ESP_OK;
    }

    for (uint8_t i = 0; i < device_count; i++) {
        for (uint8_t j = 0; j < devices[i].register_count; j++) {
            mqtt_client_publish_register(devices[i].device_id, devices[i].name, 
                                        &devices[i].registers[j]);
        }
    }

    return ESP_OK;
}

esp_err_t mqtt_client_publish_discovery(void)
{
    if (mqtt_client == NULL || mqtt_state != MQTT_STATE_CONNECTED) {
        return ESP_FAIL;
    }

    uint8_t device_count = 0;
    modbus_device_t *devices = modbus_list_devices(&device_count);

    if (devices == NULL) {
        return ESP_OK;
    }

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char device_id[16];
    snprintf(device_id, sizeof(device_id), "esp32modbus_%02x%02x%02x", mac[3], mac[4], mac[5]);

    for (uint8_t i = 0; i < device_count; i++) {
        for (uint8_t j = 0; j < devices[i].register_count; j++) {
            char topic[128];
            char payload[512];
            char unique_id[64];
            
            snprintf(unique_id, sizeof(unique_id), "%s_%d_%d", 
                    device_id, devices[i].device_id, devices[i].registers[j].address);

            const char *ha_type;
            const char *value_template;
            
            if (devices[i].registers[j].type == REGISTER_TYPE_COIL && devices[i].registers[j].writable) {
                ha_type = "switch";
                snprintf(topic, sizeof(topic), "homeassistant/switch/%s/config", unique_id);
                snprintf(payload, sizeof(payload),
                    "{\"name\": \"%s\", \"command_topic\": \"%s/%d/%d/set\", "
                    "\"state_topic\": \"%s/%d/%d/state\", "
                    "\"unique_id\": \"%s\", "
                    "\"device\": {\"name\": \"ESP32 Modbus\", \"identifiers\": \"%s\", \"manufacturer\": \"Custom\", \"model\": \"ESP32-C3\"}}",
                    devices[i].registers[j].name,
                    mqtt_config.prefix, devices[i].device_id, devices[i].registers[j].address,
                    mqtt_config.prefix, devices[i].device_id, devices[i].registers[j].address,
                    unique_id, device_id);
            } else if (devices[i].registers[j].writable) {
                ha_type = "number";
                snprintf(topic, sizeof(topic), "homeassistant/number/%s/config", unique_id);
                snprintf(payload, sizeof(payload),
                    "{\"name\": \"%s\", \"command_topic\": \"%s/%d/%d/set\", "
                    "\"state_topic\": \"%s/%d/%d/state\", "
                    "\"value_template\": \"{{ value }}\", "
                    "\"unique_id\": \"%s\", "
                    "\"device\": {\"name\": \"ESP32 Modbus\", \"identifiers\": \"%s\", \"manufacturer\": \"Custom\", \"model\": \"ESP32-C3\"}}",
                    devices[i].registers[j].name,
                    mqtt_config.prefix, devices[i].device_id, devices[i].registers[j].address,
                    mqtt_config.prefix, devices[i].device_id, devices[i].registers[j].address,
                    unique_id, device_id);
            } else {
                ha_type = "sensor";
                snprintf(topic, sizeof(topic), "homeassistant/sensor/%s/config", unique_id);
                snprintf(payload, sizeof(payload),
                    "{\"name\": \"%s\", \"state_topic\": \"%s/%d/%d/state\", "
                    "\"unit_of_measurement\": \"%s\", "
                    "\"value_template\": \"{{ value }}\", "
                    "\"unique_id\": \"%s\", "
                    "\"device\": {\"name\": \"ESP32 Modbus\", \"identifiers\": \"%s\", \"manufacturer\": \"Custom\", \"model\": \"ESP32-C3\"}}",
                    devices[i].registers[j].name,
                    mqtt_config.prefix, devices[i].device_id, devices[i].registers[j].address,
                    devices[i].registers[j].unit,
                    unique_id, device_id);
            }

            int msg_id = esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, true);
            ESP_LOGI(TAG, "Published HA discovery: %s (msg_id=%d)", ha_type, msg_id);
        }
    }

    return ESP_OK;
}

esp_err_t mqtt_client_publish_lwt(bool online)
{
    if (mqtt_client == NULL || mqtt_state != MQTT_STATE_CONNECTED) {
        return ESP_FAIL;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/tele/LWT", mqtt_config.prefix);
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, online ? "Online" : "Offline", 0, 0, 1);
    ESP_LOGI(TAG, "Published LWT: %s (msg_id=%d)", online ? "Online" : "Offline", msg_id);

    return ESP_OK;
}

void mqtt_client_set_register_write_callback(mqtt_register_write_cb_t callback)
{
    write_callback = callback;
}

esp_err_t mqtt_client_update_config(const mqtt_config_t *config)
{
    memcpy(&mqtt_config, config, sizeof(mqtt_config_t));
    
    if (mqtt_client != NULL) {
        mqtt_client_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
        mqtt_client_start(config);
    }
    
    return ESP_OK;
}
