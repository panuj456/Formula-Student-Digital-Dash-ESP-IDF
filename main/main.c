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


#define CAN_MSG_SIZE (sizeof(can_message_t))


void app_main()
{
    xECU = xQueueCreate(1, sizeof(encoded_message_t));
    //xECU = xQueueCreate(1, CAN_MSG_SIZE); //uint8_t
    if (xECU == NULL) {
        ESP_LOGE(TAG_MAIN,"Failed to create ECU queue= %p",xECU); // Failed to create the queue.
    }
    configASSERT(xECU);
    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD - calls lv_init in lvgl_port_init
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    ESP_LOGI(TAG_MAIN, "LVGL display initialisation");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        lvgl_display_init(); //check this
        dash_create2();
<<<<<<< HEAD
        CAN_INIT();
=======
>>>>>>> parent of d89d2d7 (Working Version with TaskDelay(1))
        // Release the mutex
        lvgl_port_unlock();

    }
<<<<<<< HEAD
    //CAN_INIT(); //it was here when working if breaks - logic i believe can is initiing every loop
    
    init_lvgl_tick_timer();
    
=======
    CAN_INIT();
    //xTaskCreatePinnedToCore(CAN_INIT, "CAN Task", 4096, NULL, 5, NULL, 1); // Runs on Core 1 - comment out for testing
>>>>>>> parent of d89d2d7 (Working Version with TaskDelay(1))
    xTaskCreate(CAN_Task, "Can Task", 8192, NULL, 5, NULL);
    xTaskCreate(LVGL_Task, "LVGL Task", 8192, NULL, 5, NULL);
    xTaskCreate(Display_Task, "Dashboard Task", 8192, NULL, 5, NULL);
<<<<<<< HEAD
    /*
    //Dont use pinning unless performance drops to 50%+ CPU usage
    xTaskCreatePinnedToCore(CAN_Task, "CAN Task", 8192, NULL, 5, NULL, 1);       // Core 1
    xTaskCreatePinnedToCore(Display_Task, "Display Task", 8192, NULL, 5, NULL, 0); // Core 0
    xTaskCreatePinnedToCore(LVGL_Task, "LVGL Task", 8192, NULL, 5, NULL, 0);     // Core 0
    */

    vTaskDelay(pdMS_TO_TICKS(1));
=======
>>>>>>> parent of d89d2d7 (Working Version with TaskDelay(1))
}
