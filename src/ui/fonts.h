#ifndef EEZ_LVGL_UI_FONTS_H
#define EEZ_LVGL_UI_FONTS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern lv_font_t *ui_font_qweather_icons_32;
extern lv_font_t *ui_font_qweather_icons_64;
extern lv_font_t *ui_font_qweather_icons_128;
extern lv_font_t *ui_font_alibaba_pu_hui_ti_16;
extern lv_font_t *ui_font_alibaba_pu_hui_ti_24;
extern lv_font_t *ui_font_alibaba_pu_hui_ti_32;
extern lv_font_t *ui_font_alibaba_pu_hui_ti_64;

#ifndef EXT_FONT_DESC_T
#define EXT_FONT_DESC_T
typedef struct _ext_font_desc_t {
    const char *name;
    const void *font_ptr;
} ext_font_desc_t;
#endif

extern ext_font_desc_t fonts[];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_FONTS_H*/