#ifndef TWAI_H_
#define TWAI_H_

#include <stdint.h>
#include <stddef.h>
#include "freertos/queue.h"

#define RX_GPIO_NUM 16
#define TX_GPIO_NUM 15
#define CAN_SPEED 1000000  // 1 Mbps

#define MAX_FIELDS 2  // max number of fields per message

// Field types
#define FIELD_UINT8   1
#define FIELD_INT8    2
#define FIELD_UINT16  3
#define FIELD_FLOAT   4

typedef struct {
    uint16_t id;
    uint8_t data[8];  // raw CAN frame data
    uint8_t length;   // data length
} can_message_t;

// Define the message definition struct
typedef struct {
    uint16_t message_id;
    size_t field_count;
    uint8_t fields[MAX_FIELDS];  // array of field types
} message_def_t;

// Message definitions array
static const message_def_t message_defs[] = {
    { 0x3E0, 2, { FIELD_FLOAT, FIELD_FLOAT } },        // Coolant Temp, Oil Temp
    { 0x470, 1, { FIELD_UINT8, 0 } },                  // Gear Position
    { 0x360, 2, { FIELD_UINT16, FIELD_UINT8 } },       // RPM, Throttle Position
    { 0x361, 2, { FIELD_FLOAT, FIELD_FLOAT } },        // Fuel Pressure, Oil Pressure
    { 0x370, 1, { FIELD_FLOAT, 0 } },                  // Vehicle Speed
    { 0x372, 1, { FIELD_FLOAT, 0 } },                  // Battery Voltage
    { 0x3E9, 1, { FIELD_FLOAT, 0 } },                  // Lambda
    { 0x36B, 1, { FIELD_FLOAT, 0 } },                  // Brake Pressure Sensor
    { 0x3E5, 1, { FIELD_UINT8, 0 } }                   // Ignition Switch State
};

static const size_t message_defs_count = sizeof(message_defs) / sizeof(message_defs[0]);

// Dashboard stats struct
typedef struct {
    uint8_t Coolant_Temp;
    uint8_t Oil_Temp;
    uint8_t Gear_Position;
    uint8_t RPM;
    uint8_t Throttle_Position;
    uint8_t Fuel_Pressure;
    uint8_t Oil_Pressure;
    uint8_t Vehicle_Speed;
    uint8_t Battery_Voltage;
    uint8_t Lambda;
    uint8_t Brake_Pressure_Sensor;
    uint8_t Ignition_Switch_State;
} stats_t;

// Function prototypes
void CAN_Task(void *pvParameters);
void CAN_INIT(void);
void receive_can_message();
size_t encode_message(uint16_t message_id, const void *fields[], uint8_t *buffer);

#endif // TWAI_H_
