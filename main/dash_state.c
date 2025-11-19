#include "dash_state.h"
#include <string.h>

dash_state_t g_dash_front;
dash_state_t g_dash_back;

void dash_state_init(void) {
    memset(&g_dash_front, 0, sizeof(g_dash_front));
    memset(&g_dash_back, 0, sizeof(g_dash_back));
    //dash_state_t g_dash_back = {0};
    //dash_state_t g_dash_front = {0};
}
void dash_state_copy_back_to_front(void)
{
    // Assuming caller holds LVGL lock or other sync method.
    memcpy(&g_dash_front, &g_dash_back, sizeof(dash_state_t));
    g_dash_back.dirty = false;
}