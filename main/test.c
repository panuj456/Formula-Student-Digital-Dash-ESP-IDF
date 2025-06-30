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


#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

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

lv_obj_t *coolant_temp_label;
lv_obj_t *oil_temp_label;
lv_obj_t *oil_pressure_label;
lv_obj_t *fuel_pressure_label;


// Make sure you’ve got these fonts enabled in your lv_conf.h
LV_FONT_DECLARE(lv_font_montserrat_48);
LV_FONT_DECLARE(lv_font_montserrat_36);
LV_FONT_DECLARE(lv_font_montserrat_16);
LV_FONT_DECLARE(lv_font_montserrat_12);

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

    rpm_bar = lv_bar_create(primary_data);
    lv_obj_set_size(rpm_bar, 475, 70);
    lv_obj_align(rpm_bar, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(rpm_bar, lv_color_hex(0xAAAAAA), 0);
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
    lv_label_set_text(rpm_label, "-----");
    lv_obj_set_style_text_font(rpm_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(rpm_label, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_bg_color(rpm_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN); //lv_color_black()
    lv_obj_set_style_bg_opa(rpm_label, LV_OPA_COVER, LV_PART_MAIN);    
    lv_obj_align_to(rpm_label, rpm_bar, LV_ALIGN_CENTER, 0, 80); //fix later

    lv_obj_t *rpm_unit_label = lv_label_create(primary_data);
    lv_label_set_text(rpm_unit_label, "RPM");
    lv_obj_set_style_text_font(rpm_unit_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(rpm_unit_label, lv_color_hex(0x999999), 0);
    lv_obj_align_to(rpm_unit_label, rpm_bar, LV_ALIGN_CENTER, 0, 125);

    
    // --- Gear Label ---
    gear_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_bold_72, 0); //works with 48
    lv_obj_set_style_text_color(gear_label, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_label_set_text(gear_label, "ABC 123");
    // Set background color and opacity
    lv_obj_set_style_bg_color(gear_label,  lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(gear_label, LV_OPA_COVER, LV_PART_MAIN);    
    lv_obj_align(gear_label, LV_ALIGN_CENTER, 0, 30);

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
    lv_obj_set_style_text_font(coolant_temp_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(coolant_temp_label, lv_palette_main(LV_PALETTE_RED), 0);
    lv_label_set_text(coolant_temp_label, "ABC 123");
    lv_obj_align_to(coolant_temp_label, coolant_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 10px horizontal gap

    // Oil Temp
    lv_obj_t *oil_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(oil_label, "Oil Temp (°C)");
    lv_obj_set_style_text_color(oil_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align_to(oil_label, coolant_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);  // 10px vertical gap

    oil_temp_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_temp_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(oil_temp_label, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_label_set_text(oil_temp_label, "ABC 123");
    lv_obj_align_to(oil_temp_label, oil_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 10px horizontal gap

    // Oil Pressure
    lv_obj_t *oil_ps_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_ps_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(oil_ps_label, "Oil PSI");
    lv_obj_set_style_text_color(oil_ps_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align_to(oil_ps_label, oil_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);  // 10px vertical gap

    oil_pressure_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(oil_pressure_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(oil_pressure_label, lv_palette_main(LV_PALETTE_PURPLE), 0);
    lv_label_set_text(oil_pressure_label, "ABC 123");
    lv_obj_align_to(oil_pressure_label, oil_ps_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 10px horizontal gap

    // Fuel Pressure
    lv_obj_t *fuel_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(fuel_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(fuel_label, "Fuel PSI:");
    lv_obj_set_style_text_color(fuel_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align_to(fuel_label, oil_ps_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);  // 10px vertical gap

    fuel_pressure_label = lv_label_create(primary_data);
    lv_obj_set_style_text_font(fuel_pressure_label, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(fuel_pressure_label, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_label_set_text(fuel_pressure_label, "ABC 123");
    lv_obj_align_to(fuel_pressure_label, fuel_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 10px horizontal gap
}


void decode_and_dispatch(const encoded_message_t *encoded_msg) {
    if (encoded_msg->length < 2) return;  // must have at least 2 bytes for ID

    uint16_t id = (encoded_msg->data[0] << 8) | encoded_msg->data[1];
    const uint8_t *payload = encoded_msg->data + 2;
    size_t payload_len = encoded_msg->length - 2;

    switch (id) {

        case 992: { // 0x3E0: Coolant & Oil Temp
            if (payload_len >= 2 * sizeof(float)) {
                float coolant_temp, oil_temp;
                memcpy(&coolant_temp, payload, sizeof(float));
                memcpy(&oil_temp, payload + sizeof(float), sizeof(float));

                // Update coolant progress bar
                if (temp_bar && lv_obj_is_valid(temp_bar)) {
                    lv_bar_set_value(temp_bar, (int)coolant_temp, LV_ANIM_ON);
                }

                // Update coolant temp label
                if (coolant_temp_label && lv_obj_is_valid(coolant_temp_label)) {
                    int val = (int)(coolant_temp * 10);  // One decimal place
                    char buf[8];

                    buf[0] = '0' + (val / 1000) % 10;  // Hundreds
                    buf[1] = '0' + (val / 100) % 10;   // Tens
                    buf[2] = '0' + (val / 10) % 10;    // Ones
                    buf[3] = '.';
                    buf[4] = '0' + (val % 10);         // Tenths
                    buf[5] = '\0';                     // Null-terminate

                    int start = 0;
                    while (start < 3 && buf[start] == '0') start++;
                    if (start == 3) start--;  // Keep at least one digit

                    lv_label_set_text(coolant_temp_label, &buf[start]);
                    lv_obj_invalidate(coolant_temp_label);
                }

                // Update oil temp label
                if (oil_temp_label && lv_obj_is_valid(oil_temp_label)) {
                    int val = (int)(oil_temp * 10);
                    char buf[8];

                    buf[0] = '0' + (val / 1000) % 10;
                    buf[1] = '0' + (val / 100) % 10;
                    buf[2] = '0' + (val / 10) % 10;
                    buf[3] = '.';
                    buf[4] = '0' + (val % 10);
                    buf[5] = '\0';

                    int start = 0;
                    while (start < 3 && buf[start] == '0') start++;
                    if (start == 3) start--;

                    lv_label_set_text(oil_temp_label, &buf[start]);
                    lv_obj_invalidate(oil_temp_label);
                }
            }
            break;
        }



    case 1136: { // 0x470: Gear
        uint8_t gear = payload[7]; //might comment out
        static uint8_t last_gear = 0xFF;  // impossible initial value

        memcpy(&gear, payload, sizeof(uint8_t));

        printf("gear value at test.c: %d\n", gear);
        printf("Received CAN ID 0x470: gear=%02X\n", gear);

        /*
        if (gear == 0x00 && last_gear <= 0x02) {
            printf("Ignoring bounce to 0\n"); //slow af apparently
            break;
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

        printf("Setting label to: %s\n", gear_str);

        lv_label_set_text(gear_label, gear_str);

        break;
    }

        case 864: { // 0x360: RPM & Throttle //else if (last_gear >= 1 && last_gear <= 6) {
            if (payload_len >= sizeof(uint16_t) + sizeof(float)) {
                uint16_t rpm;
                float throttle;
                memcpy(&rpm, payload, sizeof(uint16_t));
                memcpy(&throttle, payload + sizeof(uint16_t), sizeof(float));

                // Limit RPM to max display value
                if (rpm > 11500) rpm = 11500;

                // Update RPM bar and label
                if (rpm_bar && lv_obj_is_valid(rpm_bar)) {
                    lv_bar_set_value(rpm_bar, rpm, LV_ANIM_OFF); //turn on if want animation to next value
                }
                if (rpm_label && lv_obj_is_valid(rpm_label)) {
                    lv_label_set_text_fmt(rpm_label, "%05d", rpm);
                }

                // Update throttle bar
                if (throttle_bar && lv_obj_is_valid(throttle_bar)) {
                    lv_bar_set_value(throttle_bar, (int)throttle, LV_ANIM_OFF); //turn on if want animation to next value
                }

                // Shift-up logic
                if (gear_label && lv_obj_is_valid(gear_label)) {
                    if (rpm >= 10800) { //to 11000
                        // Flash gear label red (RPM limiter alert)
                        static bool flash = false;
                        flash = !flash; // toggle each time this is called
                        lv_obj_set_style_bg_color(gear_label, flash ? lv_color_hex(0xFF0000) : lv_color_hex(0x000000), 0);
                    } else if (rpm >= 9000) {
                        // Shift-up warning (solid red background)
                        lv_obj_set_style_bg_color(gear_label, lv_color_hex(0x550000), 0);
                    } else {
                        // Normal background
                        lv_obj_set_style_bg_color(gear_label, lv_color_hex(0x000000), 0);
                    }
                }
            }
            break;
        }

    case 865: { // 0x361: Fuel & Oil Pressure
        if (payload_len >= 2 * sizeof(float)) {
            float fuel_pressure, oil_pressure;
            memcpy(&fuel_pressure, payload, sizeof(float));
            memcpy(&oil_pressure, payload + sizeof(float), sizeof(float));

            // Convert to integer * 10 for 1 decimal place, e.g. 12.3 -> 123
            int fuel_val = (int)(fuel_pressure * 10);
            int oil_val = (int)(oil_pressure * 10);

            if (fuel_pressure_label && lv_obj_is_valid(fuel_pressure_label)) {
                char buf[8]; // Enough for "xxx.x\0"
                // Format as xxx.x manually
                buf[0] = '0' + (fuel_val / 1000) % 10;  // Hundreds
                buf[1] = '0' + (fuel_val / 100) % 10;   // Tens
                buf[2] = '0' + (fuel_val / 10) % 10;    // Ones
                buf[3] = '.';                           // Decimal point
                buf[4] = '0' + (fuel_val % 10);        // Tenths
                buf[5] = '\0';

                // Handle leading zeros (remove if you want)
                // Or just shift to start at first non-zero digit
                int start = 0;
                while (start < 4 && buf[start] == '0') start++;
                if (start == 4) start--;  // Keep at least one digit before decimal

                lv_label_set_text(fuel_pressure_label, &buf[start]);
                lv_obj_invalidate(fuel_pressure_label);
            }

            if (oil_pressure_label && lv_obj_is_valid(oil_pressure_label)) {
                char buf[8];
                buf[0] = '0' + (oil_val / 1000) % 10;
                buf[1] = '0' + (oil_val / 100) % 10;
                buf[2] = '0' + (oil_val / 10) % 10;
                buf[3] = '.';
                buf[4] = '0' + (oil_val % 10);
                buf[5] = '\0';

                int start = 0;
                while (start < 4 && buf[start] == '0') start++;
                if (start == 4) start--;

                lv_label_set_text(oil_pressure_label, &buf[start]);
                lv_obj_invalidate(oil_pressure_label);
            }

            // Optional: update any progress bars here

        } else {
            printf("Invalid payload length for fuel/oil pressure: %d\n", payload_len);
        }
        break;
    }

        case 875: { // 0x36B: Brake Pressure
            if (payload_len >= sizeof(float)) {
                float brake_pressure;
                memcpy(&brake_pressure, payload, sizeof(float));

                // Clamp to valid bar range (0–100) to prevent UI issues
                if (brake_pressure < 0) brake_pressure = 0;
                if (brake_pressure > 100) brake_pressure = 100;

                if (brake_bar && lv_obj_is_valid(brake_bar)) {
                    lv_bar_set_value(brake_bar, (int)brake_pressure, LV_ANIM_OFF); //turn on if want animation to next value
                }
            }
            break;
        }

        case 880: { // 0x370: Vehicle Speed
            //printf("Speed payload bytes: %02X %02X %02X %02X\n", payload[0], payload[1], payload[2], payload[3]);
            if (payload_len >= sizeof(float)) {
                float speed_float;
                memcpy(&speed_float, payload, sizeof(float));
                int speed = (int)speed_float;

                printf("Speed: %03d\n", speed);

                if (speed_label && lv_obj_is_valid(speed_label)) {
                    // Build a 3-digit zero-padded string manually (e.g., "005", "120")
                    char buf[4];
                    buf[0] = '0' + (speed / 100) % 10;
                    buf[1] = '0' + (speed / 10) % 10;
                    buf[2] = '0' + speed % 10;
                    buf[3] = '\0';

                    lv_label_set_text(speed_label, buf);
                    lv_obj_invalidate(speed_label);
                }
            }
            break;
        }

    case 882: { // Battery voltage
        printf("Voltage payload bytes: %02X %02X %02X %02X\n", payload[0], payload[1], payload[2], payload[3]);
        if (payload_len >= sizeof(float)) {
            float voltage;
            memcpy(&voltage, payload, sizeof(float));
            printf("Voltage: %.2f\n", voltage);

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

                lv_label_set_text(battery_label, buf);
                lv_obj_invalidate(battery_label);  // refresh label
            }
        }
        break;
    }

        case 1001: { // 0x3E9: Lambda
            if (payload_len >= sizeof(float)) {
                float lambda;
                memcpy(&lambda, payload, sizeof(float));
                // Optional: update lambda display
            }
            break;
        }

        default: {
            // Unknown ID
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

void Display_Task(void *pvParameters) {
    encoded_message_t msg;
    while (1) {
        if (xQueueReceive(xECU, &msg, 0) == pdPASS) {
            if (lvgl_port_lock(-1)) {
                decode_and_dispatch(&msg);
                lvgl_port_unlock();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}