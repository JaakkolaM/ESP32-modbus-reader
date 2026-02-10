#include "modbus_protocol.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "MODBUS_PROTOCOL";

uint16_t modbus_calculate_crc(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

bool modbus_validate_crc(const uint8_t *data, uint16_t length)
{
    if (length < 3) {
        return false;
    }
    
    uint16_t received_crc = (data[length - 1] << 8) | data[length - 2];
    uint16_t calculated_crc = modbus_calculate_crc(data, length - 2);
    
    return received_crc == calculated_crc;
}

esp_err_t modbus_build_request(uint8_t device_id, uint8_t function, 
                              uint16_t address, uint16_t quantity,
                              const uint8_t *data, uint16_t data_len,
                              uint8_t *frame, uint16_t *frame_len)
{
    if (frame_len == NULL || frame == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint16_t index = 0;

    frame[index++] = device_id;
    frame[index++] = function;

    if (function == MODBUS_FC_WRITE_SINGLE_COIL || function == MODBUS_FC_WRITE_SINGLE_REGISTER) {
        frame[index++] = (address >> 8) & 0xFF;
        frame[index++] = address & 0xFF;

        if (function == MODBUS_FC_WRITE_SINGLE_COIL) {
            frame[index++] = data[0] ? 0xFF : 0x00;
            frame[index++] = 0x00;
        } else {
            frame[index++] = (data[0] >> 8) & 0xFF;
            frame[index++] = data[0] & 0xFF;
        }
    } else {
        frame[index++] = (address >> 8) & 0xFF;
        frame[index++] = address & 0xFF;
        frame[index++] = (quantity >> 8) & 0xFF;
        frame[index++] = quantity & 0xFF;

        if (data != NULL && data_len > 0) {
            if (function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS) {
                frame[index++] = data_len * 2;
                for (uint16_t i = 0; i < data_len; i++) {
                    frame[index++] = (data[i] >> 8) & 0xFF;
                    frame[index++] = data[i] & 0xFF;
                }
            } else if (function == MODBUS_FC_WRITE_MULTIPLE_COILS) {
                frame[index++] = data_len;
                for (uint16_t i = 0; i < data_len; i++) {
                    frame[index++] = data[i];
                }
            }
        }
    }
    
    uint16_t crc = modbus_calculate_crc(frame, index);
    frame[index++] = crc & 0xFF;
    frame[index++] = (crc >> 8) & 0xFF;
    
    *frame_len = index;
    
    return ESP_OK;
}

esp_err_t modbus_parse_response(const uint8_t *frame, uint16_t frame_len,
                               modbus_response_t *response)
{
    if (response == NULL || frame == NULL || frame_len < 3) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!modbus_validate_crc(frame, frame_len)) {
        ESP_LOGE(TAG, "CRC validation failed");
        return ESP_FAIL;
    }
    
    memset(response, 0, sizeof(modbus_response_t));
    
    response->device_id = frame[0];
    response->function = frame[1];
    
    if (response->function & 0x80) {
        response->is_exception = true;
        response->exception_code = frame[2];
        return ESP_OK;
    }
    
    response->is_exception = false;
    
    switch (response->function) {
        case MODBUS_FC_READ_COILS:
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            if (frame_len < 3) return ESP_ERR_INVALID_ARG;
            response->byte_count = frame[2];
            if (response->byte_count > MODBUS_MAX_DATA_LEN) {
                return ESP_ERR_INVALID_ARG;
            }
            if (frame_len < (3 + response->byte_count + 2)) {
                return ESP_ERR_INVALID_ARG;
            }
            memcpy(response->data, &frame[3], response->byte_count);
            break;
            
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
            if (frame_len < 3) return ESP_ERR_INVALID_ARG;
            response->byte_count = frame[2];
            if (response->byte_count > MODBUS_MAX_DATA_LEN) {
                return ESP_ERR_INVALID_ARG;
            }
            if (frame_len < (3 + response->byte_count + 2)) {
                return ESP_ERR_INVALID_ARG;
            }
            memcpy(response->data, &frame[3], response->byte_count);
            break;
            
        case MODBUS_FC_WRITE_SINGLE_COIL:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            if (frame_len < 6) return ESP_ERR_INVALID_ARG;
            response->byte_count = 4;
            memcpy(response->data, &frame[2], 4);
            break;
            
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            if (frame_len < 6) return ESP_ERR_INVALID_ARG;
            response->byte_count = 4;
            memcpy(response->data, &frame[2], 4);
            break;
            
        default:
            return ESP_ERR_NOT_SUPPORTED;
    }
    
    response->crc = (frame[frame_len - 1] << 8) | frame[frame_len - 2];
    
    return ESP_OK;
}

esp_err_t modbus_build_exception_response(uint8_t device_id, uint8_t function,
                                        uint8_t exception_code,
                                        uint8_t *frame, uint16_t *frame_len)
{
    if (frame == NULL || frame_len == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    frame[0] = device_id;
    frame[1] = function | 0x80;
    frame[2] = exception_code;
    
    uint16_t crc = modbus_calculate_crc(frame, 3);
    frame[3] = crc & 0xFF;
    frame[4] = (crc >> 8) & 0xFF;
    
    *frame_len = 5;
    
    return ESP_OK;
}

const char* modbus_exception_to_string(uint8_t exception_code)
{
    switch (exception_code) {
        case MODBUS_EXCEPTION_ILLEGAL_FUNCTION:
            return "Illegal function";
        case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
            return "Illegal data address";
        case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE:
            return "Illegal data value";
        case MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE:
            return "Server device failure";
        case MODBUS_EXCEPTION_ACKNOWLEDGE:
            return "Acknowledge";
        case MODBUS_EXCEPTION_SERVER_DEVICE_BUSY:
            return "Server device busy";
        case MODBUS_EXCEPTION_MEMORY_PARITY_ERROR:
            return "Memory parity error";
        case MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE:
            return "Gateway path unavailable";
        case MODBUS_EXCEPTION_GATEWAY_TARGET_DEVICE_FAILED:
            return "Gateway target device failed";
        default:
            return "Unknown exception";
    }
}

const char* modbus_function_to_string(uint8_t function_code)
{
    switch (function_code) {
        case MODBUS_FC_READ_COILS:
            return "Read Coils";
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            return "Read Discrete Inputs";
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            return "Read Holding Registers";
        case MODBUS_FC_READ_INPUT_REGISTERS:
            return "Read Input Registers";
        case MODBUS_FC_WRITE_SINGLE_COIL:
            return "Write Single Coil";
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            return "Write Single Register";
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
            return "Write Multiple Coils";
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            return "Write Multiple Registers";
        default:
            return "Unknown function";
    }
}