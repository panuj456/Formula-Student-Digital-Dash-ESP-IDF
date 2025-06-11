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
extern lv_obj_t *canvas;
extern lv_obj_t * obj;

void dash_create2();
void create_canvas();


#endif /* LVGL_DEMO_H */