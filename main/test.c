#include "lvgl.h"

lv_obj_t *canvas;
static void dash_create(lv_obj_t * parent); //declaration

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

static void dash_create(lv_obj_t * parent);
    {
    lv_meter_scale_t * scale;
    lv_meter_indicator_t * indic;
    meter1 = create_meter_box(parent, "Monthly Target", "Revenue: 63%", "Sales: 44%", "Costs: 58%");
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

    meter2 = create_meter_box(parent, "Sessions", "Desktop: ", "Tablet: ", "Mobile: ");
    if(disp_size < DISP_LARGE) lv_obj_add_flag(lv_obj_get_parent(meter2), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    scale = lv_meter_add_scale(meter2);
    lv_meter_set_scale_range(meter2, scale, 0, 100, 360, 90);
    lv_meter_set_scale_ticks(meter2, scale, 0, 0, 0, lv_color_black());

    static lv_meter_indicator_t * meter2_indic[3];
    meter2_indic[0] = lv_meter_add_arc(meter2, scale, 20, lv_palette_main(LV_PALETTE_RED), -10);
    lv_meter_set_indicator_start_value(meter2, meter2_indic[0], 0);
    lv_meter_set_indicator_end_value(meter2, meter2_indic[0], 39);

    meter2_indic[1] = lv_meter_add_arc(meter2, scale, 30, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter2, meter2_indic[1], 40);
    lv_meter_set_indicator_end_value(meter2, meter2_indic[1], 69);

    meter2_indic[2] = lv_meter_add_arc(meter2, scale, 10, lv_palette_main(LV_PALETTE_GREEN), -20);
    lv_meter_set_indicator_start_value(meter2, meter2_indic[2], 70);
    lv_meter_set_indicator_end_value(meter2, meter2_indic[2], 99);

    meter2_timer = lv_timer_create(meter2_timer_cb, 100, meter2_indic);

    meter3 = create_meter_box(parent, "Network Speed", "Low speed", "Normal Speed", "High Speed");
    if(disp_size < DISP_LARGE) lv_obj_add_flag(lv_obj_get_parent(meter3), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    /*Add a special circle to the needle's pivot*/
    lv_obj_set_style_pad_hor(meter3, 10, 0);
    lv_obj_set_style_size(meter3, 10, LV_PART_INDICATOR);
    lv_obj_set_style_radius(meter3, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(meter3, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(meter3, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_INDICATOR);
    lv_obj_set_style_outline_color(meter3, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(meter3, 3, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(meter3, lv_palette_darken(LV_PALETTE_GREY, 1), LV_PART_TICKS);

    scale = lv_meter_add_scale(meter3);
    lv_meter_set_scale_range(meter3, scale, 10, 60, 220, 360 - 220);
    lv_meter_set_scale_ticks(meter3, scale, 21, 3, 17, lv_color_white());
    lv_meter_set_scale_major_ticks(meter3, scale, 4, 4, 22, lv_color_white(), 15);

    indic = lv_meter_add_arc(meter3, scale, 10, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter3, indic, 0);
    lv_meter_set_indicator_end_value(meter3, indic, 20);

    indic = lv_meter_add_scale_lines(meter3, scale, lv_palette_darken(LV_PALETTE_RED, 3), lv_palette_darken(LV_PALETTE_RED,
                                                                                                            3), true, 0);
    lv_meter_set_indicator_start_value(meter3, indic, 0);
    lv_meter_set_indicator_end_value(meter3, indic, 20);

    indic = lv_meter_add_arc(meter3, scale, 12, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter3, indic, 20);
    lv_meter_set_indicator_end_value(meter3, indic, 40);

    indic = lv_meter_add_scale_lines(meter3, scale, lv_palette_darken(LV_PALETTE_BLUE, 3),
                                     lv_palette_darken(LV_PALETTE_BLUE, 3), true, 0);
    lv_meter_set_indicator_start_value(meter3, indic, 20);
    lv_meter_set_indicator_end_value(meter3, indic, 40);

    indic = lv_meter_add_arc(meter3, scale, 10, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_meter_set_indicator_start_value(meter3, indic, 40);
    lv_meter_set_indicator_end_value(meter3, indic, 60);

    indic = lv_meter_add_scale_lines(meter3, scale, lv_palette_darken(LV_PALETTE_GREEN, 3),
                                     lv_palette_darken(LV_PALETTE_GREEN, 3), true, 0);
    lv_meter_set_indicator_start_value(meter3, indic, 40);
    lv_meter_set_indicator_end_value(meter3, indic, 60);

    indic = lv_meter_add_needle_line(meter3, scale, 4, lv_palette_darken(LV_PALETTE_GREY, 4), -25);

    lv_obj_t * mbps_label = lv_label_create(meter3);
    lv_label_set_text(mbps_label, "-");
    lv_obj_add_style(mbps_label, &style_title, 0);

    lv_obj_t * mbps_unit_label = lv_label_create(meter3);
    lv_label_set_text(mbps_unit_label, "Mbps");

    lv_anim_init(&a);
    lv_anim_set_values(&a, 10, 60);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, meter3_anim_cb);
    lv_anim_set_var(&a, indic);
    lv_anim_set_time(&a, 4100);
    lv_anim_set_playback_time(&a, 800);
    lv_anim_start(&a);

    lv_obj_update_layout(parent);
    if(disp_size == DISP_MEDIUM) {
        lv_obj_set_size(meter1, 200, 200);
        lv_obj_set_size(meter2, 200, 200);
        lv_obj_set_size(meter3, 200, 200);
    }
    else {
        lv_coord_t meter_w = lv_obj_get_width(meter1);
        lv_obj_set_height(meter1, meter_w);
        lv_obj_set_height(meter2, meter_w);
        lv_obj_set_height(meter3, meter_w);
    }

    lv_obj_align(mbps_label, LV_ALIGN_TOP_MID, 10, lv_pct(55));
    lv_obj_align_to(mbps_unit_label, mbps_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);
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
    lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_COVER);
    
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
    lv_obj_set_style_line_width(chart, 0, LV_PART_ITEMS);                      /* Remove chart lines  */

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