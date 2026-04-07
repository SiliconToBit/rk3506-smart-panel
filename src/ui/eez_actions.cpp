// ============================================================================
// EEZ Studio Actions 动作实现
//
// 此文件放在 src/ui/ 目录下，不会被 EEZ Studio 覆盖
// ============================================================================

#include "actions.h"
#include "screens.h"
#include "fonts.h"
#include "app/WeatherApp.h"
#include "app/CityData.h"
#include "hal/HttpClient.h"
#include <cstdio>
#include <cstring>

// 前向声明 objects 结构体（由 screens.c 定义）
extern _objects_t objects;

extern "C"
{

    // ============================================================================
    //                              音乐播放相关
    // ============================================================================

    void action_load_music_directory(lv_event_t* e)
    {
        printf("[Stub] action_load_music_directory\n");
    }

    void action_toggle_play_pause(lv_event_t* e)
    {
        printf("[Stub] action_toggle_play_pause\n");
    }

    void action_play_next(lv_event_t* e)
    {
        printf("[Stub] action_play_next\n");
    }

    void action_play_prev(lv_event_t* e)
    {
        printf("[Stub] action_play_prev\n");
    }

    void action_update_album_art(lv_event_t* e)
    {
        printf("[Stub] action_update_album_art\n");
    }

    void action_init_music_screen(lv_event_t* e)
    {
        printf("[Stub] action_init_music_screen\n");
    }

    void action_deinit_music_screen(lv_event_t* e)
    {
        printf("[Stub] action_deinit_music_screen\n");
    }

    void action_music_volume_changed(lv_event_t* e)
    {
        printf("[Stub] action_music_volume_changed\n");
    }

    void action_music_progress_changed(lv_event_t* e)
    {
        printf("[Stub] action_music_progress_changed\n");
    }

    // ============================================================================
    //                              天气相关
    // ============================================================================

    // 城市数据 CSV 文件路径
    static const char* CITY_CSV_PATH = "/root/China-City-List-latest.csv";
    static bool g_weather_callbacks_bound = false;
    static bool g_weather_network_warmed = false;

    static void warmup_weather_network_once()
    {
        if (g_weather_network_warmed)
        {
            return;
        }

        (void) hal::HttpClient::getInstance();
        g_weather_network_warmed = true;
    }

    // 更新天气 UI 显示
    static void update_weather_ui()
    {
        auto& weatherApp = app::WeatherApp::getInstance();
        auto weather = weatherApp.getCurrentWeather();
        auto forecast = weatherApp.getForecast();
        auto toDisplayInt = [](float value) -> int
        { return static_cast<int>(value >= 0.0f ? value + 0.5f : value - 0.5f); };

        if (!weather.valid)
        {
            printf("[Weather] No valid data yet\n");
            return;
        }

        // 更新当前天气
        if (objects.weather_temp_label)
        {
            lv_label_set_text_fmt(objects.weather_temp_label, "%d°C", toDisplayInt(weather.temperature));
        }

        if (objects.weather_text_label)
        {
            lv_label_set_text(objects.weather_text_label, weather.weather.c_str());
        }

        if (objects.weather_wind_label)
        {
            lv_label_set_text_fmt(objects.weather_wind_label, "%s %dkm/h", weather.windDirection.c_str(),
                                  toDisplayInt(weather.windSpeed));
        }

        if (objects.weather_icon_label)
        {
            // Some deployed font packs do not contain all fill variants.
            std::string iconStr = app::WeatherApp::getWeatherIconStr(weather.icon, false);
            lv_label_set_text(objects.weather_icon_label, iconStr.c_str());
            lv_obj_set_style_text_font(objects.weather_icon_label, ui_font_qweather_icons_128, 0);
        }

        if (objects.weather_city_label)
        {
            lv_label_set_text(objects.weather_city_label, weatherApp.getFullCityName().c_str());
        }

        if (objects.weather_data_label)
        {
            lv_label_set_text_fmt(objects.weather_data_label, "湿度 %d%%", toDisplayInt(weather.humidity));
        }

        // 更新预报（最多3天）
        for (size_t i = 0; i < 3 && i < forecast.size(); ++i)
        {
            lv_obj_t* iconLabel = nullptr;
            lv_obj_t* tempLabel = nullptr;
            lv_obj_t* windLabel = nullptr;

            if (i == 0)
            {
                iconLabel = objects.weather_fcst_icon_0_label;
                tempLabel = objects.weather_fcst_temp_0_label;
                windLabel = objects.weather_fcst_wind_0_label;
            }
            else if (i == 1)
            {
                iconLabel = objects.weather_fcst_icon_1_label;
                tempLabel = objects.weather_fcst_temp_1_label;
                windLabel = objects.weather_fcst_wind_1_label;
            }
            else if (i == 2)
            {
                iconLabel = objects.weather_fcst_icon_2_label;
                tempLabel = objects.weather_fcst_temp_2_label;
                windLabel = objects.weather_fcst_wind_2_label;
            }

            if (iconLabel)
            {
                std::string iconStr = app::WeatherApp::getWeatherIconStr(forecast[i].icon, false);
                lv_label_set_text(iconLabel, iconStr.c_str());
                lv_obj_set_style_text_font(iconLabel, ui_font_qweather_icons_32, 0);
            }

            if (tempLabel)
            {
                lv_label_set_text_fmt(tempLabel, "%d°/%d°", toDisplayInt(forecast[i].tempHigh),
                                      toDisplayInt(forecast[i].tempLow));
            }

            if (windLabel)
            {
                lv_label_set_text_fmt(windLabel, "%s", forecast[i].windDirection.c_str());
            }
        }

        printf("[Weather] UI updated: %s, %d°C\n", weather.weather.c_str(), toDisplayInt(weather.temperature));
    }

    static void bind_weather_callbacks_once()
    {
        if (g_weather_callbacks_bound)
        {
            return;
        }

        auto& weatherApp = app::WeatherApp::getInstance();
        weatherApp.setOnDataUpdated([]() { update_weather_ui(); });

        weatherApp.setOnError(
            [](const std::string& errorMsg)
            {
                printf("[Weather] Async refresh failed: %s\n", errorMsg.c_str());
                if (objects.weather_text_label)
                {
                    lv_label_set_text(objects.weather_text_label, "获取失败");
                }
            });

        g_weather_callbacks_bound = true;
    }

    void action_init_weather_screen(lv_event_t* e)
    {
        printf("[Weather] Initializing weather screen\n");

        warmup_weather_network_once();

        auto& weatherApp = app::WeatherApp::getInstance();
        auto& cityData = app::CityData::getInstance();

        // 加载城市数据（如果未加载）
        if (!cityData.isLoaded())
        {
            if (cityData.loadFromCSV(CITY_CSV_PATH))
            {
                printf("[Weather] City data loaded: %d provinces\n", cityData.getProvinceCount());

                // 初始化省份 Roller
                if (objects.weather_prov_roller)
                {
                    std::string provList = cityData.getProvinceList();
                    lv_roller_set_options(objects.weather_prov_roller, provList.c_str(), LV_ROLLER_MODE_NORMAL);
                    lv_roller_set_selected(objects.weather_prov_roller, 0, LV_ANIM_OFF);
                }
            }
            else
            {
                printf("[Weather] Failed to load city data\n");
            }
        }

        // 设置默认城市（如果未设置）
        if (weatherApp.getLocationId().empty())
        {
            // 默认使用衡阳常宁
            std::string code = cityData.findCityCode("湖南", "衡阳", "常宁");
            if (!code.empty())
            {
                weatherApp.setLocation(code, "衡阳常宁");
            }
            else
            {
                weatherApp.setCity("湖南", "衡阳");
            }
        }

        bind_weather_callbacks_once();

        // 先显示缓存数据，随后异步刷新，避免进入页面阻塞
        if (weatherApp.hasValidData())
        {
            update_weather_ui();
        }

        if (objects.weather_text_label)
        {
            lv_label_set_text(objects.weather_text_label, "刷新中...");
        }

        weatherApp.refreshAsync();
    }

    void action_deinit_weather_screen(lv_event_t* e)
    {
        printf("[Weather] Deinitializing weather screen\n");
    }

    void action_confirm_city_picker(lv_event_t* e)
    {
        printf("[Weather] Confirm city picker\n");

        auto& weatherApp = app::WeatherApp::getInstance();
        auto& cityData = app::CityData::getInstance();

        if (!cityData.isLoaded())
        {
            printf("[Weather] City data not loaded\n");
            return;
        }

        // 获取选中的省份、城市、区县索引
        int provIdx = 0, cityIdx = 0, countyIdx = 0;

        if (objects.weather_prov_roller)
            provIdx = lv_roller_get_selected(objects.weather_prov_roller);

        if (objects.weather_city_roller)
            cityIdx = lv_roller_get_selected(objects.weather_city_roller);

        if (objects.weather_county_roller)
            countyIdx = lv_roller_get_selected(objects.weather_county_roller);

        // 获取城市代码
        std::string code = cityData.getCityCode(provIdx, cityIdx, countyIdx);
        std::string fullName = cityData.getFullCityName(provIdx, cityIdx, countyIdx);

        if (!code.empty())
        {
            printf("[Weather] Selected: %s, code: %s\n", fullName.c_str(), code.c_str());
            weatherApp.setLocation(code, fullName);

            if (objects.weather_text_label)
            {
                lv_label_set_text(objects.weather_text_label, "刷新中...");
            }

            weatherApp.refreshAsync();
        }
        else
        {
            printf("[Weather] Failed to get city code\n");
        }

        // 关闭城市选择器
        if (objects.weather_city_picker_mask)
        {
            lv_obj_add_flag(objects.weather_city_picker_mask, LV_OBJ_FLAG_HIDDEN);
        }
    }

    void action_prov_changed(lv_event_t* e)
    {
        printf("[Weather] Province changed\n");

        auto& cityData = app::CityData::getInstance();

        if (!cityData.isLoaded())
            return;

        // 省份改变时更新城市列表
        if (objects.weather_prov_roller && objects.weather_city_roller)
        {
            int provIdx = lv_roller_get_selected(objects.weather_prov_roller);

            std::string cityList = cityData.getCityList(provIdx);
            if (!cityList.empty())
            {
                lv_roller_set_options(objects.weather_city_roller, cityList.c_str(), LV_ROLLER_MODE_NORMAL);
                lv_roller_set_selected(objects.weather_city_roller, 0, LV_ANIM_OFF);
            }

            // 同时更新区县列表（选择第一个城市的区县）
            if (objects.weather_county_roller)
            {
                std::string countyList = cityData.getCountyList(provIdx, 0);
                if (!countyList.empty())
                {
                    lv_roller_set_options(objects.weather_county_roller, countyList.c_str(), LV_ROLLER_MODE_NORMAL);
                    lv_roller_set_selected(objects.weather_county_roller, 0, LV_ANIM_OFF);
                }
            }
        }
    }

    void action_city_changed(lv_event_t* e)
    {
        printf("[Weather] City changed\n");

        auto& cityData = app::CityData::getInstance();

        if (!cityData.isLoaded())
            return;

        // 城市改变时更新区县列表
        if (objects.weather_prov_roller && objects.weather_city_roller && objects.weather_county_roller)
        {
            int provIdx = lv_roller_get_selected(objects.weather_prov_roller);
            int cityIdx = lv_roller_get_selected(objects.weather_city_roller);

            std::string countyList = cityData.getCountyList(provIdx, cityIdx);
            if (!countyList.empty())
            {
                lv_roller_set_options(objects.weather_county_roller, countyList.c_str(), LV_ROLLER_MODE_NORMAL);
                lv_roller_set_selected(objects.weather_county_roller, 0, LV_ANIM_OFF);
            }
        }
    }

    // ============================================================================
    //                              设置相关
    // ============================================================================

    void action_setting_brightness_changed(lv_event_t* e)
    {
        printf("[Stub] action_setting_brightness_changed\n");
    }

    void action_setting_volume_changed(lv_event_t* e)
    {
        printf("[Stub] action_setting_volume_changed\n");
    }

    void action_setting_wifi_state_changed(lv_event_t* e)
    {
        printf("[Stub] action_setting_wifi_state_changed\n");
    }

    void action_init_wifi_setting_page(lv_event_t* e)
    {
        printf("[Stub] action_init_wifi_setting_page\n");
    }

    void action_confirm_wifi_password(lv_event_t* e)
    {
        printf("[Stub] action_confirm_wifi_password\n");
    }

} // extern "C"
