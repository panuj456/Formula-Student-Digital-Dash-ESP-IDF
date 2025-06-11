#include "lvgl.h"


// Declare the fonts (this makes sure linker knows about them)
LV_FONT_DECLARE(lv_font_montserrat_36);
LV_FONT_DECLARE(lv_font_montserrat_48);


#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

static lv_obj_t *rpm_arc;
static lv_obj_t *rpm_label;
static lv_obj_t *gear_label;
static lv_obj_t *speed_label;
static lv_obj_t *fuel_bar;
static lv_obj_t *temp_bar;
static lv_obj_t *canvas;

// Make sure you’ve got these fonts enabled in your lv_conf.h
LV_FONT_DECLARE(lv_font_montserrat_48);
LV_FONT_DECLARE(lv_font_montserrat_36);
LV_FONT_DECLARE(lv_font_montserrat_16);

void dash_create2(void)
{
    // Main container with horizontal scroll
    lv_obj_t *main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_scroll_dir(main_container, LV_DIR_HOR);
    lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_style_pad_all(main_container, 0, 0);

    // Page 1: primary data
    lv_obj_t *primary_data = lv_obj_create(main_container);
    lv_obj_set_size(primary_data, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(primary_data, LV_ALIGN_LEFT_MID, 0, 0);

    // Add primary meters here...

    // --- RPM Arc Meter ---
    lv_obj_t *rpm_arc = lv_arc_create(primary_data);
    lv_obj_set_size(rpm_arc, 150, 150);
    lv_obj_center(rpm_arc);
    lv_arc_set_range(rpm_arc, 0, 8000);
    lv_arc_set_value(rpm_arc, 0);
    lv_obj_set_style_arc_color(rpm_arc, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_arc_width(rpm_arc, 10, 0);
    lv_arc_set_bg_angles(rpm_arc, 135, 45); // For a gauge style arc

    
    // --- Gear Label ---
    lv_obj_t *gear_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(gear_label, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_label_set_text(gear_label, "N");
    lv_obj_align(gear_label, LV_ALIGN_CENTER, 0, 80);

    // --- Speed Label ---
    lv_obj_t *speed_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(speed_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(speed_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(speed_label, "0 KM/H");
    lv_obj_align(speed_label, LV_ALIGN_BOTTOM_MID, 0, -20);


    // Page 2: secondary data
    lv_obj_t *secondary_data = lv_obj_create(main_container);
    lv_obj_set_size(secondary_data, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align_to(secondary_data, primary_data, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

    // Fuel Temp Label
    lv_obj_t *fuel_temp_label = lv_label_create(secondary_data);
    lv_obj_set_style_text_font(fuel_temp_label, &lv_font_montserrat_24, 0);
    lv_label_set_text(fuel_temp_label, "Fuel Temp:\n80 °C");
    lv_obj_align(fuel_temp_label, LV_ALIGN_TOP_MID, 0, 20);

    // Oil Pressure Label
    lv_obj_t *oil_pressure_label = lv_label_create(secondary_data);
    lv_obj_set_style_text_font(oil_pressure_label, &lv_font_montserrat_24, 0);
    lv_label_set_text(oil_pressure_label, "Oil Pressure:\n3.2 Bar");
    lv_obj_align(oil_pressure_label, LV_ALIGN_TOP_MID, 0, 80);
}

// Example function called periodically:
void update_dashboard(int rpm, int gear, int speed, int fuel, int temp) {
    // RPM
    lv_arc_set_value(rpm_arc, rpm);
    lv_label_set_text_fmt(rpm_label, "%d", rpm);

    // Gear
    lv_label_set_text_fmt(gear_label, "%d", gear);

    // Speed
    lv_label_set_text_fmt(speed_label, "%d", speed);

    // Fuel % (0–100)
    lv_bar_set_value(fuel_bar, fuel, LV_ANIM_ON);

    // Coolant Temp (0–120°C)
    lv_bar_set_value(temp_bar, temp, LV_ANIM_ON);
}

void create_canvas()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_scr_load(scr);

    canvas = lv_canvas_create(scr);
    lv_obj_set_size(canvas, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);

    lv_color_t *buf = (lv_color_t *)lv_mem_alloc(LV_HOR_RES * LV_VER_RES * sizeof(lv_color_t));
    lv_canvas_set_buffer(canvas, buf, LV_HOR_RES, LV_VER_RES, LV_IMG_CF_TRUE_COLOR);

    // Set a background color for the canvas
    lv_canvas_fill_bg(canvas, lv_color_hex(0x078080), LV_OPA_COVER);
    
}