#include "driver/twai.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
extern QueueHandle_t xECU;
#include "freertos/event_groups.h"

#include "twai.h"
#include <inttypes.h> 

#include <string.h>

#define RX_GPIO_NUM 16
#define TX_GPIO_NUM 15

static const char * TWAI_TAG = "TWAI";

void receive_can_message() {
    twai_message_t received_msg;

    // Wait for a CAN message to be received
    esp_err_t ret = twai_receive(&received_msg, 500 / portTICK_PERIOD_MS); //1000
    //esp_err_t ret = twai_receive(&received_msg, pdMS_TO_TICKS(10));
    twai_status_info_t status;
    twai_get_status_info(&status);
    ESP_LOGI("TWAI", "TX ERR: %u, RX ERR: %u, State: %u",
            (unsigned int)status.tx_error_counter,
            (unsigned int)status.rx_error_counter,
            (unsigned int)status.state
        );    

    if (ret == ESP_OK) {
        uint16_t id = received_msg.identifier;
        uint8_t encoded_buffer[32];
        const void *fields[4];  // Adjust if more fields needed
        size_t encoded_size = 0;

        switch (id) {
            case 992: { // Coolant Temp & Oil Temp
                float coolant = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f;
                float oil = ((received_msg.data[6] << 8) | received_msg.data[7]) / 10.0f;

                fields[0] = &coolant;
                fields[1] = &oil;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 1136: { // Gear Selector & Gear
                uint8_t selector = received_msg.data[6];
                uint8_t gear = received_msg.data[7];

                fields[0] = &selector;
                fields[1] = &gear;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 864: { // RPM & Throttle
                uint16_t rpm = (received_msg.data[0] << 8) | received_msg.data[1];
                float throttle = ((received_msg.data[4] << 8) | received_msg.data[5]) / 10.0f;

                fields[0] = &rpm;
                fields[1] = &throttle;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 865: { // Fuel & Oil Pressure
                float fuel = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f - 101.3f;
                float oil = ((received_msg.data[2] << 8) | received_msg.data[3]) / 10.0f - 101.3f;

                fields[0] = &fuel;
                fields[1] = &oil;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 875: { // Brake Pressure
                float brake = ((received_msg.data[0] << 8) | received_msg.data[1]) - 101.3f;
                fields[0] = &brake;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 880: { // Vehicle Speed
                float speed = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f;
                fields[0] = &speed;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 882: { // Battery Voltage
                float voltage = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f;
                fields[0] = &voltage;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 1001: { // Lambda
                float lambda = ((received_msg.data[4] << 8) | received_msg.data[5]) / 1000.0f;
                fields[0] = &lambda;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            case 997: { // Ignition State
                uint8_t ignition = received_msg.data[0];
                fields[0] = &ignition;
                encoded_size = encode_message(id, fields, encoded_buffer);
                break;
            }

            default:
                return; // Message not handled
        }

        if (encoded_size > 0) {
            if (xQueueSend(xECU, encoded_buffer, portMAX_DELAY) != pdPASS) {
                ESP_LOGE(TWAI_TAG, "Failed to send encoded message to queue");
            }
        } else {
            ESP_LOGW(TWAI_TAG, "Encoding failed for ID: %u", id);
        }

    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TWAI_TAG, "CAN Receive Timeout");
    } else {
        twai_status_info_t status;
        twai_get_status_info(&status);
        // ESP_LOGW(TWAI_TAG,
        //     "Receive failed: %s | RX err: %" PRIu32 " | TX err: %" PRIu32 " | State: %" PRIu32,
        //     esp_err_to_name(ret),
        //     status.rx_error_counter,
        //     status.tx_error_counter,
        //     status.state);
    }
}

size_t encode_message(uint16_t message_id, const void *fields[], uint8_t *buffer) {
    size_t offset = 0;

    // ID
    buffer[offset++] = (message_id >> 8) & 0xFF;
    buffer[offset++] = message_id & 0xFF;

    const message_def_t *def = NULL;
    for (size_t i = 0; i < message_defs_count; i++) {
        if (message_defs[i].message_id == message_id) {
            def = &message_defs[i];
            break;
        }
    }
    if (!def) return 0;

    size_t i;
    for (i = 0; i < def->field_count; i++) {
        switch (def->fields[i]) {
            case FIELD_UINT8:
                buffer[offset++] = *(uint8_t*)fields[i];
                break;
            case FIELD_INT8:
                buffer[offset++] = *(int8_t*)fields[i];
                break;
            case FIELD_UINT16:
                {
                    uint16_t val = *(uint16_t*)fields[i];
                    buffer[offset++] = (val >> 8) & 0xFF;
                    buffer[offset++] = val & 0xFF;
                }
                break;
            case FIELD_FLOAT:
                memcpy(buffer + offset, fields[i], sizeof(float));
                offset += sizeof(float);
                break;
        }
    }

    // Pad remaining fields as zeros (assume each field takes 4 bytes worst case)
    while (i++ < MAX_FIELDS) {
        memset(buffer + offset, 0, sizeof(float));
        offset += sizeof(float);
    }

    return offset;
}

/*
void CAN_INIT(void) {
    // Configure the TWAI driver for 1Mbps and 11-bit IDs
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

    // Configure timing for 1MBit speed
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();//1MBITS();

    // Set filter to 11-bit CAN IDs
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all IDs for now

    // Install the TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));

    // Start the TWAI driver
    ESP_ERROR_CHECK(twai_start());

    ESP_LOGI(TWAI_TAG, "CAN driver installed and started.");
    
}
*/

void CAN_INIT(void) {
    // Setup CAN for 500 kbps, 29-bit extended IDs (Haltech uses extended IDs)
    /*twai_general_config_t g_config = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = TX_GPIO_NUM,
        .rx_io = RX_GPIO_NUM,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 5,
        .rx_queue_len = 10,
        .alerts_enabled = TWAI_ALERT_ALL,
        .clkout_divider = 0
    }; */

    // Configure the TWAI driver for 1Mbps and 11-bit IDs
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

    // Increase RX queue length from default (32) to 64 (or any desired value)
    g_config.rx_queue_len = 128;

    // Timing config for 500 kbps (typical for Haltech Elite)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();

    // Filter config to accept all extended IDs (you can narrow this down if needed)
    // Note: Acceptance filter is 29-bit extended IDs enabled here
    // Set filter to 11-bit CAN IDs
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all IDs for now

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    ESP_LOGI(TWAI_TAG, "CAN driver installed and started at 1MBIT, extended ID mode.");
}

void CAN_Task(void *pvParameters) {
    while (1) {
        receive_can_message();  // No need to lock LVGL here
        vTaskDelay(pdMS_TO_TICKS(100)); //100
    }
}