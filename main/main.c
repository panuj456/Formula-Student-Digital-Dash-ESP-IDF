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
QueueHandle_t xECU; 

void app_main()
{
    // Create Queue
    xECU= xQueueCreate(1, sizeof(stats_t));
    if ( xECU == 0 ) {
        ESP_LOGE(TAG_MAIN,"Failed to create ECU queue= %p",xECU); // Failed to create the queue.
    }
    configASSERT(xECU);

    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    ESP_LOGI(TAG_MAIN, "Display LVGL demos");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        // lv_demo_stress();
        // lv_demo_benchmark();
        // lv_demo_music();
        //lv_demo_widgets();

        //example_lvgl_demo_ui();
        // Release the mutex
        lvgl_port_unlock();

        ESP_LOGI(TAG_MAIN, "First LVGL function");
        //create_canvas();
        dash_create2();

    }

    xTaskCreatePinnedToCore(CAN_INIT, "CAN Task", 4096, NULL, 5, NULL, 1); // Runs on Core 1
}
