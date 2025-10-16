#include "test.h"
#include "lvgl.h"                     // For LVGL functions
#include "lvgl_port.h"               // For lvgl_port_lock/unlock (project specific)
#include "freertos/FreeRTOS.h"       // For vTaskDelay and pdMS_TO_TICKS
#include "freertos/task.h"
#include "twai.h"

extern QueueHandle_t xECU;
void decode_and_dispatch(const encoded_message_t *encoded_msg);

// Declare the fonts (this makes sure linker knows about them)
LV_FONT_DECLARE(lv_font_montserrat_36);
LV_FONT_DECLARE(lv_font_montserrat_48);

extern const lv_font_t lv_font_montserrat_72;
LV_FONT_DECLARE(lv_font_montserrat_72);

extern const lv_font_t lv_font_montserrat_bold_72;
LV_FONT_DECLARE(lv_font_montserrat_bold_72);

extern const lv_font_t lv_font_montserrat_bold_96;
LV_FONT_DECLARE(lv_font_montserrat_bold_96);


#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

//shifting
static lv_obj_t *shift_leds[NUM_SHIFT_LEDS];
static lv_color_t led_colors[NUM_SHIFT_LEDS];  // Current color state

static uint16_t dtc_code = 0;
static lv_obj_t *dtc_severity_label = NULL;
static uint8_t severity = 0;
int oil_val = 0;
int coolant_val = 0;
int fuel_val = 0;



static lv_obj_t *rpm_bar;
static lv_obj_t *rpm_label;
static lv_obj_t *gear_label;
static lv_obj_t *speed_label;
static lv_obj_t *fuel_bar;
static lv_obj_t *temp_bar;
static lv_obj_t *brake_bar;
static lv_obj_t *canvas;
static lv_obj_t *rpm_value;
static lv_obj_t *throttle_bar;
static lv_obj_t *throttle_label;
static lv_obj_t *battery_label;
static lv_obj_t *coolant_label;
static lv_obj_t *brake_label;
static lv_obj_t *severity_value_label, *dtc_code_label;


lv_obj_t *coolant_temp_label;
lv_obj_t *oil_temp_label;
lv_obj_t *oil_pressure_label;
lv_obj_t *fuel_pressure_label;


// Make sure you’ve got these fonts enabled in your lv_conf.h
LV_FONT_DECLARE(lv_font_montserrat_48);
LV_FONT_DECLARE(lv_font_montserrat_36);
LV_FONT_DECLARE(lv_font_montserrat_16);
LV_FONT_DECLARE(lv_font_montserrat_12);

void lv_tick_task(void* arg);  // Forward declaration


