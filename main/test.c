#include "test.h"
#include "lvgl.h"                     // For LVGL functions
#include "lvgl_port.h"               // For lvgl_port_lock/unlock (project specific)
#include "freertos/FreeRTOS.h"       // For vTaskDelay and pdMS_TO_TICKS
#include "freertos/task.h"
#include "twai.h"



// Declare the fonts (this makes sure linker knows about them)
LV_FONT_DECLARE(lv_font_montserrat_36);
LV_FONT_DECLARE(lv_font_montserrat_48);


#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

static lv_obj_t *rpm_bar;
static lv_obj_t *rpm_label;
static lv_obj_t *gear_label;
static lv_obj_t *speed_label;
static lv_obj_t *fuel_bar;
static lv_obj_t *temp_bar;
static lv_obj_t *canvas;
static lv_obj_t *rpm_value;
static lv_obj_t *throttle_bar;
static lv_obj_t *throttle_label;
static lv_obj_t *battery_label;

extern uint16_t g_rpm;
extern volatile int g_gear;
extern volatile int g_speed;
extern volatile int g_temp;
extern volatile int g_fuel;
extern volatile int g_throttle;
extern volatile int g_battery;

void update_dashboard(void);
void dash_create2(void);
void CAN_task(void *pvParameters);

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
    /*
    lv_obj_t *rpm_arc = lv_arc_create(primary_data);
    lv_obj_clear_flag(rpm_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_style(rpm_arc, NULL, LV_PART_KNOB);
    lv_obj_set_size(rpm_arc, 150, 150);
    lv_obj_center(rpm_arc);
    lv_arc_set_range(rpm_arc, 0, 8000);
    lv_arc_set_value(rpm_arc, 0);
    lv_obj_set_style_arc_color(rpm_arc, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_arc_width(rpm_arc, 20, 0);
    lv_arc_set_bg_angles(rpm_arc, 135, 45); // For a gauge style arc
    */

    // Create a style for the throttle bar (optional)
    static lv_style_t throttle_style;
    lv_style_init(&throttle_style);
    lv_style_set_bg_color(&throttle_style, lv_color_hex(0x00FF00));  // Green fill
    lv_style_set_radius(&throttle_style, 5);

    // Create the bar
    throttle_bar = lv_bar_create(primary_data);
    lv_obj_add_style(throttle_bar, &throttle_style, LV_PART_INDICATOR);
    lv_obj_set_size(throttle_bar, 30, 300);  // Width, Height
    lv_obj_set_style_bg_color(throttle_bar, lv_color_black(), 0);
    lv_obj_set_style_border_width(throttle_bar, 10, 0);
    lv_obj_align(throttle_bar, LV_ALIGN_LEFT_MID, 10, 0);  // Left center of screen with 10px margin
    lv_bar_set_range(throttle_bar, 0, 100);  // Throttle % from 0 to 100
    lv_bar_set_value(throttle_bar, 0, LV_ANIM_OFF);  // Initial value
    // Create a label for throttle
    throttle_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(throttle_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(throttle_label, "Throttle");
    lv_obj_align_to(throttle_label, throttle_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 5px below the bar
    
    rpm_bar = lv_bar_create(primary_data);
    lv_obj_set_size(rpm_bar, 475, 50);
    lv_obj_align(rpm_bar, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(rpm_bar, lv_color_black(), 0);
    lv_obj_set_style_border_width(rpm_bar, 10, 0);
    lv_bar_set_range(rpm_bar, 0, 12000); // example range
    lv_bar_set_value(rpm_bar, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(rpm_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Style bar
    lv_obj_set_style_bg_color(rpm_bar, lv_color_make(30, 30, 30), LV_PART_MAIN);
    lv_obj_set_style_radius(rpm_bar, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rpm_bar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
    lv_obj_set_style_radius(rpm_bar, 10, LV_PART_INDICATOR);
        

    // RPM Value Label (outside arc)
    rpm_label = lv_label_create(primary_data);
    lv_label_set_text(rpm_label, "0000");
    lv_obj_set_style_text_font(rpm_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(rpm_label, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_bg_color(rpm_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rpm_label, LV_OPA_COVER, LV_PART_MAIN);    
    lv_obj_align_to(rpm_label, rpm_bar, LV_ALIGN_CENTER, 0, 70);

    lv_obj_t *rpm_unit_label = lv_label_create(primary_data);
    lv_label_set_text(rpm_unit_label, "RPM");
    lv_obj_set_style_text_font(rpm_unit_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(rpm_unit_label, lv_color_hex(0x999999), 0);
    lv_obj_align_to(rpm_unit_label, rpm_bar, LV_ALIGN_CENTER, 0, 100);

    
    // --- Gear Label ---
    gear_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(gear_label, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_label_set_text(gear_label, "N");
    // Set background color and opacity
    lv_obj_set_style_bg_color(gear_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(gear_label, LV_OPA_COVER, LV_PART_MAIN);    
    lv_obj_align(gear_label, LV_ALIGN_CENTER, 0, 0);

    // --- Speed Label ---
    speed_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(speed_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(speed_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_color(speed_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(speed_label, LV_OPA_COVER, LV_PART_MAIN);   
    lv_label_set_text(speed_label, "000");
    lv_obj_align(speed_label, LV_ALIGN_BOTTOM_MID, 0, -50);
    lv_obj_t *kmh_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(kmh_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(kmh_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(kmh_label, "KM/H");
    lv_obj_align(kmh_label, LV_ALIGN_BOTTOM_MID, 50, -50);

    // Battery label
    battery_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(battery_label, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_bg_color(battery_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(battery_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_text(battery_label, "0");
    lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);


    // Page 2: secondary data
    lv_obj_t *secondary_data = lv_obj_create(main_container);
    lv_obj_set_size(secondary_data, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align_to(secondary_data, primary_data, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

    // Fuel Temp Label
    lv_obj_t *fuel_temp_label = lv_label_create(secondary_data);
    lv_obj_set_style_text_font(fuel_temp_label, &lv_font_montserrat_24, 0);
    lv_label_set_text(fuel_temp_label, "Fuel Temp");
    lv_obj_align(fuel_temp_label, LV_ALIGN_TOP_MID, 0, 20);

    temp_bar = lv_bar_create(secondary_data);
    lv_obj_set_size(temp_bar, 200, 20);
    lv_obj_align(temp_bar, LV_ALIGN_TOP_LEFT, 20, 200);
    lv_bar_set_range(temp_bar, 0, 120); // Coolant temp in °C
    
    fuel_bar = lv_bar_create(secondary_data);
    lv_obj_set_size(fuel_bar, 200, 20);
    lv_obj_align(fuel_bar, LV_ALIGN_TOP_LEFT, 20, 150);
    lv_bar_set_range(fuel_bar, 0, 100); // Example range for fuel %

    // Oil Pressure Label
    lv_obj_t *oil_pressure_label = lv_label_create(secondary_data);
    lv_obj_set_style_text_font(oil_pressure_label, &lv_font_montserrat_24, 0);
    lv_label_set_text(oil_pressure_label, "Oil Pressure");
    lv_obj_align(oil_pressure_label, LV_ALIGN_TOP_MID, 0, 80);
}

// Example function called periodically:
void update_dashboard(void) { //update peripheral
    //throttle position
    lv_bar_set_value(throttle_bar, g_throttle, LV_ANIM_ON);
    
    // --- RPM ---
    if (g_rpm > 12000) g_rpm = 12000;
    // Map RPM to arc value (assuming arc max is 100)
    lv_bar_set_value(rpm_bar, g_rpm, LV_ANIM_ON);
    lv_label_set_text_fmt(rpm_label, "%05d", g_rpm);  // Set text directly

    // --- Gear ---
    // Gear text
    if (gear_label != NULL && lv_obj_is_valid(gear_label)) {
        if (g_gear == 0) {
            lv_label_set_text(gear_label, "N");
        } else {
            lv_label_set_text_fmt(gear_label, "%d", g_gear);
        }
    } // Show gear as digit (or replace with "N" if gear == 0 etc.)

    // --- Speed ---
    lv_label_set_text_fmt(speed_label, "%03d", g_speed);

    lv_label_set_text_fmt(battery_label, "%02d", g_battery);

    // update more values

    // --- Fuel ---
    //lv_bar_set_value(fuel_bar, g_fuel, LV_ANIM_ON);

    // --- Coolant Temp ---
    //lv_bar_set_value(temp_bar, g_temp, LV_ANIM_ON);
}

void CAN_task(void *pvParameters) {
    while (1) {
        receive_can_message();  // No need to lock LVGL here

        if (lvgl_port_lock(-1)) {
            update_dashboard(); //since using globals no need for params
            lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(100)); //100
    }
}
