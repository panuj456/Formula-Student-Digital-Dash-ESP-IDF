#ifndef TWAI_H_
#define TWAI_H_

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

typedef struct {
    uint16_t id;
    uint8_t data[8];  // raw CAN frame data
    uint8_t length;   // data length
} can_message_t;

// Encoded application message for queue transport
typedef struct {
    uint8_t data[32];  // buffer size for encoded messages
    size_t length;
} encoded_message_t;

// Define the message definition struct
typedef struct {
    uint16_t message_id;
    size_t field_count;
    uint8_t fields[MAX_FIELDS];  // array of field types
} message_def_t;

// Message definitions array
static const message_def_t message_defs[] = {
    { 0x3E0, 2, { FIELD_FLOAT, FIELD_FLOAT } },        // Coolant Temp, Oil Temp
    { 0x470, 1, { FIELD_UINT8, FIELD_NONE } },         // Gear Position
    { 0x360, 2, { FIELD_UINT16, FIELD_UINT8 } },       // RPM, Throttle Position
    { 0x361, 2, { FIELD_FLOAT, FIELD_FLOAT } },        // Fuel Pressure, Oil Pressure
    { 0x370, 1, { FIELD_FLOAT, FIELD_NONE } },         // Vehicle Speed
    { 0x372, 1, { FIELD_FLOAT, FIELD_NONE } },         // Battery Voltage
    { 0x3E9, 1, { FIELD_FLOAT, FIELD_NONE } },         // Lambda
    { 0x36B, 1, { FIELD_FLOAT, FIELD_NONE } },         // Brake Pressure Sensor
    { 0x3E5, 1, { FIELD_UINT8, FIELD_NONE } }          // Ignition Switch State
};

static const size_t message_defs_count = sizeof(message_defs) / sizeof(message_defs[0]);

// Dashboard stats struct
typedef struct {
    float Coolant_Temp;
    float Oil_Temp;
    uint8_t Gear_Position;
    uint16_t RPM;
    float Throttle_Position;
    float Fuel_Pressure;
    float Oil_Pressure;
    float Vehicle_Speed;
    float Battery_Voltage;
    float Lambda;
    float Brake_Pressure_Sensor;
} stats_t;

// Function prototypes
void CAN_Task(void *pvParameters);
void CAN_INIT(void);
void receive_can_message(void);
//size_t encode_message(uint16_t message_id, const void *fields[], uint8_t *buffer);
bool is_accepted_id(uint32_t id);

// Extern queue for encoded messages
extern QueueHandle_t xECU;

#endif // TWAI_H_