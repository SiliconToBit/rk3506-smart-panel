#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_load_music_directory(lv_event_t * e);
extern void action_toggle_play_pause(lv_event_t * e);
extern void action_play_next(lv_event_t * e);
extern void action_play_prev(lv_event_t * e);
extern void action_update_album_art(lv_event_t * e);
extern void action_init_music_screen(lv_event_t * e);
extern void action_deinit_music_screen(lv_event_t * e);
extern void action_init_weather_screen(lv_event_t * e);
extern void action_deinit_weather_screen(lv_event_t * e);
extern void action_music_volume_changed(lv_event_t * e);
extern void action_music_progress_changed(lv_event_t * e);
extern void action_confirm_city_picker(lv_event_t * e);
extern void action_prov_changed(lv_event_t * e);
extern void action_city_changed(lv_event_t * e);
extern void action_setting_brightness_changed(lv_event_t * e);
extern void action_setting_volume_changed(lv_event_t * e);
extern void action_setting_wifi_state_changed(lv_event_t * e);
extern void action_init_wifi_setting_page(lv_event_t * e);
extern void action_confirm_wifi_password(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/