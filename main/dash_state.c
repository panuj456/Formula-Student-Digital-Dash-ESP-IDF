#include "dash_state.h"
#include <string.h>

dash_state_t g_dash_front;
dash_state_t g_dash_back;

void dash_state_init(void) {
    memset(&g_dash_front, 0, sizeof(g_dash_front));
    memset(&g_dash_back, 0, sizeof(g_dash_back));
    g_dash_back.dirty = false;
    g_dash_front.dirty = false;
    for (int i = 0; i < NUM_SHIFT_LEDS; ++i) {
        g_dash_back.shift_led_color[i] = lv_color_hex(0x222222);
        g_dash_front.shift_led_color[i] = lv_color_hex(0x222222);
    }

    g_dash_back.gear_bg_color = lv_color_hex(0x000000);
    g_dash_front.gear_bg_color = lv_color_hex(0x000000);
}
void dash_state_copy_back_to_front(void)
{
    // Assuming caller holds LVGL lock or other sync method.
    memcpy(&g_dash_front, &g_dash_back, sizeof(dash_state_t));
    g_dash_back.dirty = false;
}