#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

static const char *screen_names[] = { "Main", "P1", "P2", "MusicPage", "WeatherPage", "SensorPage", "SettingPage", "CpuPage", "ImagePage", "VideoPage", "CameraPage" };
static const char *object_names[] = { "main", "p1", "p2", "music_page", "weather_page", "sensor_page", "setting_page", "cpu_page", "image_page", "video_page", "camera_page", "main_time_label", "setting_btn", "weather_btn", "sensor_btn", "music_btn", "image_btn", "cpu_btn", "video_btn", "camera_btn", "ai_btn", "obj0", "obj1", "obj2", "music_album_img", "music_volume_slider", "music_progress_slider", "music_curr_time_label", "music_total_time_label", "music_title_label", "music_lyrics_label", "music_play_pause_imgbtn", "music_list_panel", "scroll_list_area", "obj3", "weather_time_panel", "weather_temp_label", "weather_wind_label", "weather_text_label", "weather_icon_label", "time_location_panel", "weather_time_label", "weather_data_label", "weather_city_label", "weather_forecast_panel", "weather_fcst_icon_0_label", "weather_fcst_temp_0_label", "weather_fcst_wind_0_label", "weather_fcst_icon_1_label", "weather_fcst_temp_1_label", "weather_fcst_wind_1_label", "weather_fcst_icon_2_label", "weather_fcst_temp_2_label", "weather_fcst_wind_2_label", "sys_calendar", "weather_city_picker_mask", "obj4", "obj5", "weather_prov_roller", "weather_city_roller", "weather_county_roller", "led0_btn", "led1_btn", "rgb_led_btn", "buzzer_btn", "dht11_temp_label", "dht11_humi_label", "mpu6050_temp_label", "mpu6050_pitch_label", "mpu6050_roll_label", "mpu6050_yaw_label", "obj6", "left_menu_panel", "setting_lightness_btn", "setting_sound_btn", "setting_wifi_btn", "wifi_content_area", "wifi_switch", "connected_wifi_info_label", "wifi_list_panel", "lightness_content_area", "setting_lightness_slider", "sound_content_area", "setting_sound_slider", "setting_wifi_secret_picker_mask", "obj7", "obj8", "setting_wifi_secret", "setting_keyboard" };

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_main_main(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_p1_p1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_p1_setting_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 0, 0, e);
    }
}

static void event_handler_cb_p1_weather_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 2, 0, e);
    }
}

static void event_handler_cb_p1_sensor_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
}

static void event_handler_cb_p1_music_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 4, 0, e);
    }
}

static void event_handler_cb_p1_image_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_p1_cpu_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 6, 0, e);
    }
}

static void event_handler_cb_p1_video_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 7, 0, e);
    }
}

static void event_handler_cb_p1_camera_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

static void event_handler_cb_p2_p2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_music_page_music_page(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
    if (event == LV_EVENT_SCREEN_LOADED) {
        e->user_data = (void *)0;
        action_init_music_screen(e);
    }
    if (event == LV_EVENT_SCREEN_UNLOADED) {
        e->user_data = (void *)0;
        action_deinit_music_screen(e);
    }
}

static void event_handler_cb_music_page_obj0(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_play_next(e);
    }
}

static void event_handler_cb_music_page_obj1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_play_prev(e);
    }
}

static void event_handler_cb_music_page_obj2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_load_music_directory(e);
    }
}

static void event_handler_cb_music_page_music_album_img(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_update_album_art(e);
    }
}

static void event_handler_cb_music_page_music_volume_slider(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_music_volume_changed(e);
    }
}

static void event_handler_cb_music_page_music_progress_slider(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_music_progress_changed(e);
    }
}

static void event_handler_cb_music_page_music_play_pause_imgbtn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_toggle_play_pause(e);
    }
}

static void event_handler_cb_music_page_obj3(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 15, 0, e);
    }
}

static void event_handler_cb_weather_page_weather_page(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
    if (event == LV_EVENT_SCREEN_LOADED) {
        e->user_data = (void *)0;
        action_init_weather_screen(e);
    }
    if (event == LV_EVENT_SCREEN_UNLOADED) {
        e->user_data = (void *)0;
        action_deinit_weather_screen(e);
    }
}

static void event_handler_cb_weather_page_weather_city_label(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_weather_page_obj4(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 32, 0, e);
    }
}

static void event_handler_cb_weather_page_obj5(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_confirm_city_picker(e);
    }
}

static void event_handler_cb_weather_page_weather_prov_roller(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_prov_changed(e);
    }
}

static void event_handler_cb_weather_page_weather_city_roller(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_city_changed(e);
    }
}

