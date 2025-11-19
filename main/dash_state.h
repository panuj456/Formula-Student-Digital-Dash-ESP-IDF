#pragma once
#include <stdint.h>
#include <stdbool.h>
#define DASH_TAG_LEN  20


typedef struct {
    uint16_t rpm;
    uint16_t throttle;
    uint8_t gear;
    int16_t coolant_tenths;
    int16_t oil_tenths;
    int16_t fuel_tenths;
    int16_t oilp_tenths;
    uint16_t speed;
    uint32_t last_update_ms;
    bool dirty;
} dash_state_t;

extern dash_state_t g_dash_front;
extern dash_state_t g_dash_back;

void dash_state_init(void);
void dash_state_copy_back_to_front(void);