#ifndef TEST_H
#define TEST_H

#include "lvgl.h"
<<<<<<< HEAD
<<<<<<< HEAD
#include "esp_timer.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"

//#define portMAX_DELAY ((TickType_t)0xffffffffUL)

//#include "lv_meter.h"
//#include "lv_widgets.h"


#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

#define FIELD_UINT8   1
#define FIELD_INT8    2
#define FIELD_UINT16  3
#define FIELD_FLOAT   4
#define MAX_FIELDS    2  // Or higher if needed

// Display resolution
#define LVGL_PORT_H_RES 800
#define LVGL_PORT_V_RES 480

// LCD RGB Panel GPIOs
#define EXAMPLE_LCD_IO_RGB_VSYNC   (GPIO_NUM_3)
#define EXAMPLE_LCD_IO_RGB_HSYNC   (GPIO_NUM_46)
#define EXAMPLE_LCD_IO_RGB_DE      (GPIO_NUM_5)
#define EXAMPLE_LCD_IO_RGB_PCLK    (GPIO_NUM_7)
#define EXAMPLE_LCD_IO_RGB_DISP    (-1)

#define EXAMPLE_LCD_IO_RGB_DATA0   (GPIO_NUM_14)
#define EXAMPLE_LCD_IO_RGB_DATA1   (GPIO_NUM_38)
// ... Define up to DATA15 as needed

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (16 * 1000 * 1000)
#define EXAMPLE_RGB_BIT_PER_PIXEL      16
#define EXAMPLE_RGB_DATA_WIDTH         16

//shifting rpm rev limitter
#define NUM_SHIFT_LEDS 16
#define SHIFT_LED_WIDTH 20
#define SHIFT_LED_HEIGHT 20
#define SHIFT_LED_GAP 15
#define RPM_MIN 1000
#define RPM_MAX 11200


// Example thresholds (adjust to your engine)
static const uint16_t rpm_thresholds[NUM_SHIFT_LEDS] = {
    2000, 8000, 8200, 8400, 8600, 8800, 9000, 9200, 9400, 9600, 9800, 10000, 10200, 10400, 10600, 10800
};

static lv_obj_t *rpm_arc;
static lv_obj_t *rpm_label;
static lv_obj_t *gear_label;
static lv_obj_t *speed_label;
static lv_obj_t *fuel_bar;
static lv_obj_t *temp_bar;
static lv_obj_t *canvas;
static lv_obj_t *canvas;
static lv_obj_t * obj;

// Define buffers - lvgl driver
#define BUF_LINES 60  // Adjust based on your available RAM
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t buf1[800 * 60]; // Enough for 60 lines of 800px
static lv_color_t buf2[800 * 60];
extern esp_lcd_panel_handle_t my_lcd_panel_handle;


void dash_create2(void);
void Display_Task(void *pvParameters);
float swap_float_bytes(const uint8_t *data);
void shift_light_update(uint16_t rpm);
lv_obj_t* shift_light_create(lv_obj_t *parent);
void LVGL_Task(void *pvParameters);
void lv_tick_task(void* arg);
void init_lvgl_tick_timer(void);
void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void decode_and_dispatch(const encoded_message_t *encoded_msg);
void lvgl_display_init(void *pvParameters) 

//size_t decode_message(const uint8_t *buffer, void *fields_out[]);



#endif /* TEST_H */
=======