static void event_handler_cb_sensor_page_sensor_page(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_setting_page_obj6(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
}

static void event_handler_cb_setting_page_setting_lightness_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 6, 0, e);
    }
}

static void event_handler_cb_setting_page_setting_sound_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 9, 0, e);
    }
}

static void event_handler_cb_setting_page_setting_wifi_btn(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 12, 0, e);
    }
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_init_wifi_setting_page(e);
    }
}

static void event_handler_cb_setting_page_wifi_switch(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_setting_wifi_state_changed(e);
    }
}

static void event_handler_cb_setting_page_setting_lightness_slider(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_setting_brightness_changed(e);
    }
}

static void event_handler_cb_setting_page_setting_sound_slider(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        action_setting_volume_changed(e);
    }
}

static void event_handler_cb_setting_page_obj7(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 30, 0, e);
    }
}

static void event_handler_cb_setting_page_obj8(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        action_confirm_wifi_password(e);
    }
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 32, 0, e);
    }
}

static void event_handler_cb_setting_page_setting_wifi_secret(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_CLICKED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 36, 0, e);
    }
}

static void event_handler_cb_setting_page_setting_keyboard(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_READY) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 37, 0, e);
    }
}

static void event_handler_cb_image_page_image_page(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_video_page_video_page(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_camera_page_camera_page(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_GESTURE) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

//
// Screens
//

void create_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_main_main, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            // main_time_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_time_label = obj;
            lv_obj_set_pos(obj, 338, 47);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_64, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "12:12");
        }
    }
    
    tick_screen_main();
}

void delete_screen_main() {
    lv_obj_del(objects.main);
    objects.main = 0;
    objects.main_time_label = 0;
    deletePageFlowState(0);
}

void tick_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
}

void create_screen_p1() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.p1 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_p1_p1, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            // setting_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.setting_btn = obj;
            lv_obj_set_pos(obj, 63, 70);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_setting_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_setting, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weather_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.weather_btn = obj;
            lv_obj_set_pos(obj, 267, 70);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_weather_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_weather, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00f0ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // sensor_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.sensor_btn = obj;
            lv_obj_set_pos(obj, 676, 70);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_sensor_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_sensor, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2a632c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // music_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.music_btn = obj;
            lv_obj_set_pos(obj, 267, 284);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_music_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_music, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // image_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.image_btn = obj;
            lv_obj_set_pos(obj, 63, 284);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_image_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_image, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff237cff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // cpu_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.cpu_btn = obj;
            lv_obj_set_pos(obj, 472, 70);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_cpu_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_cpu, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffc5c5c5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // video_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.video_btn = obj;
            lv_obj_set_pos(obj, 472, 284);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_video_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_video, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff5f5f5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // camera_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.camera_btn = obj;
            lv_obj_set_pos(obj, 676, 284);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_event_cb(obj, event_handler_cb_p1_camera_btn, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_img_src(obj, &img_camera, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_p1();
}

void delete_screen_p1() {
    lv_obj_del(objects.p1);
    objects.p1 = 0;
    objects.setting_btn = 0;
    objects.weather_btn = 0;
    objects.sensor_btn = 0;
    objects.music_btn = 0;
    objects.image_btn = 0;
    objects.cpu_btn = 0;
    objects.video_btn = 0;
    objects.camera_btn = 0;
    deletePageFlowState(1);
}

void tick_screen_p1() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
}

void create_screen_p2() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.p2 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_p2_p2, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ai_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.ai_btn = obj;
            lv_obj_set_pos(obj, 63, 70);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_set_style_bg_img_src(obj, &img_doubao, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_p2();
}

void delete_screen_p2() {
    lv_obj_del(objects.p2);
    objects.p2 = 0;
    objects.ai_btn = 0;
    deletePageFlowState(2);
}

void tick_screen_p2() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
}

