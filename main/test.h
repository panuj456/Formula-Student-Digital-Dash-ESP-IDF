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
size_t decode_message(const uint8_t *buffer, void *fields_out[]);
void decode_and_dispatch();


#endif /* LVGL_DEMO_H */