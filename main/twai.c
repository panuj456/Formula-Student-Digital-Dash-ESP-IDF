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
    esp_err_t ret = twai_receive(&received_msg, 5 / portTICK_PERIOD_MS);
    //esp_err_t ret = twai_receive(&received_msg, pdMS_TO_TICKS(5));


    twai_status_info_t status;
    twai_get_status_info(&status);

    /*
    ESP_LOGI(TWAI_TAG, "Time: %lu, TX ERR: %u, RX ERR: %u, State: %u",
             (unsigned long)esp_timer_get_time(),
             (unsigned int)status.tx_error_counter,
             (unsigned int)status.rx_error_counter,
             (unsigned int)status.state);
    */

    if (ret == ESP_OK) {
        uint16_t id = received_msg.identifier;
        if (!is_accepted_id(id)) return;

        /* //Checking tool
        printf("Time: %lu - Received CAN ID: 0x%lX, DLC: %d, Data: ",
               (unsigned long)esp_timer_get_time(), received_msg.identifier, received_msg.data_length_code);
        for (int i = 0; i < received_msg.data_length_code; i++) {
            printf("%02X ", received_msg.data[i]);
        }
        printf("\n"); */

        encoded_message_t encoded = { .length = 0 };

        //encoded_message_t encoded;
        encoded.id = id;
        encoded.dlc = received_msg.data_length_code;
        // copy only the bytes reported by DLC
        if (encoded.dlc > 8) encoded.dlc = 8;
        memcpy(encoded.data, received_msg.data, encoded.dlc);


        switch (id) {
            case 0x470: {
                encoded.data[encoded.length++] = received_msg.data[7];
                break;
            }
            case 0x360: {
                // Just copy the raw CAN payload (6 bytes)
                memcpy(encoded.data + encoded.length, received_msg.data, 6);
                encoded.length += 6;
                break;
            }
            case 0x3E0: {
                memcpy(encoded.data + encoded.length, received_msg.data, 8);
                encoded.length += 8;
                break;
            }
            case 0x361: {
                memcpy(encoded.data + encoded.length, received_msg.data, 8);
                encoded.length += 8;
                break;
            }

            case 0x36B: {
                // Copy raw 2 bytes (brake sensor)
                memcpy(encoded.data + encoded.length, received_msg.data, 2);
                encoded.length += 2;
                break;
            }
            case 0x370: {
                // Copy raw 2 bytes
                memcpy(encoded.data + encoded.length, received_msg.data, 2);
                encoded.length += 2;
                break;
            }

            case 0x372: {
                memcpy(encoded.data + encoded.length, received_msg.data, 2);
                encoded.length += 2;
                break;
            }
            case 0x3E9: {
                encoded.data[encoded.length++] = received_msg.data[4];
                encoded.data[encoded.length++] = received_msg.data[5];
                break;
            }

            case 0x363: {
                // 4 bytes: slip_H, slip_L, diff_H, diff_L
                memcpy(encoded.data + encoded.length, received_msg.data, 4);
                encoded.length += 4;
                break;
            }

            case 0x36C: {
                memcpy(encoded.data + encoded.length, received_msg.data, 8);
                encoded.length += 8;
                break;
            }
            /*case 0x3E5: { //ignition switch
                encoded.data[encoded.length++] = received_msg.data[0];
                break;
            }*/
           case 0x6F3: {
                // Severity – 1 byte at payload index 5
                encoded.data[encoded.length++] = received_msg.data[5];

                // Raw DTC – 2 bytes at payload indices [6:7]
                encoded.data[encoded.length++] = received_msg.data[6];
                encoded.data[encoded.length++] = received_msg.data[7];

                break;
            }
            default:
                ESP_LOGW(TWAI_TAG, "Unhandled message ID: 0x%03X", id);
                return;
        }
    BaseType_t status;
    // Prioritise critical messages (gear, rpm/throttle, brake)
    if (id == 0x470 || id == 0x360 || id == 0x36B) {
        status = xQueueSendToFront(xECU, &encoded, 0); // non-blocking
    } else {
        status = xQueueSend(xECU, &encoded, 0);
    }

    if (status != pdPASS) {
        // queue full, consider incrementing a counter for diagnostics; avoid heavy logging
    }
}

void CAN_INIT(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 128;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS(); //reading haltech CAN protocol
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    ESP_LOGI("CAN", "TWAI started at 1Mbps with software ID filtering");
}

void CAN_Task(void *pvParameters) {
    encoded_message_t msg;
    while (1) {
        receive_can_message();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}