void create_screen_music_page() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.music_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_music_page_music_page, LV_EVENT_ALL, flowState);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffb4b4b4), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 649, 336);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_next);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_obj0, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 415, 336);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_pre);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_obj1, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 766, 336);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_list);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_obj2, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            // music_album_img
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.music_album_img = obj;
            lv_obj_set_pos(obj, 49, 112);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_music_image);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_music_album_img, LV_EVENT_ALL, flowState);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 415, 240);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_sound);
        }
        {
            // music_volume_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.music_volume_slider = obj;
            lv_obj_set_pos(obj, 499, 262);
            lv_obj_set_size(obj, 279, 10);
            lv_slider_set_value(obj, 50, LV_ANIM_OFF);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_music_volume_slider, LV_EVENT_ALL, flowState);
        }
        {
            // music_progress_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.music_progress_slider = obj;
            lv_obj_set_pos(obj, 415, 442);
            lv_obj_set_size(obj, 415, 10);
            lv_slider_set_value(obj, 50, LV_ANIM_OFF);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_music_progress_slider, LV_EVENT_ALL, flowState);
        }
        {
            // music_curr_time_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.music_curr_time_label = obj;
            lv_obj_set_pos(obj, 415, 410);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00");
        }
        {
            // music_total_time_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.music_total_time_label = obj;
            lv_obj_set_pos(obj, 783, 410);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "04:11");
        }
        {
            // music_title_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.music_title_label = obj;
            lv_obj_set_pos(obj, 559, 34);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "歌曲名字");
        }
        {
            // music_lyrics_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.music_lyrics_label = obj;
            lv_obj_set_pos(obj, 494, 101);
            lv_obj_set_size(obj, 289, 112);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "歌词嘎嘎减肥的是快乐");
        }
        {
            // music_play_pause_imgbtn
            lv_obj_t *obj = lv_imgbtn_create(parent_obj);
            objects.music_play_pause_imgbtn = obj;
            lv_obj_set_pos(obj, 532, 336);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, 64);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, &img_play, NULL);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &img_pause, NULL);
            lv_obj_add_event_cb(obj, event_handler_cb_music_page_music_play_pause_imgbtn, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        }
        {
            // music_list_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.music_list_panel = obj;
            lv_obj_set_pos(obj, 854, 0);
            lv_obj_set_size(obj, 250, LV_PCT(100));
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_opa(obj, 220, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // scroll_list_area
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.scroll_list_area = obj;
                    lv_obj_set_pos(obj, -22, 50);
                    lv_obj_set_size(obj, 250, 394);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.obj3 = obj;
                    lv_obj_set_pos(obj, 22, -22);
                    lv_obj_set_size(obj, 64, 64);
                    lv_obj_add_event_cb(obj, event_handler_cb_music_page_obj3, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_bg_img_src(obj, &img_off, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }
    
    tick_screen_music_page();
}

void delete_screen_music_page() {
    lv_obj_del(objects.music_page);
    objects.music_page = 0;
    objects.obj0 = 0;
    objects.obj1 = 0;
    objects.obj2 = 0;
    objects.music_album_img = 0;
    objects.music_volume_slider = 0;
    objects.music_progress_slider = 0;
    objects.music_curr_time_label = 0;
    objects.music_total_time_label = 0;
    objects.music_title_label = 0;
    objects.music_lyrics_label = 0;
    objects.music_play_pause_imgbtn = 0;
    objects.music_list_panel = 0;
    objects.scroll_list_area = 0;
    objects.obj3 = 0;
    deletePageFlowState(3);
}

void tick_screen_music_page() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
}

void create_screen_weather_page() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.weather_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_weather_page_weather_page, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            // weather_time_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weather_time_panel = obj;
            lv_obj_set_pos(obj, 20, 20);
            lv_obj_set_size(obj, 400, 200);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 163, 16);
                    lv_obj_set_size(obj, 192, 124);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // weather_temp_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_temp_label = obj;
                            lv_obj_set_pos(obj, 222, 12);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "26℃");
                        }
                        {
                            // weather_wind_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_wind_label = obj;
                            lv_obj_set_pos(obj, 200, 62);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "东风二级");
                        }
                        {
                            // weather_text_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_text_label = obj;
                            lv_obj_set_pos(obj, 152, 111);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "大暴雨到特大暴雨");
                        }
                    }
                }
                {
                    // weather_icon_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weather_icon_label = obj;
                    lv_obj_set_pos(obj, LV_PCT(1), LV_PCT(1));
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_qweather_icons_128, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            // time_location_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.time_location_panel = obj;
            lv_obj_set_pos(obj, 445, 20);
            lv_obj_set_size(obj, 400, 200);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 72, -17);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // weather_time_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_time_label = obj;
                            lv_obj_set_pos(obj, 83, -19);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_64, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "12:00");
                        }
                        {
                            // weather_data_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_data_label = obj;
                            lv_obj_set_pos(obj, 67, 78);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "2025-12-11 星期四");
                        }
                        {
                            // weather_city_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_city_label = obj;
                            lv_obj_set_pos(obj, 109, 120);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_add_event_cb(obj, event_handler_cb_weather_page_weather_city_label, LV_EVENT_ALL, flowState);
                            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "广州番禺");
                        }
                    }
                }
            }
        }
        {
            // weather_forecast_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weather_forecast_panel = obj;
            lv_obj_set_pos(obj, 20, 240);
            lv_obj_set_size(obj, 452, 230);
            lv_obj_set_scroll_dir(obj, LV_DIR_HOR);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -16, -15);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "3日天气预报");
                }
                {
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    lv_obj_set_pos(obj, -21, 30);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(1));
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 400, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, -11, 39);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 10, 5);
                            lv_obj_set_size(obj, 80, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "今天");
                        }
                        {
                            // weather_fcst_icon_0_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_icon_0_label = obj;
                            lv_obj_set_pos(obj, 139, 8);
                            lv_obj_set_size(obj, 50, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_qweather_icons_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "");
                        }
                        {
                            // weather_fcst_temp_0_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_temp_0_label = obj;
                            lv_obj_set_pos(obj, 198, 3);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "24℃-26℃");
                        }
                        {
                            // weather_fcst_wind_0_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_wind_0_label = obj;
                            lv_obj_set_pos(obj, 440, 3);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "南风");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, -11, 93);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -11, 93);
                            lv_obj_set_size(obj, 80, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "明天");
                        }
                        {
                            // weather_fcst_icon_1_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_icon_1_label = obj;
                            lv_obj_set_pos(obj, 91, 98);
                            lv_obj_set_size(obj, 50, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_qweather_icons_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "");
                        }
                        {
                            // weather_fcst_temp_1_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_temp_1_label = obj;
                            lv_obj_set_pos(obj, 162, 93);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "24℃-26℃");
                        }
                        {
                            // weather_fcst_wind_1_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_wind_1_label = obj;
                            lv_obj_set_pos(obj, 354, 93);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "南风");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, -11, 152);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -11, 151);
                            lv_obj_set_size(obj, 80, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "后天");
                        }
                        {
                            // weather_fcst_icon_2_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_icon_2_label = obj;
                            lv_obj_set_pos(obj, 92, 156);
                            lv_obj_set_size(obj, 50, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_qweather_icons_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "");
                        }
                        {
                            // weather_fcst_temp_2_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_temp_2_label = obj;
                            lv_obj_set_pos(obj, 162, 146);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "24℃-26℃");
                        }
                        {
                            // weather_fcst_wind_2_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.weather_fcst_wind_2_label = obj;
                            lv_obj_set_pos(obj, 355, 147);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "南风");
                        }
                    }
                }
            }
        }
        {
            // sys_calendar
            lv_obj_t *obj = lv_calendar_create(parent_obj);
            objects.sys_calendar = obj;
            lv_obj_set_pos(obj, 487, 240);
            lv_obj_set_size(obj, 358, 230);
            lv_calendar_header_arrow_create(obj);
            lv_calendar_set_today_date(obj, 2022, 11, 1);
            lv_calendar_set_showed_date(obj, 2022, 11);
        }
        {
            // weather_city_picker_mask
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weather_city_picker_mask = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_opa(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.obj4 = obj;
                    lv_obj_set_pos(obj, 239, 328);
                    lv_obj_set_size(obj, 100, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_weather_page_obj4, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "取消");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.obj5 = obj;
                    lv_obj_set_pos(obj, 492, 328);
                    lv_obj_set_size(obj, 100, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_weather_page_obj5, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "确定");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 81, 31);
                    lv_obj_set_size(obj, LV_PCT(80), LV_PCT(60));
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // weather_prov_roller
                            lv_obj_t *obj = lv_roller_create(parent_obj);
                            objects.weather_prov_roller = obj;
                            lv_obj_set_pos(obj, 96, 57);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, 100);
                            lv_roller_set_options(obj, "Option 1\nOption 2\nOption 3", LV_ROLLER_MODE_NORMAL);
                            lv_obj_add_event_cb(obj, event_handler_cb_weather_page_weather_prov_roller, LV_EVENT_ALL, flowState);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            // weather_city_roller
                            lv_obj_t *obj = lv_roller_create(parent_obj);
                            objects.weather_city_roller = obj;
                            lv_obj_set_pos(obj, 273, 58);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, 100);
                            lv_roller_set_options(obj, "Option 1\nOption 2\nOption 3", LV_ROLLER_MODE_NORMAL);
                            lv_obj_add_event_cb(obj, event_handler_cb_weather_page_weather_city_roller, LV_EVENT_ALL, flowState);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            // weather_county_roller
                            lv_obj_t *obj = lv_roller_create(parent_obj);
                            objects.weather_county_roller = obj;
                            lv_obj_set_pos(obj, 450, 57);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, 100);
                            lv_roller_set_options(obj, "Option 1\nOption 2\nOption 3", LV_ROLLER_MODE_NORMAL);
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_weather_page();
}

