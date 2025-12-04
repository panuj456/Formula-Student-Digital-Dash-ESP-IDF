#include "test.h"
#include "lvgl.h"                     // For LVGL functions
#include "lvgl_port.h"               // For lvgl_port_lock/unlock (project specific)
#include "freertos/FreeRTOS.h"       // For vTaskDelay and pdMS_TO_TICKS
#include "freertos/task.h"
#include "twai.h"
#include "dash_state.h"


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
    // Calculate starting X position for centering (assuming 800px wide display)
    lv_coord_t total_width = (NUM_SHIFT_LEDS * SHIFT_LED_WIDTH + (NUM_SHIFT_LEDS - 1) * SHIFT_LED_GAP);
    lv_coord_t start_x = (800 - total_width) / 2;
    lv_coord_t y = 50;

    for (int i = 0; i < NUM_SHIFT_LEDS; i++) {
        // Create the individual LED object and store its pointer globally
        shift_leds[i] = lv_obj_create(parent);
        lv_obj_set_size(shift_leds[i], SHIFT_LED_WIDTH, SHIFT_LED_HEIGHT);
        lv_obj_clear_flag(shift_leds[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(shift_leds[i], LV_RADIUS_CIRCLE, 0);
        
        // Set initial background color (grey)
        lv_color_t default_color = lv_color_hex(0xAAAAAA);
        lv_obj_set_style_bg_color(shift_leds[i], default_color, 0);
        lv_obj_set_style_bg_opa(shift_leds[i], LV_OPA_COVER, 0);
        
        lv_obj_align(shift_leds[i], LV_ALIGN_TOP_LEFT, start_x + i * (SHIFT_LED_WIDTH + SHIFT_LED_GAP), y);

        // Store the default color in our state array
        led_colors[i] = default_color;
    }

    return parent;
}

void update_shift_lights_optimized(uint16_t current_rpm) {
    uint32_t active_color_hex = 0xAAAAAA; // Default color (grey/off)

    // Iterate once through all potential LED stages
    for (int i = 0; i < NUM_SHIFT_LEDS; i++) {
        
        // Check if the current RPM meets or exceeds this specific LED's threshold
        if (current_rpm >= shift_map[i].threshold) {
            
            // If yes, update the current active color to the color for this stage
            active_color_hex = shift_map[i].color_rgb;
            
            // Turn this specific LED object ON with the determined color
            lv_color_t target_color = lv_color_hex(active_color_hex);
            
            if (lv_color_cmp(led_colors[i], target_color) != 0) {
                lv_obj_set_style_bg_color(shift_leds[i], target_color, 0);
                led_colors[i] = target_color;
            }

        } else {
            
            // If the RPM falls below this threshold, all subsequent LEDs must be OFF
            lv_color_t target_color = lv_color_hex(0xAAAAAA); // Grey (off)

            if (lv_color_cmp(led_colors[i], target_color) != 0) {
                lv_obj_set_style_bg_color(shift_leds[i], target_color, 0);
                led_colors[i] = target_color;
            }
            
            // Since the thresholds are sorted, we can immediately break the loop
            // as all remaining LEDs will also be off.h
            // Note: If you want *all* LEDs off above a certain threshold (rather than just subsequent ones),
            // you'd need slightly different logic, but this works for typical sequential shift lights.
            // break; 
        }
    }

    // Flashing logic for MAX RPM can still be added here
    if (current_rpm >= RPM_MAX) {
       // initiate flashing routine using LVGL animations or a timer
    }
}

static lv_color_t compute_gear_bg_color(uint16_t rpm)
{
    if (rpm >= 10800) return lv_color_hex(0xFF0000);     // limiter flash (caller can toggle flash bit)
    if (rpm >= 9000)  return lv_color_hex(0x550000);     // shift-up alarm
    if (rpm <= 3000)  return lv_color_hex(0x990000);     // low rev
    return lv_color_hex(0x000000);                       // normal
}

void display_shift_light_color(uint16_t current_rpm) {
    uint32_t active_color = 0x000000; // Default off (black)

    // Iterate through the map to find the appropriate color based on RPM
    for (size_t i = 0; i < NUM_SHIFT_LEDS; i++) {
        if (current_rpm >= shift_map[i].threshold) {
            active_color = shift_map[i].color_rgb;
        } else {
            // Since thresholds are ordered, we can stop iterating once we pass the current RPM
            break; 
        }
    }
}


void decode_and_dispatch(const encoded_message_t *encoded_msg)
{
    for (;;) {
        if (xQueueReceive(xECU, &msg, rx_timeout) != pdPASS) {
            // nothing received; loop (optionally sleep a short time)
            taskYIELD();
            continue;
        }

    // parse encoded message: first two bytes = ID, remainder payload
    if (msg.length < 2) continue;
    uint16_t id = ((uint16_t)msg.data[0] << 8) | msg.data[1];
    const uint8_t *payload = &msg.data[2];
    size_t dlc = msg.length - 2;
    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

    dash_state_t local = {0};
    bool want_commit = false;

    switch (id) {
        case 1136: {
            if (dlc >= 8) {
                local.gear = payload[7];
                local.last_update_ms = now_ms;
                
                want_commit = true;
            }
            break;
        }

        case 864: { // 0x360: RPM & Throttle
            if (dlc >= 6) {
                uint16_t rpm_raw = (payload[0] << 8) | payload[1];
                uint16_t throttle_raw = (payload[4] << 8) | payload[5];

                // store raw in back buffer
                local.rpm = rpm_raw;
                local.throttle = throttle_raw / 10.0f;
                local.last_update_ms = now_ms;
                
                local.gear_bg_color = compute_gear_bg_color(local.rpm);
                want_commit = true;
            }
            break;
        }
        case 992: { // Coolant & Oil Temp
            uint16_t raw_coolant = (received_msg.data[0] << 8) | received_msg.data[1];
            uint16_t raw_oil     = (received_msg.data[6] << 8) | received_msg.data[7];

            // Haltech normalisation: Kelvin -> Celsius
            local.coolant_temp = (raw_coolant / 10.0f) - 273.15f;
            local.oil_temp     = (raw_oil     / 10.0f) - 273.15f;

            local.last_update_ms = esp_timer_get_time() / 1000;
            want_commit = true;
            break;
            }
        case 865: {
            uint16_t raw_fuel  = (received_msg.data[0] << 8) | received_msg.data[1];
            uint16_t raw_oilp  = (received_msg.data[2] << 8) | received_msg.data[3];

            // Haltech normalisation: kPa minus atmospheric offset
            local.fuel_pressure = (raw_fuel / 10.0f) - 101.3f;
            local.oil_pressure  = (raw_oilp / 10.0f) - 101.3f;

            local.last_update_ms = now_ms;
            want_commit.dirty = true;
            break;
        }
        case 875: {
            if (dlc >= 2) {
                uint16_t brake_raw = (payload[0] << 8) | payload[1];
                local.brake_pressure = (brake_raw / 10.0f) - 101.3f;
                local.last_update_ms = now_ms;
                want_commit.dirty = true;
            }
            break;
        }
        case 880: {
            if (dlc >= 2) {
                uint16_t speed_raw = (payload[0] << 8) | payload[1];
                local.speed = speed_raw / 10.0f;
                local.last_update_ms = now_ms;
                want_commit = true;
            }
            break;
        }
        case 882: {
            if (dlc >= 2) {
                uint16_t raw_voltage = (payload[0] << 8) | payload[1];
                local.voltage = raw_voltage / 10.0f;

                local.last_update_ms = now_ms;
                want_commit = true;
            }
            break;
        }
        case 1001: {
            if (dlc >= 6) {
                uint16_t raw_lambda = (payload[4] << 8) | payload[5];

                // Haltech normalisation: lambda = raw / 1000
                local.lambda = raw_lambda / 1000.0f;

                local.last_update_ms = now_ms;
                want_commit = true;
            }
            break;
        }

        case 0x363: {
            //wheel slip/diff
            if (dlc >= 4) {
                uint16_t raw_slip = (payload[0] << 8) | payload[1];
                uint16_t raw_diff = (payload[2] << 8) | payload[3];

                local.wheel_slip = raw_slip / 10.0f;
                local.wheel_diff = raw_diff / 10.0f;

                local.last_update_ms = now_ms;
                want_commit = true;
            }
            break;
        }

        case 0x36C: {
            //wheel speed
            if (dlc >= 8) {
                local.wheel_speed_fl = ((payload[0] << 8) | payload[1]) / 10.0f;
                local.wheel_speed_fr = ((payload[2] << 8) | payload[3]) / 10.0f;
                local.wheel_speed_rl = ((payload[4] << 8) | payload[5]) / 10.0f;
                local.wheel_speed_rr = ((payload[6] << 8) | payload[7]) / 10.0f;

                local.last_update_ms = now_ms;
                want_commit = true;
            }
            break;
        }
        case 1779: {

            // Engine protection severity
            if (dlc >= 8) {

                uint8_t leaking_flags = payload[4];
                uint8_t severity = payload[5];
                uint16_t dtc = (payload[6] << 8) | payload[7];

                local.leak_flags   = leaking_flags;
                local.severity     = severity;
                local.dtc_code     = dtc;

                local.last_update_ms = now_ms;
                want_commit = true;
            }
            break;
        }

        default:
            // ignore
            break;
    }
    taskENTER_CRITICAL();
        // Minimal copy: for each field that may be non-zero in local copy them
        if (local.last_update_ms) g_dash_back.last_update_ms = local.last_update_ms;
        if (local.rpm) g_dash_back.rpm = local.rpm;
        if (local.throttle) g_dash_back.throttle = local.throttle;
        if (local.lambda) g_dash_back.lambda = local.lambda;
        if (local.speed) g_dash_back.speed = local.speed;
        if (local.brake) g_dash_back.brake = local.brake;
        if (local.voltage) g_dash_back.voltage = local.voltage;
        if (local.gear) g_dash_back.gear = local.gear;

        if (local.fuel_pressure) g_dash_back.fuel_pressure = local.fuel_pressure;
        if (local.oil_pressure) g_dash_back.oil_pressure = local.oil_pressure;
        if (local.coolant_temp) g_dash_back.coolant_temp = local.coolant_temp;
        if (local.oil_temp) g_dash_back.oil_temp = local.oil_temp;

        // wheel values (copy if non-zero)
        if (local.wheelSlipFL) g_dash_back.wheelSlipFL = local.wheelSlipFL;
        if (local.wheelSlipFR) g_dash_back.wheelSlipFR = local.wheelSlipFR;
        if (local.wheelSlipRL) g_dash_back.wheelSlipRL = local.wheelSlipRL;
        if (local.wheelSlipRR) g_dash_back.wheelSlipRR = local.wheelSlipRR;

        if (local.wheelSpeedFL) g_dash_back.wheelSpeedFL = local.wheelSpeedFL;
        if (local.wheelSpeedFR) g_dash_back.wheelSpeedFR = local.wheelSpeedFR;
        if (local.wheelSpeedRL) g_dash_back.wheelSpeedRL = local.wheelSpeedRL;
        if (local.wheelSpeedRR) g_dash_back.wheelSpeedRR = local.wheelSpeedRR;


            // gear bg colour
            if (local.gear_bg_color.full != 0) g_dash_back.gear_bg_color = local.gear_bg_color;

            // other bytes copy
            for (int i = 0; i < 8; ++i) {
                if (local.other_bytes[i]) g_dash_back.other_bytes[i] = local.other_bytes[i];
            }

        g_dash_back.dirty = true;
        taskEXIT_CRITICAL();
    }
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

void Decode_Task(void *pvParameters) {

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

void Display_Task(void *pvParameters) {
    static uint32_t last_ui_update = 0;

    while (1) {

        // Update UI at ~50Hz
        uint32_t now = esp_timer_get_time() / 1000;
        if (now - last_ui_update >= 20) {      
            last_ui_update = now;

                // swap buffers
            bool will_update = false;
            taskENTER_CRITICAL();
            if (g_dash_back.dirty) {
                g_dash_front = g_dash_back;        // fast struct copy
                g_dash_back.dirty = false;
                will_update = true;
            }
            taskEXIT_CRITICAL();

            if (!will_update) {
                // Nothing new — short sleep
                vTaskDelay(pdMS_TO_TICKS(2));
                continue;
            }
            // single LVGL lock; keep it short and only do DOM operations
            if (lvgl_port_lock(10)) { // 10 ms timeout
                // update textual labels (use lv_label_set_text or formatted helper)
                lv_label_set_text_fmt(rpm_label, "%u", g_dash_front.rpm);
                lv_label_set_text_fmt(throttle_label, "%.1f", g_dash_front.throttle);
                lv_label_set_text_fmt(lambda_label, "%.3f", g_dash_front.lambda);
                lv_label_set_text_fmt(speed_label, "%.1f", g_dash_front.speed);
                lv_label_set_text_fmt(brake_label, "%.1f", g_dash_front.brake);
                lv_label_set_text_fmt(voltage_label, "%.1f", g_dash_front.voltage);
                lv_label_set_text_fmt(gear_label, "%u", g_dash_front.gear);
                lv_label_set_text_fmt(fuel_pressure_label, "%.1f", g_dash_front.fuel_pressure);
                lv_label_set_text_fmt(oil_pressure_label, "%.1f", g_dash_front.oil_pressure);
                lv_label_set_text_fmt(coolant_label, "%.1f", g_dash_front.coolant_temp);
                lv_label_set_text_fmt(oiltemp_label, "%.1f", g_dash_front.oil_temp);

                // gear bg color (precomputed)
                lv_obj_set_style_bg_color(gear_label, g_dash_front.gear_bg_color, 0);
                update_shift_lights_optimized(g_dash_front.rpm);

                // shift LEDs: paint from precomputed colours
                for (int i = 0; i < NUM_SHIFT_LEDS; ++i) {
                    if (shift_leds[i] && lv_obj_is_valid(shift_leds[i])) {
                        lv_obj_set_style_bg_color(shift_leds[i], g_dash_front.shift_led_color[i], 0);
                    }
                }

                // ... any small DOM-only changes go here ...

                lvgl_port_unlock();
            } else {
                // could not lock LVGL quickly — skip this frame
                ESP_LOGW("DISPLAY", "lvgl lock timeout");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5));   // light sleep
    }
}