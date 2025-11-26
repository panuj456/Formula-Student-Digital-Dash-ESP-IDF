#ifndef DASH_STATE_H
#define DASH_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "lvgl.h"
#define DASH_TAG_LEN  20



typedef struct {
    // metadata
    volatile bool dirty;           // set by decode task
    uint32_t last_update_ms;       // ms

    // processed/normalised values (display-ready)
    uint16_t rpm;                  // integer RPM
    float throttle;                // percent (0..100)
    float lambda;                  // lambda value
    float speed;                   // km/h
    float brake;                   // e.g. bar or % (display units)
    float voltage;                 // volts
    uint8_t gear;                  // 0..N / special

    float fuel_pressure;           // kPa / display units
    float oil_pressure;            // kPa / display units
    float coolant_temp;            // Celsius
    float oil_temp;                // Celsius

    // wheel / tyre optional (if used)
    float wheelSlipFL, wheelSlipFR, wheelSlipRL, wheelSlipRR;
    float wheelSpeedFL, wheelSpeedFR, wheelSpeedRL, wheelSpeedRR;

    // precomputed UI colours (so Display only sets them)
    lv_color_t gear_bg_color;
    lv_color_t shift_led_color[NUM_SHIFT_LEDS];

    // any miscellaneous small values
    uint8_t other_bytes[8];

} dash_state_t;

extern dash_state_t g_dash_front;
extern dash_state_t g_dash_back;

void dash_state_init(void);
void dash_state_copy_back_to_front(void);

#endif // DASH_STATE_H