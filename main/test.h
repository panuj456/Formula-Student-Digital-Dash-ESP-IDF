#ifndef LVGL_DEMO_H
#define LVGL_DEMO_H

#include "lvgl.h"
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
#define NUM_SHIFT_LEDS 10
#define SHIFT_LED_WIDTH 20
#define SHIFT_LED_HEIGHT 10
#define SHIFT_LED_GAP 4
#define RPM_MIN 3000
#define RPM_MAX 11200

// Example thresholds (adjust to your engine)
static const uint16_t rpm_thresholds[NUM_SHIFT_LEDS] = {
    2000, 3000, 4000, 5000, 6000, 7000, 7500, 8000, 8500, 9000
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
void Display_Task(void *pvParameters);
float swap_float_bytes(const uint8_t *data);
void shift_light_update(uint16_t rpm);
lv_obj_t* shift_light_create(lv_obj_t *parent);
//size_t decode_message(const uint8_t *buffer, void *fields_out[]);


#endif /* LVGL_DEMO_H */