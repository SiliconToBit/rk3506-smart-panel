#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_P1 = 2,
    SCREEN_ID_P2 = 3,
    SCREEN_ID_MUSIC_PAGE = 4,
    SCREEN_ID_WEATHER_PAGE = 5,
    SCREEN_ID_SENSOR_PAGE = 6,
    SCREEN_ID_SETTING_PAGE = 7,
    SCREEN_ID_CPU_PAGE = 8,
    SCREEN_ID_IMAGE_PAGE = 9,
    SCREEN_ID_VIDEO_PAGE = 10,
    SCREEN_ID_CAMERA_PAGE = 11,
    _SCREEN_ID_LAST = 11
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *p1;
    lv_obj_t *p2;
    lv_obj_t *music_page;
    lv_obj_t *weather_page;
    lv_obj_t *sensor_page;
    lv_obj_t *setting_page;
    lv_obj_t *cpu_page;
    lv_obj_t *image_page;
    lv_obj_t *video_page;
    lv_obj_t *camera_page;
    lv_obj_t *main_time_label;
    lv_obj_t *setting_btn;
    lv_obj_t *weather_btn;
    lv_obj_t *sensor_btn;
    lv_obj_t *music_btn;
    lv_obj_t *image_btn;
    lv_obj_t *cpu_btn;
    lv_obj_t *video_btn;
    lv_obj_t *camera_btn;
    lv_obj_t *ai_btn;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *music_album_img;
    lv_obj_t *music_volume_slider;
    lv_obj_t *music_progress_slider;
    lv_obj_t *music_curr_time_label;
    lv_obj_t *music_total_time_label;
    lv_obj_t *music_title_label;
    lv_obj_t *music_lyrics_label;
    lv_obj_t *music_play_pause_imgbtn;
    lv_obj_t *music_list_panel;
    lv_obj_t *scroll_list_area;
    lv_obj_t *obj3;
    lv_obj_t *weather_time_panel;
    lv_obj_t *weather_temp_label;
    lv_obj_t *weather_wind_label;
    lv_obj_t *weather_text_label;
    lv_obj_t *weather_icon_label;
    lv_obj_t *time_location_panel;
    lv_obj_t *weather_time_label;
    lv_obj_t *weather_data_label;
    lv_obj_t *weather_city_label;
    lv_obj_t *weather_forecast_panel;
    lv_obj_t *weather_fcst_icon_0_label;
    lv_obj_t *weather_fcst_temp_0_label;
    lv_obj_t *weather_fcst_wind_0_label;
    lv_obj_t *weather_fcst_icon_1_label;
    lv_obj_t *weather_fcst_temp_1_label;
    lv_obj_t *weather_fcst_wind_1_label;
    lv_obj_t *weather_fcst_icon_2_label;
    lv_obj_t *weather_fcst_temp_2_label;
    lv_obj_t *weather_fcst_wind_2_label;
    lv_obj_t *sys_calendar;
    lv_obj_t *weather_city_picker_mask;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *weather_prov_roller;
    lv_obj_t *weather_city_roller;
    lv_obj_t *weather_county_roller;
    lv_obj_t *led0_btn;
    lv_obj_t *led1_btn;
    lv_obj_t *rgb_led_btn;
    lv_obj_t *buzzer_btn;
    lv_obj_t *dht11_temp_label;
    lv_obj_t *dht11_humi_label;
    lv_obj_t *mpu6050_temp_label;
    lv_obj_t *mpu6050_pitch_label;
    lv_obj_t *mpu6050_roll_label;
    lv_obj_t *mpu6050_yaw_label;
    lv_obj_t *obj6;
    lv_obj_t *left_menu_panel;
    lv_obj_t *setting_lightness_btn;
    lv_obj_t *setting_sound_btn;
    lv_obj_t *setting_wifi_btn;
    lv_obj_t *wifi_content_area;
    lv_obj_t *wifi_switch;
    lv_obj_t *connected_wifi_info_label;
    lv_obj_t *wifi_list_panel;
    lv_obj_t *lightness_content_area;
    lv_obj_t *setting_lightness_slider;
    lv_obj_t *sound_content_area;
    lv_obj_t *setting_sound_slider;
    lv_obj_t *setting_wifi_secret_picker_mask;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *setting_wifi_secret;
    lv_obj_t *setting_keyboard;
} objects_t;

extern objects_t objects;

void create_screen_main();
void delete_screen_main();
void tick_screen_main();

void create_screen_p1();
void delete_screen_p1();
void tick_screen_p1();

void create_screen_p2();
void delete_screen_p2();
void tick_screen_p2();

void create_screen_music_page();
void delete_screen_music_page();
void tick_screen_music_page();

void create_screen_weather_page();
void delete_screen_weather_page();
void tick_screen_weather_page();

void create_screen_sensor_page();
void delete_screen_sensor_page();
void tick_screen_sensor_page();

void create_screen_setting_page();
void delete_screen_setting_page();
void tick_screen_setting_page();

void create_screen_cpu_page();
void delete_screen_cpu_page();
void tick_screen_cpu_page();

void create_screen_image_page();
void delete_screen_image_page();
void tick_screen_image_page();

void create_screen_video_page();
void delete_screen_video_page();
void tick_screen_video_page();

void create_screen_camera_page();
void delete_screen_camera_page();
void tick_screen_camera_page();

void create_screen_by_id(enum ScreensEnum screenId);
void delete_screen_by_id(enum ScreensEnum screenId);
void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/