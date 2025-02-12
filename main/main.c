/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "waveshare_rgb_lcd_port.h"
#include "test.h"
#include "test.c"

void app_main()
{
    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    ESP_LOGI(TAG, "Display LVGL demos");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        // lv_demo_stress();
        // lv_demo_benchmark();
        // lv_demo_music();
        lv_demo_widgets();
        example_lvgl_demo_ui();
        // Release the mutex
        // lvgl_port_unlock();

        ESP_LOGI("Main", "First LVGL function");
        //create_canvas();

    }
}

/*
#include "driver/gpio.h"
//#include "driver/can.h" //can.h changed name to TWAI
#include "driver/twai.h"

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/twai.html

void app_main()
{

    // Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_15, GPIO_NUM_16, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    // Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }

    

}

    //Initialize configuration structures using macro initializers
    //can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_15, GPIO_NUM_16, CAN_MODE_NORMAL);
    //can_timing_config_t t_config = CAN_TIMING_CONFIG_100000KBITS(); //was 500 depend on ecu config
    //can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
    //Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    //Start CAN driver
    if (can_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }

}
*/