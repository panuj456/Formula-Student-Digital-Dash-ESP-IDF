#include "driver/twai.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
extern QueueHandle_t xECU;
#include "freertos/event_groups.h"

#include "twai.h"


static const char * TWAI_TAG = "TWAI";

void receive_can_message() { 
    twai_message_t received_msg;

    // Wait for a CAN message to be received
    esp_err_t ret = twai_receive(&received_msg, 1000 / portTICK_PERIOD_MS);
    if (ret == ESP_OK) {
        // Filter specific message IDs
        switch (received_msg.identifier) {
            case 992:  // Coolant Temp (0x3E0)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t CoolantTemp = (received_msg.data[0] << 8) | received_msg.data[1];
                float CoolantTempComp =  CoolantTemp /10;
                uint8_t OilTemp = (received_msg.data[6] << 8) | received_msg.data[7];
                float OilTempComp = OilTemp/10;
                ESP_LOGI(TWAI_TAG, "Coolant Temp: %d (Computed: %.2f)", CoolantTemp, CoolantTempComp);
                ESP_LOGI(TWAI_TAG, "Oil Temp: %d (Computed: %.2f)", OilTemp, OilTempComp);
                break;
        
            case 1136: // GearSelectorPosition (0x470)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t GearSelectorPosition = received_msg.data[6];
                if (GearSelectorPosition == 0) {
                    ESP_LOGI(TWAI_TAG, "Neutral");
                } else if (GearSelectorPosition == 1) {
                    ESP_LOGI(TWAI_TAG, "Reverse");
                } else if (GearSelectorPosition == 2) {
                    ESP_LOGI(TWAI_TAG, "Park");
                } else if (GearSelectorPosition == 3) {
                    ESP_LOGI(TWAI_TAG, "Drive");
                } else if (GearSelectorPosition == 4) {
                    ESP_LOGI(TWAI_TAG, "Sport");
                } else if (GearSelectorPosition == 5) {
                    ESP_LOGI(TWAI_TAG, "Manual");
                } else if (GearSelectorPosition == 6) {
                    ESP_LOGI(TWAI_TAG, "Low");
                } else if (GearSelectorPosition == 7) {
                    ESP_LOGI(TWAI_TAG, "Overdrive");
                } else {
                    ESP_LOGI(TWAI_TAG, "Gear Selector Position: %d", GearSelectorPosition);
                }

                uint8_t Gear = received_msg.data[7];
                const char *gear_text;
                switch (Gear) {
                    case 0: g_gear = 0; gear_text = "1st"; break;
                    case 1: g_gear = 1; gear_text = "2nd"; break;
                    case 2: g_gear = 2; gear_text = "3rd"; break;
                    case 3: g_gear = 3; gear_text = "4th"; break;
                    case 4: g_gear = 4; gear_text = "5th"; break;
                    case 5: g_gear = 5; gear_text = "6th"; break;
                    case 6: g_gear = 6; gear_text = "7th"; break;
                    default:
                        gear_text = "?";
                        g_gear = -1;
                        ESP_LOGW(TWAI_TAG, "Unknown manual gear value: %d", Gear);
                        break;
                }
                ESP_LOGI(TWAI_TAG, "Manual Gear: %s", gear_text);

                break;
                
        
            case 864:  // RPM, Throttle Position (0x360)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                // Extract RPM and Throttle Position
                uint8_t rpm = (received_msg.data[0] << 8) | received_msg.data[1];
                uint16_t g_rpm = (received_msg.data[0] << 8) | received_msg.data[1]; //globals_used
                uint8_t throttlePosition = (received_msg.data[4] << 8) | received_msg.data[5];
                uint8_t g_throttle = (received_msg.data[4] << 8) | received_msg.data[5];

                // Perform computation for Throttle Position
                float throttlePositionComp = throttlePosition / 10.0;
                    
                ESP_LOGI(TWAI_TAG, "RPM: %d", rpm);
                ESP_LOGI(TWAI_TAG, "Throttle Position: %d (Computed: %.2f)", throttlePosition, throttlePositionComp);
                break;
        
            case 865:  // Fuel Pressure (0x361)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                // Extract Fuel Pressure and Oil Pressure
                uint8_t fuelPressure = (received_msg.data[0] << 8) | received_msg.data[1];
                uint8_t oilPressure = (received_msg.data[2] << 8) | received_msg.data[3];
                    
                // Perform computation for Fuel Pressure
                float fuelPressureComp = fuelPressure / 10.0 - 101.3;
                float oilPressureComp = oilPressure / 10.0 - 101.3;
                    
                ESP_LOGI(TWAI_TAG, "Fuel Pressure: %d (Computed: %.2f)", fuelPressure, fuelPressureComp);
                ESP_LOGI(TWAI_TAG, "Oil Pressure: %d (Computed: %.2f)", oilPressure, oilPressureComp);
                break;
        
            case 875:  // Brake Pressure Sensor (0x36B)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t BrakePressureSensor =  (received_msg.data[0] << 8) | received_msg.data[1];
                float BrakePressureSensorComp = BrakePressureSensor - 101.3;
                ESP_LOGI(TWAI_TAG, "Brake Pressure Sensor: %d (Computed: %.2f)", BrakePressureSensor, BrakePressureSensorComp);
                break;
        
            case 880:  // Vehicle Speed (0x370)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t VehicleSpeed = (received_msg.data[0] << 8) | received_msg.data[1];
                uint8_t g_speed = (received_msg.data[0] << 8) | received_msg.data[1];//globals_used
                float VehicleSpeedComp = VehicleSpeed / 10.0;
                ESP_LOGI(TWAI_TAG, "Vehicle Speed: %d (Computed: %.2f)", VehicleSpeed, VehicleSpeedComp);
                break;
        
            case 882:  // Battery Voltage (0x372)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t BatteryVoltage = (received_msg.data[0] << 8) | received_msg.data[1];
                float BatteryVoltageComp = BatteryVoltage / 10.0;
                ESP_LOGI(TWAI_TAG, "Battery Voltage: %d (Computed: %.2f)", BatteryVoltage, BatteryVoltageComp);
                break;
        
            case 1001: // Lambda (0x3E9)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t Lambda = (received_msg.data[4] << 8) | received_msg.data[5];
                float LambdaComp = Lambda / 1000.0;
                ESP_LOGI(TWAI_TAG, "Lambda: %d (Computed: %.2f)", Lambda, LambdaComp);
                break;
        
            case 997:  // Ignition Switch State (0x3E5)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint8_t ignitionState = received_msg.data[0];
                if (ignitionState == 0) {
                    ESP_LOGI(TWAI_TAG, "Ignition Switch State: OFF");
                } else if (ignitionState == 1) {
                    ESP_LOGI(TWAI_TAG, "Ignition Switch State: ON");
                } else {
                    ESP_LOGI(TWAI_TAG, "Ignition Switch State: Unknown");
                }

                if (xQueueSend(xECU, &ignitionState , portMAX_DELAY ) != pdPASS) {
                    ESP_LOGE(TWAI_TAG, "xQueueSend Fail");
                }
                break;
        
            default:
                // Ignore other messages
                break;
        } // End of switch
    } else {
        ESP_LOGE(TWAI_TAG, "Failed to receive message!");
    } // End of if ret == ESP_OK
} // End of receive_can_message function


void CAN_INIT(void *arg) {
    // Configure the TWAI driver for 1Mbps and 11-bit IDs
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

    // Configure timing for 1MBit speed
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();

    // Set filter to 11-bit CAN IDs
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all IDs for now

    // Install the TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));

    // Start the TWAI driver
    ESP_ERROR_CHECK(twai_start());

    ESP_LOGI(TWAI_TAG, "CAN driver installed and started.");

    while (1) {
        receive_can_message();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Avoids watchdog timeout
    }
    
}
