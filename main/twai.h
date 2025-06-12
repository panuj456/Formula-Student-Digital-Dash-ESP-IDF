/* 
	TWAI driver
*/

#ifndef TWAI_H_
#define TWAI_H_

#define RX_GPIO_NUM 16
#define TX_GPIO_NUM 15

#define CAN_SPEED 1000000  // 1Mbps

#include "freertos/queue.h"




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

extern uint16_t g_rpm;
extern volatile int g_gear;
extern volatile int g_speed;
extern volatile int g_temp;
extern volatile int g_fuel;
extern volatile int g_throttle;


/*3E0 0-1 Coolant Temp, 6-7 Oil Temp
 
470 7 Gear Position
 
360 0-1 RPM, 4-5 Throttle Position
 
361 0-1 Fuel Pressure, 2-3 Oil Pressure
 
370 0-1 Vehicle Speed
 
372 0-1 Battery Voltage
 
3E9 4-5 Lambda
 
36B 0-1 Brake pressure sensor
 
3E5 0 Ignition switch state
 */

// == function prototypes =======================================

void CAN_INIT(void);
void receive_can_message();


#endif