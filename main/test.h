/**
 * @file test.h
*/
#ifndef LVGL_DEMO_H
#define LVGL_DEMO_H

#include "lvgl.h"
#include "esp_timer.h"
//#include "lv_meter.h"
//#include "lv_widgets.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

#define FIELD_UINT8   1
#define FIELD_INT8    2
#define FIELD_UINT16  3
#define FIELD_FLOAT   4
#define MAX_FIELDS    2  // Or higher if needed

//shifting rpm rev limitter
#define NUM_SHIFT_LEDS 12
#define SHIFT_LED_WIDTH 25
#define SHIFT_LED_HEIGHT 25
#define SHIFT_LED_GAP 15
#define RPM_MIN 1000
#define RPM_MAX 11200

typedef struct {
    uint16_t threshold;
    uint32_t color_rgb; // Use uint32_t for RGB hex values (0xRRGGBB)
} RpmThresholdColor;

#define NUM_SHIFT_LEDS (sizeof(shift_map) / sizeof(RpmThresholdColor))

static const RpmThresholdColor shift_map[] = {
    // Threshold, Color (0xRRGGBB format)
    {2000,  0x00FF00}, // Green
    {8000,  0x00FF00},
    {8280,  0x00FF00},
    {8560,  0x00FF00},
    {8840,  0xFFFF00}, // Yellow
    {9120,  0xFFFF00},
    {9400,  0xFFFF00},
    {9680,  0xFF0000}, // Red
    {9960,  0xFF0000},
    {10240, 0xFF0000},
    {10520, 0x800080}, // Purple/Magenta
    {10800, 0x800080}
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


void dash_create2(void);
/**
 * Initialises Dash visual graphs and labels
 * @note    Enables updating of each label
*/
void Display_Task(void *pvParameters);
/**
*@param pvParameters
*@note From wavesahre lcd
*/
float swap_float_bytes(const uint8_t *data);
void shift_light_update(uint16_t rpm);
lv_obj_t* shift_light_create(lv_obj_t *parent);
void LVGL_Task(void *pvParameters);
void lv_tick_task(void* arg);
void init_lvgl_tick_timer(void);
void Decode_Task(void *pvParameters);


//size_t decode_message(const uint8_t *buffer, void *fields_out[]);



#endif /* LVGL_DEMO_H */