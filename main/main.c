/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "waveshare_rgb_lcd_port.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "test.h"
#include "twai.h"

static const char * TAG_MAIN = "MAIN";
QueueHandle_t xECU; //commented out for testing
extern void CAN_task(void *pvParameters);
extern void CAN_INIT();

//global shared variables - can be a struct
uint16_t g_rpm = 0;
volatile int g_gear = 0;
volatile int g_speed = 0;
volatile int g_temp = 0;
volatile int g_fuel = 0;
volatile int g_throttle = 0;
volatile int g_battery = 0;

void app_main()
{
    // Create Queue - comment out for testing
    /*xECU= xQueueCreate(1, sizeof(stats_t));
    if ( xECU == 0 ) {*/
    xECU = xQueueCreate(1, sizeof(stats_t)); //uint8_t
    if (xECU == NULL) {
        ESP_LOGE(TAG_MAIN,"Failed to create ECU queue= %p",xECU); // Failed to create the queue.
    }
        
    configASSERT(xECU);

    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    ESP_LOGI(TAG_MAIN, "LVGL display initialisation");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        dash_create2();
        // Release the mutex
        lvgl_port_unlock();

    }
    CAN_INIT();
    //xTaskCreatePinnedToCore(CAN_INIT, "CAN Task", 4096, NULL, 5, NULL, 1); // Runs on Core 1 - comment out for testing
    xTaskCreatePinnedToCore(CAN_task, "Dashboard Task", 8192, NULL, 5, NULL, 1);
}
