#include "driver/twai.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
extern QueueHandle_t xECU;
#include "freertos/event_groups.h"

#include "twai.h"
#include <inttypes.h> 

extern uint16_t g_rpm;
extern volatile int g_gear;
extern volatile int g_speed;
extern volatile int g_temp;
extern volatile int g_fuel;
extern volatile int g_throttle;
extern volatile int g_battery;

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
            (unsigned int)status.state,
            (unsigned int)status.rx_msg_cnt
        );    
    if (ret == ESP_OK) {
        if (received_msg.extd) {
            ESP_LOGI(TWAI_TAG, "Extended frame ID: %08X", (unsigned int)received_msg.identifier); //check for extended messages
        }
        // Filter specific message IDs
        switch (received_msg.identifier) {
            case 992:  // Coolant Temp (0x3E0)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint16_t CoolantTemp = (received_msg.data[0] << 8) | received_msg.data[1];
                float CoolantTempComp =  CoolantTemp /10;
                uint16_t OilTemp = (received_msg.data[6] << 8) | received_msg.data[7];
                float OilTempComp = OilTemp/10;
                ESP_LOGI(TWAI_TAG, "Coolant Temp: %d (Computed: %.2f)", CoolantTemp, CoolantTempComp);
                ESP_LOGI(TWAI_TAG, "Oil Temp: %d (Computed: %.2f)", OilTemp, OilTempComp);
                break;
        
            case 1136: // GearSelectorPosition (0x470)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint16_t GearSelectorPosition = received_msg.data[6];
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

                uint16_t Gear = received_msg.data[7];
                const char *gear_text;
                switch (Gear) {
                    case 0: g_gear = 0; gear_text = "N"; break;
                    case 1: g_gear = 1; gear_text = "1st"; break;
                    case 2: g_gear = 2; gear_text = "2nd"; break;
                    case 3: g_gear = 3; gear_text = "3rd"; break;
                    case 4: g_gear = 4; gear_text = "4th"; break;
                    case 5: g_gear = 5; gear_text = "5th"; break;
                    case 6: g_gear = 6; gear_text = "6th"; break;
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
                uint16_t rpm = (received_msg.data[0] << 8) | received_msg.data[1];
                g_rpm = (received_msg.data[0] << 8) | received_msg.data[1]; //globals_used
                uint16_t throttlePosition = (received_msg.data[4] << 8) | received_msg.data[5];
                g_throttle = (received_msg.data[4] << 8) | received_msg.data[5];

                // Perform computation for Throttle Position
                float throttlePositionComp = throttlePosition / 10.0;
                    
                ESP_LOGI(TWAI_TAG, "RPM: %d", rpm);
                ESP_LOGI(TWAI_TAG, "Throttle Position: %d (Computed: %.2f)", throttlePosition, throttlePositionComp);
                break;
        
            case 865:  // Fuel Pressure (0x361)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                // Extract Fuel Pressure and Oil Pressure
                uint16_t fuelPressure = (received_msg.data[0] << 8) | received_msg.data[1];
                uint16_t oilPressure = (received_msg.data[2] << 8) | received_msg.data[3];
                    
                // Perform computation for Fuel Pressure
                float fuelPressureComp = fuelPressure / 10.0 - 101.3;
                float oilPressureComp = oilPressure / 10.0 - 101.3;
                    
                ESP_LOGI(TWAI_TAG, "Fuel Pressure: %d (Computed: %.2f)", fuelPressure, fuelPressureComp);
                ESP_LOGI(TWAI_TAG, "Oil Pressure: %d (Computed: %.2f)", oilPressure, oilPressureComp);
                break;
        
            case 875:  // Brake Pressure Sensor (0x36B)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint16_t BrakePressureSensor =  (received_msg.data[0] << 8) | received_msg.data[1];
                float BrakePressureSensorComp = BrakePressureSensor - 101.3;
                ESP_LOGI(TWAI_TAG, "Brake Pressure Sensor: %d (Computed: %.2f)", BrakePressureSensor, BrakePressureSensorComp);
                break;
        
            case 880:  // Vehicle Speed (0x370)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint16_t VehicleSpeed = (received_msg.data[0] << 8) | received_msg.data[1];
                g_speed = VehicleSpeed;//globals_used
                float VehicleSpeedComp = VehicleSpeed / 10.0;
                ESP_LOGI(TWAI_TAG, "Vehicle Speed: %d (Computed: %.2f)", VehicleSpeed, VehicleSpeedComp);
                break;
        
            case 882:  // Battery Voltage (0x372)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint16_t BatteryVoltage = (received_msg.data[0] << 8) | received_msg.data[1];
                float BatteryVoltageComp = BatteryVoltage / 10.0;
                ESP_LOGI(TWAI_TAG, "Battery Voltage: %d (Computed: %.2f)", BatteryVoltage, BatteryVoltageComp);
                g_battery = BatteryVoltageComp;
                break;
        
            case 1001: // Lambda (0x3E9)
                ESP_LOGI(TWAI_TAG, "Message received: ID=%03X", (unsigned int)received_msg.identifier);
                uint16_t Lambda = (received_msg.data[4] << 8) | received_msg.data[5];
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
        if (ret == ESP_ERR_TIMEOUT) {
            // No message — expected occasionally
            ESP_LOGW(TWAI_TAG, "No message - Time out");
        } else {
            // Something is wrong, check bus status
            twai_status_info_t status;
            twai_get_status_info(&status);
            const char *some_string = "CAN_RX";           // Or whatever string you want here
            const char *esp_err_str = esp_err_to_name(ret);  // Convert esp_err_t to string
            uint32_t rx_error = status.rx_error_counter;  // Example placeholder
            uint32_t tx_error = status.tx_error_counter;  // Example placeholder

            ESP_LOGW(TWAI_TAG,
            "Receive failed: %s | ESP err: %s | RX err: %" PRIu32 " | TX err: %" PRIu32 " | State: %" PRIu32,
            some_string, esp_err_str,
            (uint32_t)rx_error,
            (uint32_t)tx_error,
            (uint32_t)status.state);
        }
    }
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