void delete_screen_weather_page() {
    lv_obj_del(objects.weather_page);
    objects.weather_page = 0;
    objects.weather_time_panel = 0;
    objects.weather_temp_label = 0;
    objects.weather_wind_label = 0;
    objects.weather_text_label = 0;
    objects.weather_icon_label = 0;
    objects.time_location_panel = 0;
    objects.weather_time_label = 0;
    objects.weather_data_label = 0;
    objects.weather_city_label = 0;
    objects.weather_forecast_panel = 0;
    objects.weather_fcst_icon_0_label = 0;
    objects.weather_fcst_temp_0_label = 0;
    objects.weather_fcst_wind_0_label = 0;
    objects.weather_fcst_icon_1_label = 0;
    objects.weather_fcst_temp_1_label = 0;
    objects.weather_fcst_wind_1_label = 0;
    objects.weather_fcst_icon_2_label = 0;
    objects.weather_fcst_temp_2_label = 0;
    objects.weather_fcst_wind_2_label = 0;
    objects.sys_calendar = 0;
    objects.weather_city_picker_mask = 0;
    objects.obj4 = 0;
    objects.obj5 = 0;
    objects.weather_prov_roller = 0;
    objects.weather_city_roller = 0;
    objects.weather_county_roller = 0;
    deletePageFlowState(4);
}

