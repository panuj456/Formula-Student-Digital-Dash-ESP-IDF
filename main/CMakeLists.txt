idf_component_register(
    SRCS
        "main.c"
        "test.c"
        "waveshare_rgb_lcd_port.c"
        "lvgl_port.c"
        "lv_font_montserrat_36.c"
        "lv_font_montserrat_48.c"
        "lv_font_montserrat_72.c"
        "twai.c"
    INCLUDE_DIRS "."
    REQUIRES lvgl__lvgl
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_FONT_MONTSERRAT_36=1 LV_FONT_MONTSERRAT_48=1)


idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
