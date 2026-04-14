// ============================================================================
// EEZ Studio Actions 动作实现
//
// 此文件放在 src/ui/ 目录下，不会被 EEZ Studio 覆盖
// ============================================================================

#include "MusicApp.h"
#include "actions.h"
#include "screens.h"
#include "fonts.h"
#include "app/WeatherApp.h"
#include "app/CityData.h"
#include "hal/HttpClient.h"
#include <cstdio>
#include <cstring>
#include <vector>

// 前向声明 objects 结构体（由 screens.c 定义）
extern _objects_t objects;

namespace
{
    // Keep stable per-item indices for LVGL event user_data pointers.
    std::vector<int> g_musicListIndices;
    // Keep button handles to support highlight/auto-scroll by song index.
    std::vector<lv_obj_t*> g_musicListButtons;

    void set_current_song_highlight(int activeIndex, bool autoScroll)
    {
        for (size_t i = 0; i < g_musicListButtons.size(); ++i)
        {
            lv_obj_t* itemBtn = g_musicListButtons[i];
            if (itemBtn == nullptr)
            {
                continue;
            }

            const bool isActive = (static_cast<int>(i) == activeIndex);
            lv_obj_set_style_bg_color(itemBtn, lv_color_hex(isActive ? 0x2D6CDF : 0x333333),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(itemBtn, lv_color_hex(isActive ? 0x265AB8 : 0x666666),
                                      LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(itemBtn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_t* itemLabel = lv_obj_get_child(itemBtn, 0);
            if (itemLabel != nullptr)
            {
                lv_obj_set_style_text_color(itemLabel, lv_color_hex(isActive ? 0xFFFFFF : 0xE6E6E6), 0);
            }
        }

        if (autoScroll && activeIndex >= 0 && activeIndex < static_cast<int>(g_musicListButtons.size()))
        {
            lv_obj_t* activeBtn = g_musicListButtons[static_cast<size_t>(activeIndex)];
            if (activeBtn != nullptr)
            {
                lv_obj_scroll_to_view(activeBtn, LV_ANIM_ON);
            }
        }
    }

    void sync_play_pause_button_state(bool isPlaying)
    {
        if (objects.music_play_pause_imgbtn == nullptr)
        {
            return;
        }

        if (isPlaying)
        {
            lv_obj_add_state(objects.music_play_pause_imgbtn, LV_STATE_CHECKED);
            lv_imgbtn_set_state(objects.music_play_pause_imgbtn, LV_IMGBTN_STATE_CHECKED_RELEASED);
        }
        else
        {
            lv_obj_clear_state(objects.music_play_pause_imgbtn, LV_STATE_CHECKED);
            lv_imgbtn_set_state(objects.music_play_pause_imgbtn, LV_IMGBTN_STATE_RELEASED);
        }

        // Force redraw so icon source switches immediately on all targets.
        lv_obj_invalidate(objects.music_play_pause_imgbtn);
    }

    // Click handler for a song row in scroll_list_area.
    void on_music_list_item_clicked(lv_event_t* event)
    {
        int* indexPtr = static_cast<int*>(lv_event_get_user_data(event));
        if (indexPtr == nullptr)
        {
            return;
        }

        auto& musicApp = app::MusicApp::getInstance();
        const int requestedIndex = *indexPtr;
        const bool shouldAutoplay = musicApp.isPlaying() && !musicApp.isPaused();
        if (musicApp.play(requestedIndex, shouldAutoplay))
        {
            sync_play_pause_button_state(shouldAutoplay);
            set_current_song_highlight(requestedIndex, true);
            return;
        }

        set_current_song_highlight(musicApp.getCurrentIndex(), false);
    }

    // Rebuild music list UI from current MusicApp playlist.
    void rebuild_music_song_list_ui()
    {
        if (objects.scroll_list_area == nullptr)
        {
            return;
        }

        auto& musicApp = app::MusicApp::getInstance();
        const int songCount = musicApp.getPlaylistSize();

        lv_obj_clean(objects.scroll_list_area);
        g_musicListIndices.assign(static_cast<size_t>(songCount), 0);
        g_musicListButtons.clear();
        g_musicListButtons.reserve(static_cast<size_t>(songCount));

        lv_obj_set_layout(objects.scroll_list_area, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(objects.scroll_list_area, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(objects.scroll_list_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        for (int index = 0; index < songCount; ++index)
        {
            g_musicListIndices[static_cast<size_t>(index)] = index;

            lv_obj_t* itemBtn = lv_btn_create(objects.scroll_list_area);
            lv_obj_set_width(itemBtn, lv_pct(100));
            lv_obj_set_height(itemBtn, 44);
            lv_obj_set_style_bg_color(itemBtn, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(itemBtn, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_add_event_cb(itemBtn, on_music_list_item_clicked, LV_EVENT_CLICKED,
                                &g_musicListIndices[static_cast<size_t>(index)]);

            lv_obj_t* itemLabel = lv_label_create(itemBtn);
            lv_label_set_long_mode(itemLabel, LV_LABEL_LONG_DOT);
            lv_obj_set_width(itemLabel, lv_pct(94));
            lv_obj_set_style_text_font(itemLabel, ui_font_alibaba_pu_hui_ti_24, 0);
            lv_label_set_text(itemLabel, musicApp.getSongName(index).c_str());
            lv_obj_center(itemLabel);

            g_musicListButtons.push_back(itemBtn);
        }

        set_current_song_highlight(musicApp.getCurrentIndex(), true);
    }
} // namespace

extern "C"
{

    // ============================================================================
    //                              音乐播放相关
    // ============================================================================

    void action_load_music_directory(lv_event_t* e)
    {
        // Load song files and immediately refresh the list panel.
        auto& musicApp = app::MusicApp::getInstance();
        int count = musicApp.loadDirectory("/root/Music");
        printf("[Music] Loaded %d songs from /root/Music\n", count);
        rebuild_music_song_list_ui();
    }

    void action_toggle_play_pause(lv_event_t* e)
    {
        lv_obj_t* target = lv_event_get_target(e);
        const bool checked = (target != nullptr) && lv_obj_has_state(target, LV_STATE_CHECKED);
        auto& musicApp = app::MusicApp::getInstance();

        if (!checked)
        {
            musicApp.pause();
        }
        else
        {
            if (musicApp.isPaused())
            {
                musicApp.resume();
            }
            else
            {
                if (musicApp.getPlaylistSize() == 0)
                {
                    printf("[Music] Playlist is empty, load music first\n");
                    sync_play_pause_button_state(false);
                    return;
                }

                int index = musicApp.getCurrentIndex();
                if (index < 0)
                {
                    index = 0;
                }
                if (!musicApp.play(index))
                {
                    sync_play_pause_button_state(false);
                }
            }
        }
    }

    void action_play_next(lv_event_t* e)
    {
        app::MusicApp::getInstance().next();
    }

    void action_play_prev(lv_event_t* e)
    {
        app::MusicApp::getInstance().prev();
    }

    void action_update_album_art(lv_event_t* e)
    {
        // 专辑封面由 onSongChanged 回调处理
    }

    void action_init_music_screen(lv_event_t* e)
    {
        // Initialize playlist/UI state and bind one-time music callbacks.
        auto& musicApp = app::MusicApp::getInstance();
        int count = musicApp.loadDirectory("/root/Music");
        printf("[Music] Loaded %d songs from /root/Music\n", count);
        rebuild_music_song_list_ui();

        if (objects.music_volume_slider != nullptr)
        {
            lv_slider_set_value(objects.music_volume_slider, static_cast<int>(musicApp.getVolume() * 100.0F),
                                LV_ANIM_OFF);
        }
        if (objects.music_progress_slider != nullptr)
        {
            lv_slider_set_value(objects.music_progress_slider, 0, LV_ANIM_OFF);
        }
        if (objects.music_lyrics_label != nullptr)
        {
            lv_label_set_text(objects.music_lyrics_label, "");
        }

        // 设置歌曲切换回调
        musicApp.setOnSongChanged(
            [](int index, const std::string& songName)
            {
                if (objects.music_title_label)
                {
                    lv_label_set_text(objects.music_title_label, songName.c_str());
                }
                auto& musicApp = app::MusicApp::getInstance();
                sync_play_pause_button_state(musicApp.isPlaying() && !musicApp.isPaused());
                set_current_song_highlight(index, true);
                printf("[Music] Song changed: [%d] %s\n", index, songName.c_str());
            });

        // 设置播放状态回调
        musicApp.setOnStateChanged(
            [](bool playing)
            {
                sync_play_pause_button_state(playing);
                printf("[Music] State changed: %s\n", playing ? "playing" : "stopped");
            });

        // 初始化按钮状态，防止进入页面时按钮与实际播放状态不一致。
        sync_play_pause_button_state(musicApp.isPlaying() && !musicApp.isPaused());

        // 设置进度回调，同步进度条和时间标签
        musicApp.setOnProgressChanged(
            [](const app::MusicApp::ProgressInfo& info)
            {
                if (objects.music_progress_slider != nullptr &&
                    !lv_obj_has_state(objects.music_progress_slider, LV_STATE_PRESSED))
                {
                    const int progressValue = static_cast<int>(info.progressPercent * 100.0F);
                    lv_slider_set_value(objects.music_progress_slider, progressValue, LV_ANIM_OFF);
                }

                if (objects.music_curr_time_label != nullptr)
                {
                    const int currSec = static_cast<int>(info.currentPosition);
                    lv_label_set_text_fmt(objects.music_curr_time_label, "%02d:%02d", currSec / 60, currSec % 60);
                }

                if (objects.music_total_time_label != nullptr)
                {
                    const int totalSec = static_cast<int>(info.duration);
                    lv_label_set_text_fmt(objects.music_total_time_label, "%02d:%02d", totalSec / 60, totalSec % 60);
                }
            });

        // 设置歌词回调
        musicApp.setOnLyricChanged(
            [](const std::string& lyric)
            {
                if (objects.music_lyrics_label != nullptr)
                {
                    lv_label_set_text(objects.music_lyrics_label, lyric.c_str());
                }
            });

        // 设置错误回调
        musicApp.setOnError([](const std::string& errorMsg) { printf("[Music Error] %s\n", errorMsg.c_str()); });

        // 启动进度更新定时器
        musicApp.startProgressTimer(500);
    }

    void action_deinit_music_screen(lv_event_t* e)
    {
        app::MusicApp::getInstance().stopProgressTimer();
    }

    void action_music_volume_changed(lv_event_t* e)
    {
        lv_obj_t* slider = lv_event_get_target(e);
        if (slider == nullptr)
        {
            return;
        }

        int value = lv_slider_get_value(slider);
        app::MusicApp::getInstance().setVolume(value / 100.0F);
    }

    void action_music_progress_changed(lv_event_t* e)
    {
        lv_obj_t* slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        auto& musicApp = app::MusicApp::getInstance();
        double duration = musicApp.getDuration();
        if (duration > 0)
        {
            musicApp.seek(duration * value / 100.0);
        }
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