void tick_screen_weather_page() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
}

void create_screen_sensor_page() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.sensor_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_sensor_page_sensor_page, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            // led0_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.led0_btn = obj;
            lv_obj_set_pos(obj, 34, 35);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_img_src(obj, &img_led, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffff0000), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 45);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "LED0");
                }
            }
        }
        {
            // led1_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.led1_btn = obj;
            lv_obj_set_pos(obj, 248, 35);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_img_src(obj, &img_led, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffa600ff), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 45);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "LED1");
                }
            }
        }
        {
            // rgb_led_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.rgb_led_btn = obj;
            lv_obj_set_pos(obj, 465, 35);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_img_src(obj, &img_led, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffa600ff), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 45);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "RGB LED");
                }
            }
        }
        {
            // buzzer_btn
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.buzzer_btn = obj;
            lv_obj_set_pos(obj, 692, 35);
            lv_obj_set_size(obj, 128, 128);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_src(obj, &img_buzzer, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffa600ff), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 45);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Buzzer");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 34, 248);
            lv_obj_set_size(obj, 205, 205);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -2, 58);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "温度：20℃");
                }
                {
                    // dht11_temp_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dht11_temp_label = obj;
                    lv_obj_set_pos(obj, 29, -6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "DHT11");
                }
                {
                    // dht11_humi_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dht11_humi_label = obj;
                    lv_obj_set_pos(obj, 10, 121);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "湿度：5%");
                }
            }
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 315, 248);
            lv_obj_set_size(obj, 355, 205);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // mpu6050_temp_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.mpu6050_temp_label = obj;
                    lv_obj_set_pos(obj, 69, 146);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "TEMP:+000.04");
                }
                {
                    // mpu6050_pitch_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.mpu6050_pitch_label = obj;
                    lv_obj_set_pos(obj, 66, 40);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "PITCH:+000.04");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 81, -6);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "MPU6050");
                }
                {
                    // mpu6050_roll_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.mpu6050_roll_label = obj;
                    lv_obj_set_pos(obj, 72, 76);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "ROLL:+000.04");
                }
                {
                    // mpu6050_yaw_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.mpu6050_yaw_label = obj;
                    lv_obj_set_pos(obj, 73, 111);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "YAW:+000.04");
                }
            }
        }
    }
    
    tick_screen_sensor_page();
}

void delete_screen_sensor_page() {
    lv_obj_del(objects.sensor_page);
    objects.sensor_page = 0;
    objects.led0_btn = 0;
    objects.led1_btn = 0;
    objects.rgb_led_btn = 0;
    objects.buzzer_btn = 0;
    objects.dht11_temp_label = 0;
    objects.dht11_humi_label = 0;
    objects.mpu6050_temp_label = 0;
    objects.mpu6050_pitch_label = 0;
    objects.mpu6050_roll_label = 0;
    objects.mpu6050_yaw_label = 0;
    deletePageFlowState(5);
}

void tick_screen_sensor_page() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
}

