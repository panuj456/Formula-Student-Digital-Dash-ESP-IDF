#include "driver/twai.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <inttypes.h>

#include "twai.h"

#define RX_GPIO_NUM 16
#define TX_GPIO_NUM 15

static const char *TWAI_TAG = "TWAI";

extern QueueHandle_t xECU;

bool is_accepted_id(uint32_t id) {
    const uint16_t accepted_ids[] = {
        0x3E0, 0x470, 0x360, 0x361, 0x370, 0x372, 0x3E9, 0x36B, 0x6F3 
    };
    for (int i = 0; i < sizeof(accepted_ids) / sizeof(accepted_ids[0]); i++) {
        if (accepted_ids[i] == id) return true;
        //if (0x470 == id) return true;
    }
    return false;
}
//consider higher priority filterug
void receive_can_message() {
    twai_message_t received_msg;
    //esp_err_t ret = twai_receive(&received_msg, 500 / portTICK_PERIOD_MS);
    esp_err_t ret = twai_receive(&received_msg, 5 / portTICK_PERIOD_MS);


    twai_status_info_t status;
    twai_get_status_info(&status);

    ESP_LOGI(TWAI_TAG, "Time: %lu, TX ERR: %u, RX ERR: %u, State: %u",
             (unsigned long)esp_timer_get_time(),
             (unsigned int)status.tx_error_counter,
             (unsigned int)status.rx_error_counter,
             (unsigned int)status.state);

    if (ret == ESP_OK) {
        uint16_t id = received_msg.identifier;
        if (!is_accepted_id(id)) return;

        /*printf("Time: %lu - Received CAN ID: 0x%lX, DLC: %d, Data: ",
               (unsigned long)esp_timer_get_time(), received_msg.identifier, received_msg.data_length_code);
        for (int i = 0; i < received_msg.data_length_code; i++) {
            printf("%02X ", received_msg.data[i]);
        }
        printf("\n"); */

        encoded_message_t encoded = { .length = 0 };

        encoded.data[encoded.length++] = (id >> 8) & 0xFF;
        encoded.data[encoded.length++] = id & 0xFF;

        switch (id) {
            case 0x470: {
                uint8_t gear = received_msg.data[7];
                encoded.data[encoded.length++] = gear;
                //printf("gear value at twai.c: %d\n", gear);
                break;
            }
            case 0x360: {
                uint16_t rpm = (received_msg.data[0] << 8) | received_msg.data[1];
                float throttle = ((received_msg.data[4] << 8) | received_msg.data[5]) / 10.0f;
                memcpy(encoded.data + encoded.length, &rpm, sizeof(uint16_t));
                encoded.length += sizeof(uint16_t);
                memcpy(encoded.data + encoded.length, &throttle, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            case 0x3E0: { // Coolant & Oil Temp
                float coolant = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f;
                float oil = ((received_msg.data[6] << 8) | received_msg.data[7]) / 10.0f;
                coolant = coolant - 273.15f;
                oil = oil - 273.15f;
                memcpy(encoded.data + encoded.length, &coolant, sizeof(float));
                encoded.length += sizeof(float);
                memcpy(encoded.data + encoded.length, &oil, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            case 0x361: {
                float fuel = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f - 101.3f;
                float oilp = ((received_msg.data[2] << 8) | received_msg.data[3]) / 10.0f - 101.3f;
                memcpy(encoded.data + encoded.length, &fuel, sizeof(float));
                encoded.length += sizeof(float);
                memcpy(encoded.data + encoded.length, &oilp, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            case 0x36B: {
                float brake = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f - 101.3f;
                memcpy(encoded.data + encoded.length, &brake, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            case 0x370: {
                float speed = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f;
                memcpy(encoded.data + encoded.length, &speed, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            case 0x372: {
                float voltage = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f;
                memcpy(encoded.data + encoded.length, &voltage, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            case 0x3E9: {
                float lambda = ((received_msg.data[4] << 8) | received_msg.data[5]) / 1000.0f;
                memcpy(encoded.data + encoded.length, &lambda, sizeof(float));
                encoded.length += sizeof(float);
                break;
            }
            /*case 0x3E5: {
                encoded.data[encoded.length++] = received_msg.data[0];
                break;
            }*/
           case 0x6F3: {
                // Tyre pressures
                float front_pressure = ((received_msg.data[0] << 8) | received_msg.data[1]) / 10.0f - 101.3f;
                float rear_pressure  = ((received_msg.data[2] << 8) | received_msg.data[3]) / 10.0f - 101.3f;
                
                memcpy(encoded.data + encoded.length, &front_pressure, sizeof(float));
                encoded.length += sizeof(float);
                memcpy(encoded.data + encoded.length, &rear_pressure, sizeof(float));
                encoded.length += sizeof(float);

                // Tyre leak booleans packed in byte 4
                uint8_t leak_flags = received_msg.data[4];
                bool rr_leak = (leak_flags >> 3) & 0x01;
                bool rl_leak = (leak_flags >> 2) & 0x01;
                bool fr_leak = (leak_flags >> 1) & 0x01;
                bool fl_leak = (leak_flags >> 0) & 0x01;

                encoded.data[encoded.length++] = (uint8_t)(fl_leak);
                encoded.data[encoded.length++] = (uint8_t)(fr_leak);
                encoded.data[encoded.length++] = (uint8_t)(rl_leak);
                encoded.data[encoded.length++] = (uint8_t)(rr_leak);

                // Engine protection severity
                uint8_t severity = received_msg.data[5];
                encoded.data[encoded.length++] = severity;

                // Decode Engine Protection DTC (OBD-style)
                uint16_t raw_dtc = (received_msg.data[6] << 8) | received_msg.data[7];
                char dtc_letter;
                uint8_t prefix = (raw_dtc >> 14) & 0x03;

                switch (prefix) {
                    case 0: dtc_letter = 'P'; break;
                    case 1: dtc_letter = 'B'; break;
                    case 2: dtc_letter = 'C'; break;
                    case 3: dtc_letter = 'U'; break;
                    default: dtc_letter = '?'; break;
                }

                uint16_t dtc_number = raw_dtc & 0x3FFF;

                // Optional: Store as ASCII for debugging/serial/log
                //char dtc_code[6];
                //snprintf(dtc_code, sizeof(dtc_code), "%c%04X", dtc_letter, dtc_number);

                // Pack DTC number (raw) as 2 bytes
                encoded.data[encoded.length++] = (raw_dtc >> 8) & 0xFF;
                encoded.data[encoded.length++] = raw_dtc & 0xFF;

                // Optionally, if you want the full ASCII version (P2A00 etc.):
                // memcpy(encoded.data + encoded.length, dtc_code, 5);
                // encoded.length += 5;

                //printf("Parsed DTC: %s, Severity: %u, Tyre FL: %d, FR: %d, RL: %d, RR: %d\n",
                //    dtc_code, severity, fl_leak, fr_leak, rl_leak, rr_leak);
                break;
            }
            default:
                ESP_LOGW(TWAI_TAG, "Unhandled message ID: 0x%03X", id);
                return;
        }
    BaseType_t status;

    if (id == 0x470 || id == 0x360 || id == 0x36B) {
        status = xQueueSendToFront(xECU, &encoded, portMAX_DELAY);
    } else {
        status = xQueueSend(xECU, &encoded, portMAX_DELAY);
    }

    if (status != pdPASS) {
        ESP_LOGE(TWAI_TAG, "Failed to send message to xECU queue");
    }
    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TWAI_TAG, "CAN Receive Timeout");
        
    } else {
        ESP_LOGE(TWAI_TAG, "CAN Receive Failed: %s", esp_err_to_name(ret));
    }
}

void CAN_INIT(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 128;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    ESP_LOGI("CAN", "TWAI started at 1Mbps with software ID filtering");
}

void CAN_Task(void *pvParameters) {
    while (1) {
        receive_can_message();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}