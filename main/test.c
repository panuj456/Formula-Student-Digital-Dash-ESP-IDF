#include "lvgl.h"
//#include "lv_meter.h"
//#include "lv_widgets.h"

lv_obj_t *canvas;

// Define the previous point
int16_t prev_x = -1;
int16_t prev_y = -1;

// Define the timeout value in milliseconds (e.g., 1000ms = 1 second)
#define TOUCH_TIMEOUT_MS 100

// Variables to track the last touch time
uint32_t last_touch_time = 0;

// Custom MAX macro
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Function to calculate rainbow color based on time
lv_color_t get_rainbow_color(uint32_t time_ms)
{
    // Define the period for a complete rainbow cycle (e.g., 5000ms = 5 seconds)
    uint32_t rainbow_period = 5000;

    // Calculate the hue value based on time
    uint16_t hue = (time_ms % rainbow_period) * 360 / rainbow_period;

    // Convert the hue to an RGB color
    return lv_color_hsv_to_rgb(hue, 100, 100);
}

static void dash_create2(lv_obj_t * parent); //arc
{
    lv_meter_scale_t * scale;
    lv_meter_indicator_t * indic;
    meter1 = create_meter_box(parent, "RPM", "Revenue: 63%", "Sales: 44%", "Costs: 58%");
    lv_obj_add_flag(lv_obj_get_parent(meter1), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    scale = lv_meter_add_scale(meter1);
    lv_meter_set_scale_range(meter1, scale, 0, 100, 270, 90);
    lv_meter_set_scale_ticks(meter1, scale, 0, 0, 0, lv_color_black());

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_values(&a, 20, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

    indic = lv_meter_add_arc(meter1, scale, 15, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_anim_set_exec_cb(&a, meter1_indic1_anim_cb);
    lv_anim_set_var(&a, indic);
    lv_anim_set_time(&a, 4100);
    lv_anim_set_playback_time(&a, 2700);
    lv_anim_start(&a);

    indic = lv_meter_add_arc(meter1, scale, 15, lv_palette_main(LV_PALETTE_RED), -20);
    lv_anim_set_exec_cb(&a, meter1_indic2_anim_cb);
    lv_anim_set_var(&a, indic);
    lv_anim_set_time(&a, 2600);
    lv_anim_set_playback_time(&a, 3200);
    a.user_data = indic;
    lv_anim_start(&a);

    indic = lv_meter_add_arc(meter1, scale, 15, lv_palette_main(LV_PALETTE_GREEN), -40);
    lv_anim_set_exec_cb(&a, meter1_indic3_anim_cb);
    lv_anim_set_var(&a, indic);
    lv_anim_set_time(&a, 2800);
    lv_anim_set_playback_time(&a, 1800);
    lv_anim_start(&a);
}

void draw_rectangle_on_canvas(int16_t x, int16_t y, uint16_t pressure)
{
    // Get the current time
    uint32_t current_time = lv_tick_get();

    // Calculate the time elapsed since the last touch
    uint32_t time_elapsed = current_time - last_touch_time;

    // If the time elapsed is greater than the timeout, consider it a new line
    if (time_elapsed > TOUCH_TIMEOUT_MS) {
        prev_x = -1;
        prev_y = -1;
    }

    // Define the minimum and maximum sizes of the rectangle
    uint16_t min_rect_size = 5;   // Adjust this value for the minimum size
    uint16_t max_rect_size = 1000;  // Adjust this value for the maximum size

    // Calculate the scaling factor based on pressure
    float scale = ((float)pressure / 320000.0) * (max_rect_size - min_rect_size);

    // Calculate the size of the rectangle based on pressure and the range of sizes
    uint16_t rect_size = min_rect_size + (uint16_t)scale;

    // Calculate the position of the rectangle based on touch coordinates
    int16_t rect_x = x - rect_size / 2;
    int16_t rect_y = y - rect_size / 2;

    // Get the rainbow color based on the current time
    lv_color_t rainbow_color = get_rainbow_color(current_time);

    // If there is a previous point, interpolate between them
    if (prev_x != -1 && prev_y != -1) {
        int16_t dx = x - prev_x;
        int16_t dy = y - prev_y;
        int16_t steps = MAX(abs(dx), abs(dy)); // Number of steps to interpolate

        // Interpolate between the points and draw rectangles with rainbow color
        for (int i = 0; i < steps; i++) {
            float t = (float)i / (float)steps;
            int16_t interp_x = prev_x + (int16_t)(dx * t);
            int16_t interp_y = prev_y + (int16_t)(dy * t);
            
            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.radius = 81;
            rect_dsc.bg_opa = LV_OPA_COVER;
            rect_dsc.bg_color = rainbow_color;
            lv_canvas_draw_rect(canvas, interp_x - rect_size / 2, interp_y - rect_size / 2, rect_size, rect_size, &rect_dsc);
        }
    }

    // Draw the final rectangle at the current touch point with rainbow color
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.radius = 81;
    rect_dsc.bg_opa = LV_OPA_COVER;
    rect_dsc.bg_color = rainbow_color;
    lv_canvas_draw_rect(canvas, rect_x, rect_y, rect_size, rect_size, &rect_dsc);

    // Refresh the canvas to show the drawn rectangle
    lv_obj_invalidate(canvas);

    // Update the previous point and last touch time
    prev_x = x;
    prev_y = y;
    last_touch_time = current_time;
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
    lv_canvas_fill_bg(canvas, lv_color_hex(0x078000), LV_OPA_COVER);
    
}

/*
// This demo UI is adapted from LVGL official example: https://docs.lvgl.io/master/examples.html#scatter-chart
void example_lvgl_demo_ui() // LVGL demo UI initialization function 
{
    lv_obj_t *scr = lv_scr_act();                                              // Get the current active screen 
    lv_obj_t *chart = lv_chart_create(scr);                                    // Create a chart object 
    lv_obj_set_size(chart, 200, 150);                                          // Set chart size 
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);                                // Center the chart on the screen 
    lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL); // Add draw event callback 
    lv_obj_set_style_line_width(chart, 0, LV_PART_ITEMS);                      // Remove chart lines  

    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER); // Set chart type to scatter 

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 5, 5, 5, 1, true, 30);  // Set X-axis ticks 
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 5, true, 50); // Set Y-axis ticks 

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, 200);  // Set X-axis range 
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000); // Set Y-axis range 

    lv_chart_set_point_count(chart, 50); // Set the number of points in the chart 

    lv_chart_series_t *ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y); // Add a series to the chart 
    for (int i = 0; i < 50; i++)
    {                                                                            // Add random points to the chart 
        lv_chart_set_next_value2(chart, ser, lv_rand(0, 200), lv_rand(0, 1000)); // Set X and Y values 
    }

    lv_timer_create(add_data, 100, chart); // Create a timer to add new data every 100ms 
}
*/