void create_screen_setting_page() {
    void *flowState = getFlowState(0, 6);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.setting_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 395, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "设置");
                }
                {
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.obj6 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 100, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_obj6, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_align(obj, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "返回");
                        }
                    }
                }
            }
        }
        {
            // left_menu_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.left_menu_panel = obj;
            lv_obj_set_pos(obj, 24, 67);
            lv_obj_set_size(obj, 204, 387);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // setting_lightness_btn
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.setting_lightness_btn = obj;
                    lv_obj_set_pos(obj, 284, 0);
                    lv_obj_set_size(obj, 164, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_lightness_btn, LV_EVENT_ALL, flowState);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa8b1b8), LV_PART_MAIN | LV_STATE_CHECKED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_img_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_img_set_src(obj, &img_setting2);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 82, 40);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "背光");
                        }
                    }
                }
                {
                    // setting_sound_btn
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.setting_sound_btn = obj;
                    lv_obj_set_pos(obj, 284, 0);
                    lv_obj_set_size(obj, 164, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_sound_btn, LV_EVENT_ALL, flowState);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa8b1b8), LV_PART_MAIN | LV_STATE_CHECKED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_img_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_img_set_src(obj, &img_setting2);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 82, 40);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "声音");
                        }
                    }
                }
                {
                    // setting_wifi_btn
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.setting_wifi_btn = obj;
                    lv_obj_set_pos(obj, 284, 0);
                    lv_obj_set_size(obj, 164, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_wifi_btn, LV_EVENT_ALL, flowState);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa8b1b8), LV_PART_MAIN | LV_STATE_CHECKED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_img_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_img_set_src(obj, &img_setting2);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 82, 40);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "WIFI");
                        }
                    }
                }
            }
        }
        {
            // wifi_content_area
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wifi_content_area = obj;
            lv_obj_set_pos(obj, 289, 67);
            lv_obj_set_size(obj, 491, 387);
            lv_obj_set_style_flex_grow(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 561, 186);
                    lv_obj_set_size(obj, 313, LV_SIZE_CONTENT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_CHECKED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 24, -5);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "WIFI");
                        }
                        {
                            // wifi_switch
                            lv_obj_t *obj = lv_switch_create(parent_obj);
                            objects.wifi_switch = obj;
                            lv_obj_set_pos(obj, 112, 5);
                            lv_obj_set_size(obj, 50, 25);
                            lv_obj_add_event_cb(obj, event_handler_cb_setting_page_wifi_switch, LV_EVENT_ALL, flowState);
                        }
                        {
                            // connected_wifi_info_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.connected_wifi_info_label = obj;
                            lv_obj_set_pos(obj, 178, -5);
                            lv_obj_set_size(obj, 223, LV_SIZE_CONTENT);
                            lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "连接到 WIIF ZTE-1819，IP:192.168.1.1");
                        }
                    }
                }
                {
                    // wifi_list_panel
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.wifi_list_panel = obj;
                    lv_obj_set_pos(obj, 316, 224);
                    lv_obj_set_size(obj, 436, 285);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xff707781), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // lightness_content_area
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.lightness_content_area = obj;
            lv_obj_set_pos(obj, 289, 67);
            lv_obj_set_size(obj, 491, 83);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 7, -14);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "亮度");
                }
                {
                    // setting_lightness_slider
                    lv_obj_t *obj = lv_slider_create(parent_obj);
                    objects.setting_lightness_slider = obj;
                    lv_obj_set_pos(obj, 74, 26);
                    lv_obj_set_size(obj, 362, 20);
                    lv_slider_set_value(obj, 25, LV_ANIM_OFF);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_lightness_slider, LV_EVENT_ALL, flowState);
                }
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 15, 20);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_setting2);
                }
            }
        }
        {
            // sound_content_area
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.sound_content_area = obj;
            lv_obj_set_pos(obj, 289, 67);
            lv_obj_set_size(obj, 491, 83);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 7, -14);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "声音");
                }
                {
                    // setting_sound_slider
                    lv_obj_t *obj = lv_slider_create(parent_obj);
                    objects.setting_sound_slider = obj;
                    lv_obj_set_pos(obj, 74, 26);
                    lv_obj_set_size(obj, 362, 20);
                    lv_slider_set_value(obj, 25, LV_ANIM_OFF);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_sound_slider, LV_EVENT_ALL, flowState);
                }
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 15, 20);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_setting2);
                }
            }
        }
        {
            // setting_wifi_secret_picker_mask
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.setting_wifi_secret_picker_mask = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_opa(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.obj7 = obj;
                    lv_obj_set_pos(obj, 239, 328);
                    lv_obj_set_size(obj, 100, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_obj7, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "取消");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.obj8 = obj;
                    lv_obj_set_pos(obj, 492, 328);
                    lv_obj_set_size(obj, 100, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_obj8, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "确定");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 81, 31);
                    lv_obj_set_size(obj, LV_PCT(80), LV_PCT(60));
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 72, 86);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "密码");
                        }
                        {
                            // setting_wifi_secret
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.setting_wifi_secret = obj;
                            lv_obj_set_pos(obj, 148, 76);
                            lv_obj_set_size(obj, 329, 60);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_wifi_secret, LV_EVENT_ALL, flowState);
                            lv_obj_set_style_text_font(obj, ui_font_alibaba_pu_hui_ti_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // setting_keyboard
                    lv_obj_t *obj = lv_keyboard_create(parent_obj);
                    objects.setting_keyboard = obj;
                    lv_obj_set_pos(obj, -22, 218);
                    lv_obj_set_size(obj, 854, 240);
                    lv_obj_add_event_cb(obj, event_handler_cb_setting_page_setting_keyboard, LV_EVENT_ALL, flowState);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }
    lv_keyboard_set_textarea(objects.setting_keyboard, objects.setting_wifi_secret);
    
    tick_screen_setting_page();
}