void dash_create2(void)
{
    lv_obj_t *main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Disable all scrolling directions
    lv_obj_set_scroll_dir(main_container, LV_DIR_NONE);

    // Disable scrollbars
    lv_obj_set_scrollbar_mode(main_container, LV_SCROLLBAR_MODE_OFF);

    // Optional: Clear scroll flags just to be safe
    lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(main_container, 0, 0);

    

    // Page 1: primary data
    lv_obj_t *primary_data = lv_obj_create(main_container);
    lv_obj_set_size(primary_data, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Disable all scrolling directions
    lv_obj_set_scroll_dir(primary_data, LV_DIR_NONE);
    // Disable scrollbars
    lv_obj_set_scrollbar_mode(primary_data, LV_SCROLLBAR_MODE_OFF);
    // Optional: Clear scroll flags just to be safe
    lv_obj_clear_flag(primary_data, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_align(primary_data, LV_ALIGN_LEFT_MID, 0, 0);
    
    lv_obj_set_style_bg_color(primary_data, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(primary_data, LV_OPA_COVER, 0);

    // Create a style for the throttle bar (optional)
    static lv_style_t throttle_style;
    lv_style_init(&throttle_style);
    lv_style_set_bg_color(&throttle_style, lv_color_hex(0x00FF00));  // Green fill
    lv_style_set_radius(&throttle_style, 5);

    // Create the bar
    throttle_bar = lv_bar_create(primary_data);
    lv_obj_add_style(throttle_bar, &throttle_style, LV_PART_INDICATOR);
    lv_obj_set_size(throttle_bar, 30, 300);  // Width, Height
    lv_obj_set_style_bg_color(throttle_bar, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_border_width(throttle_bar, 10, 0);
    lv_obj_align(throttle_bar, LV_ALIGN_LEFT_MID, 10, 0);  // Left center of screen with 10px margin
    lv_bar_set_range(throttle_bar, 0, 100);  // Throttle % from 0 to 100
    lv_bar_set_value(throttle_bar, 0, LV_ANIM_OFF);  // Initial value
    // Create a label for throttle
    throttle_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(throttle_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(throttle_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(throttle_label, "Throttle");
    lv_obj_align_to(throttle_label, throttle_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 5px below the bar
    
    // Create a style for the brake pressure bar
    static lv_style_t brake_style;
    lv_style_init(&brake_style);
    lv_style_set_bg_color(&brake_style, lv_color_hex(0xFF4500));  // Orange-Red fill
    lv_style_set_radius(&brake_style, 5);

    // Create the bar
    brake_bar = lv_bar_create(primary_data);
    lv_obj_add_style(brake_bar, &brake_style, LV_PART_INDICATOR);
    lv_obj_set_size(brake_bar, 30, 300);  // Width, Height (same as throttle)
    lv_obj_set_style_bg_color(brake_bar, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_border_width(brake_bar, 10, 0);
    lv_obj_align_to(brake_bar, throttle_bar, LV_ALIGN_OUT_RIGHT_MID, 20, 0);  // 20px space to the right

    // Create a label for brake pressure
    brake_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(brake_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(brake_label, "Brake");
    lv_obj_set_style_text_color(brake_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align_to(brake_label, brake_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Below the brake bar

    // Set bar range and initial value
    lv_bar_set_range(brake_bar, 0, 100);  // Pressure % (adjust range if using psi/bar/etc.)
    lv_bar_set_value(brake_bar, 0, LV_ANIM_OFF);
    
    //DTC
    lv_obj_t *dtc_container = lv_obj_create(primary_data);
    lv_obj_set_size(dtc_container, 180, 100);
    lv_obj_align_to(dtc_container, brake_bar, LV_ALIGN_OUT_RIGHT_MID, 20, 30);
    lv_obj_set_style_bg_color(dtc_container, lv_color_black(), 0);
    lv_obj_set_style_border_width(dtc_container, 2, 0);
    lv_obj_set_style_border_color(dtc_container, lv_color_hex(0x555555), 0);
    lv_obj_clear_flag(dtc_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *severity_label = lv_label_create(dtc_container);
    lv_label_set_text(severity_label, "Severity:");
    lv_obj_set_style_text_color(severity_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(severity_label, LV_ALIGN_BOTTOM_LEFT, 10, -40);

    lv_obj_t *severity_value_label = lv_label_create(dtc_container);
    lv_label_set_text(severity_value_label, "-");
    lv_obj_set_style_text_color(severity_value_label, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align_to(severity_value_label, severity_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *dtc_code_label = lv_label_create(dtc_container);
    lv_label_set_text(dtc_code_label, "DTC: ----");
    lv_obj_set_style_text_color(dtc_code_label, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_obj_align(dtc_code_label, LV_ALIGN_BOTTOM_LEFT, 10, -15);

    rpm_bar = shift_light_create(primary_data);
        

    // RPM Value Label (outside arc)
    rpm_label = lv_label_create(primary_data);
    lv_label_set_text(rpm_label, "-----");
    lv_obj_set_style_text_font(rpm_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(rpm_label, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_bg_color(rpm_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN); //lv_color_black()
    lv_obj_set_style_bg_opa(rpm_label, LV_OPA_COVER, LV_PART_MAIN);    
    lv_obj_align_to(rpm_label, rpm_bar, LV_ALIGN_TOP_MID, -200, 90); //fix later

    lv_obj_t *rpm_unit_label = lv_label_create(primary_data);
    lv_label_set_text(rpm_unit_label, "RPM");
    lv_obj_set_style_text_font(rpm_unit_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(rpm_unit_label, lv_color_hex(0x999999), 0);
    lv_obj_align_to(rpm_unit_label, rpm_bar, LV_ALIGN_TOP_MID, -200, 145);

    
    // --- Gear Label --- create a container for this in future like DTC
    gear_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_bold_96, 0); //works with 48
    lv_obj_set_style_text_color(gear_label, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_label_set_text(gear_label, "B");
    // Set background color and opacity
    lv_obj_set_style_bg_color(gear_label,  lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(gear_label, LV_OPA_COVER, LV_PART_MAIN);    
    lv_obj_align(gear_label, LV_ALIGN_CENTER, 0, -30);

    // --- Speed Label ---
    speed_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(speed_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(speed_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_color(speed_label,  lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(speed_label, LV_OPA_COVER, LV_PART_MAIN);   
    lv_label_set_text(speed_label, "---");
    lv_obj_align(speed_label, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_t *kmh_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(kmh_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(kmh_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_label_set_text(kmh_label, "KM/H");
    lv_obj_align(kmh_label, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Battery label
    battery_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(battery_label, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_bg_color(battery_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(battery_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_text(battery_label, "-");
    lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    // === RIGHT-SIDE SENSOR LABELS (Middle Right) ===
    lv_coord_t label_x = SCREEN_WIDTH - 50;  // Align near right edge
    lv_coord_t label_y = SCREEN_HEIGHT / 2 - 100;

    // Coolant Temp
    coolant_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(coolant_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(coolant_label, "Coolant (°C)");  // You can remove () if desired
    lv_obj_set_style_text_color(coolant_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(coolant_label, LV_ALIGN_TOP_RIGHT, -90, label_y);

    coolant_temp_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(coolant_temp_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(coolant_temp_label, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(coolant_temp_label, "ABC 123");
    lv_obj_align_to(coolant_temp_label, coolant_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);  // 10px horizontal gap

    // Oil Temp
    lv_obj_t *oil_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(oil_label, "Oil Temp (°C)");
    lv_obj_set_style_text_color(oil_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align_to(oil_label, coolant_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);  // 10px vertical gap

    oil_temp_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_temp_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(oil_temp_label, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_label_set_text(oil_temp_label, "ABC 123");
    lv_obj_align_to(oil_temp_label, oil_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);  // 10px horizontal gap

    // Oil Pressure
    lv_obj_t *oil_ps_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_ps_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(oil_ps_label, "Oil PSI");
    lv_obj_set_style_text_color(oil_ps_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align_to(oil_ps_label, oil_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);  // 10px vertical gap

    oil_pressure_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_pressure_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(oil_pressure_label, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_label_set_text(oil_pressure_label, "ABC 123");
    lv_obj_align_to(oil_pressure_label, oil_ps_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 10px horizontal gap

    // Fuel Pressure
    lv_obj_t *fuel_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(fuel_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(fuel_label, "Fuel PSI:");
    lv_obj_set_style_text_color(fuel_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align_to(fuel_label, oil_ps_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);  // 10px vertical gap

    fuel_pressure_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(fuel_pressure_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(fuel_pressure_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(fuel_pressure_label, "ABC 123");
    lv_obj_align_to(fuel_pressure_label, fuel_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 10px horizontal gap
}

lv_obj_t* shift_light_create(lv_obj_t *parent) {
    lv_coord_t start_x = (800 - (NUM_SHIFT_LEDS * SHIFT_LED_WIDTH + (NUM_SHIFT_LEDS - 1) * SHIFT_LED_GAP)) / 2;
    lv_coord_t y = 50;

    for (int i = 0; i < NUM_SHIFT_LEDS; i++) {
        shift_leds[i] = lv_obj_create(parent);
        lv_obj_set_size(shift_leds[i], SHIFT_LED_WIDTH, SHIFT_LED_HEIGHT);
        lv_obj_clear_flag(shift_leds[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(shift_leds[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(shift_leds[i], lv_color_hex(0xAAAAAA), 0);
        lv_obj_set_style_bg_opa(shift_leds[i], LV_OPA_COVER, 0);
        lv_obj_align(shift_leds[i], LV_ALIGN_TOP_LEFT, start_x + i * (SHIFT_LED_WIDTH + SHIFT_LED_GAP), y);

        led_colors[i] = lv_color_hex(0xAAAAAA);
    }

    return parent;
}

void shift_light_update(uint16_t rpm) {
    for (int i = 0; i < NUM_SHIFT_LEDS; i++) {
        if (!shift_leds[i] || !lv_obj_is_valid(shift_leds[i])) continue;

        if (rpm >= rpm_thresholds[i]) {
            // Color based on how far you are in the LED range
            if (i < NUM_SHIFT_LEDS * 0.4) {
                // First 60%: green
                lv_obj_set_style_bg_color(shift_leds[i], lv_color_hex(0x00FF00), 0);
            } else if (i < NUM_SHIFT_LEDS * 0.75) {
                // Next ~25%: yellow
                lv_obj_set_style_bg_color(shift_leds[i], lv_color_hex(0xFFFF00), 0);
            } else if (i < NUM_SHIFT_LEDS * 0.9) {
                // Next ~10%: red
                lv_obj_set_style_bg_color(shift_leds[i], lv_color_hex(0xFF0000), 0);
            } else {
                // Final ~5%: purple
                lv_obj_set_style_bg_color(shift_leds[i], lv_color_hex(0x800080), 0);  // Purple
            }
        } else {
            // Dim/off
            lv_obj_set_style_bg_color(shift_leds[i], lv_color_hex(0x222222), 0);
        }
    }
}


void decode_and_dispatch(const encoded_message_t *encoded_msg) {
    if (encoded_msg->length < 2) return;  // must have at least 2 bytes for ID

    uint16_t id = (encoded_msg->data[0] << 8) | encoded_msg->data[1];
    const uint8_t *payload = encoded_msg->data + 2;
    size_t payload_len = encoded_msg->length - 2;

    switch (id) {

    case 992: { // 0x3E0: Coolant & Oil Temp
        if (payload_len >= 2 * sizeof(float)) {

            float coolant_temp = *((float *)(payload));
            float oil_temp     = *((float *)(payload + sizeof(float)));

            int coolant_val = (int)(coolant_temp * 10);
            int oil_val     = (int)(oil_temp * 10);

            char buf_coolant[8];
            char buf_oil[8];
            int start_coolant = 0, start_oil = 0;

            // --- Format coolant ---
            buf_coolant[0] = '0' + (coolant_val / 1000) % 10;
            buf_coolant[1] = '0' + (coolant_val / 100) % 10;
            buf_coolant[2] = '0' + (coolant_val / 10) % 10;
            buf_coolant[3] = '.';
            buf_coolant[4] = '0' + (coolant_val % 10);
            buf_coolant[5] = '\0';

            while (start_coolant < 3 && buf_coolant[start_coolant] == '0') start_coolant++;
            if (start_coolant == 3) start_coolant--;

            // --- Format oil ---
            buf_oil[0] = '0' + (oil_val / 1000) % 10;
            buf_oil[1] = '0' + (oil_val / 100) % 10;
            buf_oil[2] = '0' + (oil_val / 10) % 10;
            buf_oil[3] = '.';
            buf_oil[4] = '0' + (oil_val % 10);
            buf_oil[5] = '\0';

            while (start_oil < 3 && buf_oil[start_oil] == '0') start_oil++;
            if (start_oil == 3) start_oil--;

            // --- LVGL update (one lock) ---
            if (lvgl_port_lock(-1)) {

                // Color warnings
                if (oil_val >= 1000) {
                    lv_obj_set_style_bg_color(oil_temp_label, lv_color_hex(0x550000), 0);
                } else {
                    lv_obj_set_style_bg_color(oil_temp_label, lv_color_hex(0x000000), 0);
                }

                if (coolant_val >= 1000) {
                    lv_obj_set_style_bg_color(coolant_temp_label, lv_color_hex(0x550000), 0);
                } else {
                    lv_obj_set_style_bg_color(coolant_temp_label, lv_color_hex(0x000000), 0);
                }

                // Text updates
                if (coolant_temp_label && lv_obj_is_valid(coolant_temp_label)) { //validate inside lock as state may change if checked before/after locking
                    lv_label_set_text(coolant_temp_label, &buf_coolant[start_coolant]);
                }

                if (oil_temp_label && lv_obj_is_valid(oil_temp_label)) {
                    lv_label_set_text(oil_temp_label, &buf_oil[start_oil]);
                }

                lvgl_port_unlock();
            }
        }
        break;
    }



    case 1136: { // 0x470: Gear
        static uint8_t last_gear = 0xFF;  // impossible initial value

        uint8_t gear = *((uint8_t*)(payload));

        /*
        if (gear == 0x00 && last_gear <= 0x02) {
            return;
        }
            */

        if (gear == last_gear) {
            break;
        }
        else{
        last_gear = gear;}

        static char gear_str[3];  // Enough for "NN" + null terminator

        switch (last_gear) {
            case 0x00:
                gear_str[0] = 'N';
                gear_str[1] = '\0';
                break;
            case 0x0E:
                gear_str[0] = 'R';
                gear_str[1] = '\0';
                break;
            case 0x0F:
                gear_str[0] = 'P';
                gear_str[1] = '\0';
                break;
            default:
                if (last_gear > 0 && last_gear < 0x0E) {
                    gear_str[0] = '0' + last_gear;
                    gear_str[1] = '\0';
                } else {
                    gear_str[0] = '-';
                    gear_str[1] = '\0';
                }
                break;
        }
        if (lvgl_port_lock(-1)) {
            lv_label_set_text(gear_label, gear_str);
            lvgl_port_unlock();
            }

        break;
    }
    
    case 864: { // 0x360: RPM & Throttle
        if (payload_len >= sizeof(uint16_t) + sizeof(float)) {

            uint16_t rpm = *((uint16_t *)(payload));
            float throttle = *((float *)(payload + sizeof(float)));

            static uint32_t last_throttlerpm_time = 0;
            uint32_t now = xTaskGetTickCount();
            if ((now - last_throttlerpm_time) < pdMS_TO_TICKS(10)) break;
            last_throttlerpm_time = now;

            if (rpm > 11500) rpm = 11500;

            // Lock once for all LVGL updates in this frame to reduce CPU usage and mutex load
            if (lvgl_port_lock(-1)) {

                // --- Throttle bar ---
                if (throttle_bar && lv_obj_is_valid(throttle_bar)) {
                    lv_bar_set_value(throttle_bar, (int)throttle, LV_ANIM_OFF);
                }

                // --- Shift lights ---
                shift_light_update(rpm); // safe to call here if it uses the same lock or doesn't touch LVGL

                // --- Gear label background color ---
                if (gear_label && lv_obj_is_valid(gear_label)) {

                    lv_color_t color = lv_color_hex(0x000000); // default

                    if (rpm >= 10800) { // flash limiter red
                        static bool flash = false;
                        flash = !flash;
                        color = flash ? lv_color_hex(0xFF0000) : lv_color_hex(0x000000);

                    } else if (rpm >= 9000) {
                        color = lv_color_hex(0x550000); // shift-up warning

                    } else if (rpm <= 3000) {
                        color = lv_color_hex(0x990000); // low rev warning
                    }

                    lv_obj_set_style_bg_color(gear_label, color, 0);
                }

                lvgl_port_unlock();
            }
        }
        break;
    }


    case 865: { // 0x361: Fuel & Oil Pressure
        if (payload_len >= 2 * sizeof(float)) {
            float fuel_pressure = *((float *)(payload));
            float oil_pressure  = *((float *)(payload + sizeof(float)));

            int fuel_val = (int)(fuel_pressure * 10);
            int oil_val  = (int)(oil_pressure * 10);

            char buf_fuel[8];
            char buf_oil[8];
            int start_fuel = 0, start_oil = 0;

            // --- Format Fuel Pressure ---
            buf_fuel[0] = '0' + (fuel_val / 1000) % 10;
            buf_fuel[1] = '0' + (fuel_val / 100) % 10;
            buf_fuel[2] = '0' + (fuel_val / 10) % 10;
            buf_fuel[3] = '.';
            buf_fuel[4] = '0' + (fuel_val % 10);
            buf_fuel[5] = '\0';

            while (start_fuel < 4 && buf_fuel[start_fuel] == '0') start_fuel++;
            if (start_fuel == 4) start_fuel--;

            // --- Format Oil Pressure --- (Haltech Normalisations)
            buf_oil[0] = '0' + (oil_val / 1000) % 10;
            buf_oil[1] = '0' + (oil_val / 100) % 10;
            buf_oil[2] = '0' + (oil_val / 10) % 10;
            buf_oil[3] = '.';
            buf_oil[4] = '0' + (oil_val % 10);
            buf_oil[5] = '\0';

            while (start_oil < 4 && buf_oil[start_oil] == '0') start_oil++;
            if (start_oil == 4) start_oil--;

            // --- LVGL update (one lock for both) ---
            if (lvgl_port_lock(-1)) {

                // Update labels if they exist and are valid
                if (fuel_pressure_label && lv_obj_is_valid(fuel_pressure_label)) {
                    lv_label_set_text(fuel_pressure_label, &buf_fuel[start_fuel]);
                }

                if (oil_pressure_label && lv_obj_is_valid(oil_pressure_label)) {
                    lv_label_set_text(oil_pressure_label, &buf_oil[start_oil]);
                }

                lvgl_port_unlock();
            }
        }
        break;
    }

    case 875: { // 0x36B: Brake Pressure

        static uint32_t last_brake_time = 0;
        uint32_t now = xTaskGetTickCount();
        if ((now - last_brake_time) < pdMS_TO_TICKS(10)) break;

        last_brake_time = now;
        if (payload_len >= sizeof(float)) {
            float brake_pressure = *((float *)(payload));

            // Clamp to valid bar range (0–100)
            if (brake_pressure < 0) brake_pressure = 0;
            if (brake_pressure > 100) brake_pressure = 100;

            if (brake_bar && lv_obj_is_valid(brake_bar)) {
                if (lvgl_port_lock(-1)) {
                    lv_bar_set_value(brake_bar, (int)brake_pressure, LV_ANIM_OFF); //turn on if want animation to next value
                    lvgl_port_unlock();
                }
                break;
            }
        }
        break;
    }

    case 880: { // 0x370: Vehicle Speed
        //printf("Speed payload bytes: %02X %02X %02X %02X\n", payload[0], payload[1], payload[2], payload[3]);
        if (payload_len >= sizeof(float)) {
            float speed_float = *((float *)(payload));
            int speed = (int)speed_float;


            if (speed_label && lv_obj_is_valid(speed_label)) {
                // Build a 3-digit zero-padded string manually (e.g., "005", "120")
                char buf[4];
                buf[0] = '0' + (speed / 100) % 10;
                buf[1] = '0' + (speed / 10) % 10;
                buf[2] = '0' + speed % 10;
                buf[3] = '\0';

                if (lvgl_port_lock(-1)) {
                    lv_label_set_text(speed_label, buf); //check when wheel speed sensors attached
                    lvgl_port_unlock();
                }
                break;
            }
        }
        break;
    }

    case 882: { // Battery voltage
        //printf("Voltage payload bytes: %02X %02X %02X %02X\n", payload[0], payload[1], payload[2], payload[3]);
        if (payload_len >= sizeof(float)) {
            float voltage = *((float *)(payload));

            if (battery_label && lv_obj_is_valid(battery_label)) {
                // Manually build string for voltage without using sprintf
                int volts_int = (int)voltage;
                int volts_frac = (int)((voltage - volts_int) * 100);

                char buf[16];
                buf[0] = '0' + (volts_int / 10);    // tens digit
                buf[1] = '0' + (volts_int % 10);    // ones digit
                buf[2] = '.';
                buf[3] = '0' + (volts_frac / 10);   // tenths
                buf[4] = '0' + (volts_frac % 10);   // hundredths
                buf[5] = '\0';

                if (lvgl_port_lock(-1)) {
                    lv_label_set_text(battery_label, buf);
                    lvgl_port_unlock();
                }
                break;
            }
        }
        break;
    }

    case 1001: { // 0x3E9: Lambda
            if (payload_len >= sizeof(float)) {
                float lambda = *((float *)(payload));
                // Optional: update lambda display
            }
            break;
        }
    case 1779: {  // 0x6F3
        if (payload_len < 8) break;

        uint8_t severity = payload[5];
        uint16_t raw_dtc = (payload[6] << 8) | payload[7];

        char dtc_letter;
        uint8_t prefix = (raw_dtc >> 14) & 0x03;
        switch (prefix) {
            case 0: dtc_letter = 'P'; break;
            case 1: dtc_letter = 'B'; break;
            case 2: dtc_letter = 'C'; break;
            case 3: dtc_letter = 'U'; break;
            default: dtc_letter = '?'; break;
        }

        uint16_t dtc_number = raw_dtc & 0x3FFF;
        char dtc_str[7];  // 1 + 4 + 1
        dtc_str[0] = dtc_letter;

        for (int i = 0; i < 4; i++) {
            uint8_t nibble = (dtc_number >> (12 - 4 * i)) & 0xF;
            dtc_str[i + 1] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        }
        dtc_str[5] = '\0';

        /*
        if (dtc_letter == 'P') {
            switch (dtc_number) {
                case 0x2A00:
                    //printf("Fault: Wideband 1 Sensor Failure\n");
                    break;
                case 0x0101:
                    //printf("Fault: Coolant Temp Sensor\n");
                    break;
                case 0x0307:
                    //printf("Fault: Knock Sensor\n");
                    break;
                default:
                    //printf("Unknown Powertrain DTC\n");
                    break;
            }
        }
        */
        if (lvgl_port_lock(-1)) {
        if (dtc_code_label && lv_obj_is_valid(dtc_code_label)) {
                lv_label_set_text(dtc_code_label, dtc_str);
                }
        if (dtc_severity_label && lv_obj_is_valid(dtc_severity_label)) {
                    lv_label_set_text(dtc_severity_label, severity);
                }
                lvgl_port_unlock();
            }
            break;  
            } 
        }
    }




float swap_float_bytes(const uint8_t *data) {
    uint8_t temp[4];
    temp[0] = data[3];
    temp[1] = data[2];
    temp[2] = data[1];
    temp[3] = data[0];
    float val;
    memcpy(&val, temp, sizeof(float));
    return val;
}



void init_lvgl_tick_timer(void) {
    static const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = lv_tick_task,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "lv_tick"
};

    static esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 1000)); // 1ms = 1000us
}

void lv_tick_task(void* arg) {
    lv_tick_inc(1);  // Inform LVGL that 1ms has passed
}

void LVGL_Task(void *pvParameters) {
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(30)); // 200hz 16ms for 60FPS, 30ms for ~33FPS
    }
}

void Display_Task(void *pvParameters) {

    encoded_message_t msg;

    while (1) { 
        if (xQueueReceive(xECU, &msg, pdMS_TO_TICKS(200)) == pdPASS) {
            decode_and_dispatch(&msg);  // decode will lock internally per update
            }
            else {
            vTaskDelay(pdMS_TO_TICKS(50));  // Prevent tight loop when no message
        }
    }
}