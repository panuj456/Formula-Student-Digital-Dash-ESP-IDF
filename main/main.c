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
extern void CAN_task(void *pvParameters);
extern void CAN_INIT();
extern void init_lvgl_tick_timer();
extern void LVGL_Task(void *pvParameters);
extern void Display_Task(void *pvParameters);


#define CAN_MSG_SIZE (sizeof(encoded_message_t))


void app_main()
{
    xECU = xQueueCreate(40, sizeof(encoded_message_t));
    if (xECU == NULL) {
        ESP_LOGE(TAG_MAIN,"Failed to create ECU queue= %p",xECU); // Failed to create the queue.
    }
        
    configASSERT(xECU);

    waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD and lv_init
    //wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    //wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 
    
    ESP_LOGI(TAG_MAIN, "LVGL display initialisation");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        dash_create2();
        // Release the mutex
        lvgl_port_unlock();
    }
    CAN_INIT();
    init_lvgl_tick_timer();

    /*
    xTaskCreate(CAN_Task, "Can Task", 8192, NULL, 5, NULL);
    xTaskCreate(Display_Task, "Dashboard Task", 8192, NULL, 5, NULL);
    xTaskCreate(LVGL_Task, "LVGL Task", 8192, NULL, 5, NULL);
    */

    
    xTaskCreatePinnedToCore(CAN_Task, "CAN Task", 32768, NULL, 5, NULL, 0);      // Core 0: Real-time CAN
    xTaskCreatePinnedToCore(Display_Task, "Dashboard Task", 32768, NULL, 4, NULL, 1); // Core 0: CAN Decode/UI prep
    xTaskCreatePinnedToCore(LVGL_Task, "LVGL Task", 32768, NULL, 3, NULL, 1);        // Core 1: LVGL Handler
    
    
    //vTaskDelay(pdMS_TO_TICKS(1));
}