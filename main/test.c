#include "lvgl.h"
//#include "lv_meter.h" //not issue leave commented for now
//#include "lv_widgets.h" //not issue

lv_obj_t *canvas;
lv_obj_t * obj;

//Type Def
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

//static prototypes
static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
    const char * text3);
static void meter3_anim_cb(void * var, int32_t v);
static void meter1_anim_cb(void * var, int32_t v);

//statics
static lv_obj_t * meter3;
static lv_obj_t * meter2;
static lv_obj_t * meter1;
static lv_style_t style_bullet;
static lv_obj_t * meter1;
static disp_size_t disp_size;
static const lv_font_t * font_large;
static const lv_font_t * font_normal;
static lv_obj_t * tv;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static uint32_t session_desktop = 1000;
static uint32_t session_tablet = 1000;
static uint32_t session_mobile = 1000;

//LV_USE_DEMO_WIDGETS
//CONFIG_LV_USE_DEMO_WIDGETS

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

// cant use without meter objects 

void dash_create2(void)//(lv_obj_t * parent) //arc
{
    if(LV_HOR_RES <= 320) disp_size = DISP_SMALL;
    else if(LV_HOR_RES < 720) disp_size = DISP_MEDIUM;
    else disp_size = DISP_LARGE;

    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    lv_coord_t tab_h;
    if(disp_size == DISP_LARGE) {
        tab_h = 70;
#if LV_FONT_MONTSERRAT_24
        font_large     = &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_16
        font_normal    = &lv_font_montserrat_16;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }
    else if(disp_size == DISP_MEDIUM) {
        tab_h = 45;
#if LV_FONT_MONTSERRAT_20
        font_large     = &lv_font_montserrat_20;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_20 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_14
        font_normal    = &lv_font_montserrat_14;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_14 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }
    else {   /* disp_size == DISP_SMALL */
        tab_h = 45;
#if LV_FONT_MONTSERRAT_18
        font_large     = &lv_font_montserrat_18;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_12
        font_normal    = &lv_font_montserrat_12;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }
    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
    lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);
    if(disp_size == DISP_LARGE) {
        lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tv);
        lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES / 2, 0);
        lv_obj_t * logo = lv_img_create(tab_btns);
        LV_IMG_DECLARE(img_lvgl_logo);
        lv_img_set_src(logo, &img_lvgl_logo);
        lv_obj_align(logo, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 2 + 25, 0);

        lv_obj_t * label = lv_label_create(tab_btns);
        lv_obj_add_style(label, &style_title, 0);
        lv_label_set_text(label, "BCU RACING");
        lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);

        label = lv_label_create(tab_btns);
        lv_label_set_text(label, "Digital Dash Currently Demo");
        lv_obj_add_style(label, &style_text_muted, 0);
        lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);
    }
    lv_obj_t * tablelabel = lv_tabview_add_tab(tv, "Analytics");

    lv_obj_set_flex_flow(tablelabel, LV_FLEX_FLOW_ROW_WRAP);
    
    //RPM
    lv_meter_scale_t * scale2;
    lv_meter_indicator_t * indic2;
    lv_anim_t a2;
    //lv_meter_t * meter1 = (lv_meter_t *)obj;
    meter1 = create_meter_box(tablelabel, "RPM", "", "", "");
    if(disp_size < DISP_LARGE) lv_obj_add_flag(lv_obj_get_parent(meter1), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    /*Add a special circle to the needle's pivot*/
    lv_obj_set_style_pad_hor(meter1, 10, 0);
    lv_obj_set_style_size(meter1, 10, LV_PART_INDICATOR);
    lv_obj_set_style_radius(meter1, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(meter1, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(meter1, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_INDICATOR);
    lv_obj_set_style_outline_color(meter1, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(meter1, 3, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(meter1, lv_palette_darken(LV_PALETTE_GREY, 1), LV_PART_TICKS);

    scale2 = lv_meter_add_scale(meter1);
    lv_meter_set_scale_range(meter1, scale2, 10, 60, 220, 360 - 220);
    lv_meter_set_scale_ticks(meter1, scale2, 21, 3, 17, lv_color_white());
    lv_meter_set_scale_major_ticks(meter1, scale2, 4, 4, 22, lv_color_white(), 15);

    indic2 = lv_meter_add_arc(meter1, scale2, 10, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 0);
    lv_meter_set_indicator_end_value(meter1, indic2, 20);

    indic2 = lv_meter_add_scale_lines(meter1, scale2, lv_palette_darken(LV_PALETTE_LIGHT_BLUE, 3), lv_palette_darken(LV_PALETTE_LIGHT_BLUE,
                                                                                                            3), true, 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 0);
    lv_meter_set_indicator_end_value(meter1, indic2, 20);

    indic2 = lv_meter_add_arc(meter1, scale2, 12, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 20);
    lv_meter_set_indicator_end_value(meter1, indic2, 40);

    indic2 = lv_meter_add_scale_lines(meter1, scale2, lv_palette_darken(LV_PALETTE_BLUE, 3),
                                     lv_palette_darken(LV_PALETTE_BLUE, 3), true, 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 20);
    lv_meter_set_indicator_end_value(meter1, indic2, 40);

    indic2 = lv_meter_add_arc(meter1, scale2, 10, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 40);
    lv_meter_set_indicator_end_value(meter1, indic2, 60);

    indic2 = lv_meter_add_scale_lines(meter1, scale2, lv_palette_darken(LV_PALETTE_GREEN, 3),
                                     lv_palette_darken(LV_PALETTE_GREEN, 3), true, 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 40);
    lv_meter_set_indicator_end_value(meter1, indic2, 60);

    indic2 = lv_meter_add_arc(meter1, scale2, 10, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 60);
    lv_meter_set_indicator_end_value(meter1, indic2, 80);

    indic2 = lv_meter_add_scale_lines(meter1, scale2, lv_palette_darken(LV_PALETTE_RED, 3),
                                     lv_palette_darken(LV_PALETTE_RED, 3), true, 0);
    lv_meter_set_indicator_start_value(meter1, indic2, 60);
    lv_meter_set_indicator_end_value(meter1, indic2, 80);

    indic2 = lv_meter_add_needle_line(meter1, scale2, 4, lv_palette_darken(LV_PALETTE_GREY, 4), -25);
    
    lv_obj_t * mbps_label2 = lv_label_create(meter1);
    lv_label_set_text(mbps_label2, "-");
    lv_obj_add_style(mbps_label2, &style_title, 0);

    lv_obj_t * mbps_unit_label2 = lv_label_create(meter3);
    lv_label_set_text(mbps_unit_label2, "RPM");
    
    lv_anim_init(&a2);
    lv_anim_set_values(&a2, 10, 80);
    lv_anim_set_repeat_count(&a2, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a2, meter1_anim_cb);
    lv_anim_set_var(&a2, indic2);
    lv_anim_set_time(&a2, 4100);
    lv_anim_set_playback_time(&a2, 800);
    lv_anim_start(&a2);

    //gear screen here pls
    meter2 = create_meter_box(tablelabel, "GEAR", "", "", "");
    
    //Vehicle Speed
    meter3 = create_meter_box(tablelabel, "KM/H", "", "", "");
    lv_meter_scale_t * scale;
    lv_meter_indicator_t * indic;
    lv_anim_t a;
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
    lv_label_set_text(mbps_unit_label, "");
    
    lv_anim_init(&a);
    lv_anim_set_values(&a, 10, 60);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, meter3_anim_cb);
    lv_anim_set_var(&a, indic);
    lv_anim_set_time(&a, 4100);
    lv_anim_set_playback_time(&a, 800);
    lv_anim_start(&a);
}


static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
    const char * text3)
{
lv_obj_t * cont = lv_obj_create(parent);
lv_obj_set_height(cont, LV_SIZE_CONTENT);
lv_obj_set_flex_grow(cont, 1);

lv_obj_t * title_label = lv_label_create(cont);
lv_label_set_text(title_label, title);
lv_obj_add_style(title_label, &style_title, 0);

lv_obj_t * meter = lv_meter_create(cont);
lv_obj_remove_style(meter, NULL, LV_PART_MAIN);
lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);
lv_obj_set_width(meter, LV_PCT(100));

lv_obj_t * bullet1 = lv_obj_create(cont);
lv_obj_set_size(bullet1, 13, 13);
lv_obj_remove_style(bullet1, NULL, LV_PART_SCROLLBAR);
lv_obj_add_style(bullet1, &style_bullet, 0);
lv_obj_set_style_bg_color(bullet1, lv_palette_main(LV_PALETTE_RED), 0);
lv_obj_t * label1 = lv_label_create(cont);
lv_label_set_text(label1, text1);

lv_obj_t * bullet2 = lv_obj_create(cont);
lv_obj_set_size(bullet2, 13, 13);
lv_obj_remove_style(bullet2, NULL, LV_PART_SCROLLBAR);
lv_obj_add_style(bullet2, &style_bullet, 0);
lv_obj_set_style_bg_color(bullet2, lv_palette_main(LV_PALETTE_BLUE), 0);
lv_obj_t * label2 = lv_label_create(cont);
lv_label_set_text(label2, text2);

lv_obj_t * bullet3 = lv_obj_create(cont);
lv_obj_set_size(bullet3, 13, 13);
lv_obj_remove_style(bullet3,  NULL, LV_PART_SCROLLBAR);
lv_obj_add_style(bullet3, &style_bullet, 0);
lv_obj_set_style_bg_color(bullet3, lv_palette_main(LV_PALETTE_GREEN), 0);
lv_obj_t * label3 = lv_label_create(cont);
lv_label_set_text(label3, text3);

if(disp_size == DISP_MEDIUM) {
static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(8), LV_GRID_TEMPLATE_LAST};
static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 4, LV_GRID_ALIGN_START, 0, 1);
lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 3);
lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);
lv_obj_set_grid_cell(bullet3, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 4, 1);
lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 3, 1);
lv_obj_set_grid_cell(label3, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 4, 1);
}
else {
static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);
lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 1, 1);
lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
lv_obj_set_grid_cell(bullet3, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 2, 1);
lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 3, 1);
lv_obj_set_grid_cell(label3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 4, 1);
}
return meter;
}

static void meter1_anim_cb(void * var, int32_t v)
{
    lv_meter_set_indicator_value(meter1, var, v);

    lv_obj_t * label = lv_obj_get_child(meter1, 0);
    lv_label_set_text_fmt(label, "%"LV_PRId32, v);
}

static void meter3_anim_cb(void * var, int32_t v)
{
    lv_meter_set_indicator_value(meter3, var, v);

    lv_obj_t * label = lv_obj_get_child(meter3, 0);
    lv_label_set_text_fmt(label, "%"LV_PRId32, v);
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
    lv_canvas_fill_bg(canvas, lv_color_hex(0x078080), LV_OPA_COVER);
    
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