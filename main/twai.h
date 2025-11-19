/**
 * @file twai.h
*/

#ifndef TWAI_H_
#define TWAI_H_

#include "driver/twai.h"
#include <stdint.h>
#include <stddef.h>
#include "freertos/queue.h"
//#include "driver/twai.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <inttypes.h>

#define RX_GPIO_NUM 16
#define TX_GPIO_NUM 15
#define CAN_SPEED 1000000  // 1 Mbps

#define MAX_FIELDS 2  // max number of fields per message

// Field types
#define FIELD_NONE	  0
#define FIELD_UINT8   1
#define FIELD_INT8    2
#define FIELD_UINT16  3
#define FIELD_FLOAT   4

// Encoded application message for queue transport
typedef struct {
    uint16_t id;      // CAN ID
    uint8_t  dlc;     // actual number of bytes received (0..8)
    uint8_t  data[8]; // raw CAN payload (no decoding!)
} encoded_message_t;

// Function prototypes
void CAN_Task(void *pvParameters);
void CAN_INIT(void);
void receive_can_message(void);
//size_t encode_message(uint16_t message_id, const void *fields[], uint8_t *buffer);
bool is_accepted_id(uint32_t id);

// Extern queue for encoded messages
extern QueueHandle_t xECU;

#endif // TWAI_H_