#include "driver/twai.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#define RX_GPIO_NUM 16
#define TX_GPIO_NUM 15

#define CAN_SPEED 1000000  // 1Mbps


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

static const char * TAG = "TWAI";

void receive_can_message() { 
    twai_message_t received_msg;

    // Wait for a CAN message to be received
    esp_err_t ret = twai_receive(&received_msg, 1000 / portTICK_PERIOD_MS);
    if (ret == ESP_OK) {
        // Filter specific message IDs
        switch (received_msg.identifier) {
            case 992:  // Coolant Temp (0x3E0)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int CoolantTemp = (received_msg.data[0] << 8) | received_msg.data[1];
                float CoolantTempComp =  CoolantTemp /10;
                int OilTemp = (received_msg.data[6] << 8) | received_msg.data[7];
                float OilTempComp = OilTemp/10;
                ESP_LOGI(TAG, "Coolant Temp: %d (Computed: %.2f)", CoolantTemp, CoolantTempComp);
                ESP_LOGI(TAG, "Oil Temp: %d (Computed: %.2f)", OilTemp, OilTempComp);
                break;
        
            case 1136: // GearSelectorPosition (0x470)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int GearSelectorPosition = received_msg.data[7];
                if (GearSelectorPosition == 0) {
                    ESP_LOGI(TAG, "Neutral");
                } else if (GearSelectorPosition == 1) {
                    ESP_LOGI(TAG, "Reverse");
                } else if (GearSelectorPosition == 2) {
                    ESP_LOGI(TAG, "Park");
                } else if (GearSelectorPosition == 3) {
                    ESP_LOGI(TAG, "Drive");
                } else if (GearSelectorPosition == 4) {
                    ESP_LOGI(TAG, "Sport");
                } else if (GearSelectorPosition == 5) {
                    ESP_LOGI(TAG, "Manual");
                } else if (GearSelectorPosition == 6) {
                    ESP_LOGI(TAG, "Low");
                } else if (GearSelectorPosition == 7) {
                    ESP_LOGI(TAG, "Overdrive");
                } else {
                    ESP_LOGI(TAG, "Gear Selector Position: %d", GearSelectorPosition);
                }
                break;
        
            case 864:  // RPM, Throttle Position (0x360)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                // Extract RPM and Throttle Position
                int rpm = (received_msg.data[0] << 8) | received_msg.data[1];
                int throttlePosition = (received_msg.data[4] << 8) | received_msg.data[5];
                    
                // Perform computation for Throttle Position
                float throttlePositionComp = throttlePosition / 10.0;
                    
                ESP_LOGI(TAG, "RPM: %d", rpm);
                ESP_LOGI(TAG, "Throttle Position: %d (Computed: %.2f)", throttlePosition, throttlePositionComp);
                break;
        
            case 865:  // Fuel Pressure (0x361)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                // Extract Fuel Pressure and Oil Pressure
                int fuelPressure = (received_msg.data[0] << 8) | received_msg.data[1];
                int oilPressure = (received_msg.data[2] << 8) | received_msg.data[3];
                    
                // Perform computation for Fuel Pressure
                float fuelPressureComp = fuelPressure / 10.0 - 101.3;
                float oilPressureComp = oilPressure / 10.0 - 101.3;
                    
                ESP_LOGI(TAG, "Fuel Pressure: %d (Computed: %.2f)", fuelPressure, fuelPressureComp);
                ESP_LOGI(TAG, "Oil Pressure: %d (Computed: %.2f)", oilPressure, oilPressureComp);
                break;
        
            case 875:  // Brake Pressure Sensor (0x36B)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int BrakePressureSensor =  (received_msg.data[0] << 8) | received_msg.data[1];
                float BrakePressureSensorComp = BrakePressureSensor - 101.3;
                ESP_LOGI(TAG, "Brake Pressure Sensor: %d (Computed: %.2f)", BrakePressureSensor, BrakePressureSensorComp);
                break;
        
            case 880:  // Vehicle Speed (0x370)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int VehicleSpeed = (received_msg.data[0] << 8) | received_msg.data[1];
                float VehicleSpeedComp = VehicleSpeed / 10.0;
                ESP_LOGI(TAG, "Vehicle Speed: %d (Computed: %.2f)", VehicleSpeed, VehicleSpeedComp);
                break;
        
            case 882:  // Battery Voltage (0x372)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int BatteryVoltage = (received_msg.data[0] << 8) | received_msg.data[1];
                float BatteryVoltageComp = BatteryVoltage / 10.0;
                ESP_LOGI(TAG, "Battery Voltage: %d (Computed: %.2f)", BatteryVoltage, BatteryVoltageComp);
                break;
        
            case 1001: // Lambda (0x3E9)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int Lambda = (received_msg.data[4] << 8) | received_msg.data[5];
                float LambdaComp = Lambda / 1000.0;
                ESP_LOGI(TAG, "Lambda: %d (Computed: %.2f)", Lambda, LambdaComp);
                break;
        
            case 997:  // Ignition Switch State (0x3E5)
                ESP_LOGI(TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                int ignitionState = received_msg.data[0];
                if (ignitionState == 0) {
                    ESP_LOGI(TAG, "Ignition Switch State: OFF");
                } else if (ignitionState == 1) {
                    ESP_LOGI(TAG, "Ignition Switch State: ON");
                } else {
                    ESP_LOGI(TAG, "Ignition Switch State: Unknown");
                }
                break;
        
            default:
                // Ignore other messages
                break;
        } // End of switch
    } else {
        ESP_LOGE(TAG, "Failed to receive message!");
    } // End of if ret == ESP_OK
} // End of receive_can_message function


void can_task(void *arg) {
    while (1) {
        receive_can_message();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Avoids watchdog timeout
    }
}

void app_main(void) {
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

    ESP_LOGI(TAG, "CAN driver installed and started.");

    xTaskCreatePinnedToCore(can_task, "CAN Task", 4096, NULL, 5, NULL, 1); // Runs on Core 1
    
}