void delete_screen_setting_page() {
    lv_obj_del(objects.setting_page);
    objects.setting_page = 0;
    objects.obj6 = 0;
    objects.left_menu_panel = 0;
    objects.setting_lightness_btn = 0;
    objects.setting_sound_btn = 0;
    objects.setting_wifi_btn = 0;
    objects.wifi_content_area = 0;
    objects.wifi_switch = 0;
    objects.connected_wifi_info_label = 0;
    objects.wifi_list_panel = 0;
    objects.lightness_content_area = 0;
    objects.setting_lightness_slider = 0;
    objects.sound_content_area = 0;
    objects.setting_sound_slider = 0;
    objects.setting_wifi_secret_picker_mask = 0;
    objects.obj7 = 0;
    objects.obj8 = 0;
    objects.setting_wifi_secret = 0;
    objects.setting_keyboard = 0;
    deletePageFlowState(6);
}

void tick_screen_setting_page() {
    void *flowState = getFlowState(0, 6);
    (void)flowState;
    {
        bool new_val = evalBooleanProperty(flowState, 15, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.wifi_content_area, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.wifi_content_area;
            if (new_val) {
                lv_obj_add_flag(objects.wifi_content_area, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.wifi_content_area, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 21, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.lightness_content_area, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.lightness_content_area;
            if (new_val) {
                lv_obj_add_flag(objects.lightness_content_area, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.lightness_content_area, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 25, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.sound_content_area, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.sound_content_area;
            if (new_val) {
                lv_obj_add_flag(objects.sound_content_area, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.sound_content_area, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_cpu_page() {
    void *flowState = getFlowState(0, 7);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.cpu_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    
    tick_screen_cpu_page();
}

void delete_screen_cpu_page() {
    lv_obj_del(objects.cpu_page);
    objects.cpu_page = 0;
    deletePageFlowState(7);
}

void tick_screen_cpu_page() {
    void *flowState = getFlowState(0, 7);
    (void)flowState;
}

void create_screen_image_page() {
    void *flowState = getFlowState(0, 8);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.image_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_image_page_image_page, LV_EVENT_ALL, flowState);
    
    tick_screen_image_page();
}

void delete_screen_image_page() {
    lv_obj_del(objects.image_page);
    objects.image_page = 0;
    deletePageFlowState(8);
}

void tick_screen_image_page() {
    void *flowState = getFlowState(0, 8);
    (void)flowState;
}

void create_screen_video_page() {
    void *flowState = getFlowState(0, 9);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.video_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_video_page_video_page, LV_EVENT_ALL, flowState);
    
    tick_screen_video_page();
}

void delete_screen_video_page() {
    lv_obj_del(objects.video_page);
    objects.video_page = 0;
    deletePageFlowState(9);
}

void tick_screen_video_page() {
    void *flowState = getFlowState(0, 9);
    (void)flowState;
}

void create_screen_camera_page() {
    void *flowState = getFlowState(0, 10);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.camera_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 854, 480);
    lv_obj_add_event_cb(obj, event_handler_cb_camera_page_camera_page, LV_EVENT_ALL, flowState);
    
    tick_screen_camera_page();
}

void delete_screen_camera_page() {
    lv_obj_del(objects.camera_page);
    objects.camera_page = 0;
    deletePageFlowState(10);
}

void tick_screen_camera_page() {
    void *flowState = getFlowState(0, 10);
    (void)flowState;
}

typedef void (*create_screen_func_t)();
create_screen_func_t create_screen_funcs[] = {
    create_screen_main,
    create_screen_p1,
    create_screen_p2,
    create_screen_music_page,
    create_screen_weather_page,
    create_screen_sensor_page,
    create_screen_setting_page,
    create_screen_cpu_page,
    create_screen_image_page,
    create_screen_video_page,
    create_screen_camera_page,
};
void create_screen(int screen_index) {
    create_screen_funcs[screen_index]();
}
void create_screen_by_id(enum ScreensEnum screenId) {
    create_screen_funcs[screenId - 1]();
}

typedef void (*delete_screen_func_t)();
delete_screen_func_t delete_screen_funcs[] = {
    delete_screen_main,
    delete_screen_p1,
    delete_screen_p2,
    delete_screen_music_page,
    delete_screen_weather_page,
    delete_screen_sensor_page,
    delete_screen_setting_page,
    delete_screen_cpu_page,
    delete_screen_image_page,
    delete_screen_video_page,
    delete_screen_camera_page,
};
void delete_screen(int screen_index) {
    delete_screen_funcs[screen_index]();
}
void delete_screen_by_id(enum ScreensEnum screenId) {
    delete_screen_funcs[screenId - 1]();
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_p1,
    tick_screen_p2,
    tick_screen_music_page,
    tick_screen_weather_page,
    tick_screen_sensor_page,
    tick_screen_setting_page,
    tick_screen_cpu_page,
    tick_screen_image_page,
    tick_screen_video_page,
    tick_screen_camera_page,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

//
// Fonts
//

lv_font_t *ui_font_qweather_icons_32;
lv_font_t *ui_font_qweather_icons_64;
lv_font_t *ui_font_qweather_icons_128;
lv_font_t *ui_font_alibaba_pu_hui_ti_16;
lv_font_t *ui_font_alibaba_pu_hui_ti_24;
lv_font_t *ui_font_alibaba_pu_hui_ti_32;
lv_font_t *ui_font_alibaba_pu_hui_ti_64;

ext_font_desc_t fonts[] = {
    { "qweather-icons-32", NULL },
    { "qweather-icons-64", NULL },
    { "qweather-icons-128", NULL },
    { "AlibabaPuHuiTi-16", NULL },
    { "AlibabaPuHuiTi-24", NULL },
    { "AlibabaPuHuiTi-32", NULL },
    { "AlibabaPuHuiTi-64", NULL },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
//
//

void create_screens() {
    // Load external fonts
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/qweather-icons.ttf";
        info.weight = 32;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_qweather_icons_32 = info.font;
            fonts[0].font_ptr = ui_font_qweather_icons_32;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_qweather_icons_32");
        }
    }
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/qweather-icons.ttf";
        info.weight = 64;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_qweather_icons_64 = info.font;
            fonts[1].font_ptr = ui_font_qweather_icons_64;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_qweather_icons_64");
        }
    }
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/qweather-icons.ttf";
        info.weight = 128;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_qweather_icons_128 = info.font;
            fonts[2].font_ptr = ui_font_qweather_icons_128;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_qweather_icons_128");
        }
    }
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/AlibabaPuHuiTi-3-115-Black.ttf";
        info.weight = 16;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_alibaba_pu_hui_ti_16 = info.font;
            fonts[3].font_ptr = ui_font_alibaba_pu_hui_ti_16;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_alibaba_pu_hui_ti_16");
        }
    }
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/AlibabaPuHuiTi-3-115-Black.ttf";
        info.weight = 24;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_alibaba_pu_hui_ti_24 = info.font;
            fonts[4].font_ptr = ui_font_alibaba_pu_hui_ti_24;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_alibaba_pu_hui_ti_24");
        }
    }
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/AlibabaPuHuiTi-3-115-Black.ttf";
        info.weight = 32;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_alibaba_pu_hui_ti_32 = info.font;
            fonts[5].font_ptr = ui_font_alibaba_pu_hui_ti_32;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_alibaba_pu_hui_ti_32");
        }
    }
    {
        lv_ft_info_t info;
        info.name = "/usr/share/fonts/AlibabaPuHuiTi-3-115-Black.ttf";
        info.weight = 64;
        info.style = FT_FONT_STYLE_NORMAL;
        info.mem = 0;
        if (lv_ft_font_init(&info)) {
            ui_font_alibaba_pu_hui_ti_64 = info.font;
            fonts[6].font_ptr = ui_font_alibaba_pu_hui_ti_64;
        } else {
            LV_LOG_ERROR("font create failed: ui_font_alibaba_pu_hui_ti_64");
        }
    }
    
    eez_flow_init_fonts(fonts, sizeof(fonts) / sizeof(ext_font_desc_t));

// Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    // Initialize screens
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    
    eez_flow_set_create_screen_func(create_screen);
    eez_flow_set_delete_screen_func(delete_screen);
    
    // Create screens
    create_screen_main();
    create_screen_p1();
    create_screen_p2();
    create_screen_music_page();
    create_screen_weather_page();
    create_screen_sensor_page();
    create_screen_setting_page();
    create_screen_cpu_page();
    create_screen_image_page();
    create_screen_video_page();
    create_screen_camera_page();
}