#ifndef LVGL_DEMO_H
#define LVGL_DEMO_H

#include "lvgl.h"
//#include "lv_meter.h"
//#include "lv_widgets.h"
#include "lvgl.h"
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

static lv_obj_t *rpm_arc;
static lv_obj_t *rpm_label;
static lv_obj_t *gear_label;
static lv_obj_t *speed_label;
static lv_obj_t *fuel_bar;
static lv_obj_t *temp_bar;
static lv_obj_t *canvas;
static lv_obj_t *canvas;
static lv_obj_t * obj;

extern uint16_t g_rpm;
extern volatile int g_gear;
extern volatile int g_speed;
extern volatile int g_temp;
extern volatile int g_fuel;
extern volatile int g_throttle;


void dash_create2();


#endif /* LVGL_DEMO_H */