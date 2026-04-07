#include "app/WeatherApp.h"
#include "hal/HttpClient.h"
#include "nlohmann/json.hpp"
#include <lvgl/lvgl.h>
#include <iostream>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include "./include/fonts/qweather_icons.h" // 天气图标映射

namespace app
{

    using json = nlohmann::json;

    // ==========================================
    // 构造与析构
    // ==========================================

    WeatherApp::WeatherApp()
        : m_httpClient(hal::HttpClient::getInstance())
    {
        setupHttpClient();
    }

    WeatherApp::~WeatherApp()
    {
        lv_async_call_cancel(asyncDataUpdatedThunk, this);
        lv_async_call_cancel(asyncErrorThunk, this);
    }

    // ==========================================
    // 单例访问
    // ==========================================

    WeatherApp& WeatherApp::getInstance()
    {
        static WeatherApp instance;
        return instance;
    }

    // ==========================================
    // 城市管理
    // ==========================================

    void WeatherApp::setLocation(const std::string& locationId, const std::string& cityName)
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        m_locationId = locationId;
        if (!cityName.empty())
        {
            m_city = cityName;
        }
    }

    void WeatherApp::setCity(const std::string& province, const std::string& city)
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        m_province = province;
        m_city = city;

        // 常用城市代码映射表（可根据需要扩展）
        static const std::vector<std::pair<std::string, std::string>> cityCodeMap = {
            {"衡阳常宁", "101250412"}, {"衡阳", "101250400"}, {"长沙", "101250101"}, {"深圳", "101280601"},
            {"广州", "101280101"},     {"北京", "101010100"}, {"上海", "101020100"}, {"杭州", "101210101"},
            {"成都", "101270101"},     {"武汉", "101200101"}, {"西安", "101110101"}, {"南京", "101190101"},
            {"重庆", "101040100"},     {"天津", "101030100"}, {"苏州", "101190401"}, {"郑州", "101180101"},
            {"东莞", "101281601"},     {"沈阳", "101070101"}, {"青岛", "101120201"}, {"合肥", "101220101"},
            {"佛山", "101281701"},
        };

        // 查找城市代码
        std::string searchKey = city.empty() ? province : city;
        for (const auto& pair : cityCodeMap)
        {
            if (pair.first.find(searchKey) != std::string::npos)
            {
                m_locationId = pair.second;
                return;
            }
        }

        // 默认使用城市名作为 location（和风天气支持城市名查询）
        m_locationId = city.empty() ? province : city;
    }

    std::string WeatherApp::getLocationId() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_locationId;
    }

    std::string WeatherApp::getProvince() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_province;
    }

    std::string WeatherApp::getCity() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_city;
    }

    std::string WeatherApp::getFullCityName() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        if (m_province.empty() && m_city.empty())
            return "";
        if (m_province.empty())
            return m_city;
        if (m_city.empty())
            return m_province;
        return m_province + " " + m_city;
    }

    // ==========================================
    // URL 构建
    // ==========================================

    std::string WeatherApp::buildNowApiUrl() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        if (m_locationId.empty())
            return "";

        char url[512];
        std::snprintf(url, sizeof(url), APP_WEATHER_NOW_API_URL, m_locationId.c_str());
        return std::string(url);
    }

    std::string WeatherApp::build3DApiUrl() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        if (m_locationId.empty())
            return "";

        char url[512];
        std::snprintf(url, sizeof(url), APP_WEATHER_3D_API_URL, m_locationId.c_str());
        return std::string(url);
    }

    std::string WeatherApp::build7DApiUrl() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        if (m_locationId.empty())
            return "";

        char url[512];
        std::snprintf(url, sizeof(url), APP_WEATHER_7D_API_URL, m_locationId.c_str());
        return std::string(url);
    }

    // ==========================================
    // 数据获取 - 全部刷新
    // ==========================================

    bool WeatherApp::refresh()
    {
        // 先刷新实时天气，再刷新预报
        if (!refreshCurrentWeather())
            return false;

        // 预报刷新失败不影响实时天气显示
        refreshForecast3D();
        return true;
    }

    void WeatherApp::refreshAsync()
    {
        // 异步刷新实时天气
        refreshCurrentWeatherAsync();

        // 异步刷新3天预报（可以并行）
        refreshForecast3DAsync();
    }

    // ==========================================
    // 数据获取 - 实时天气
    // ==========================================

    bool WeatherApp::refreshCurrentWeather()
    {
        std::string url = buildNowApiUrl();
        if (url.empty())
        {
            notifyError("Location not set");
            return false;
        }

        auto response = m_httpClient.get(url);
        if (!response)
        {
            notifyError("Failed to fetch current weather");
            return false;
        }

        if (!parseCurrentWeatherResponse(*response))
        {
            notifyError("Failed to parse current weather data");
            return false;
        }

        notifyDataUpdated();
        return true;
    }

    void WeatherApp::refreshCurrentWeatherAsync()
    {
        std::string url = buildNowApiUrl();
        if (url.empty())
        {
            notifyError("Location not set");
            return;
        }

        m_httpClient.getAsync(url,
                              [this](int code, const std::string& body)
                              {
                                  if (code != 200 || body.empty())
                                  {
                                      notifyError("Failed to fetch current weather (HTTP " + std::to_string(code) +
                                                  ")");
                                      return;
                                  }

                                  if (!parseCurrentWeatherResponse(body))
                                  {
                                      notifyError("Failed to parse current weather data");
                                      return;
                                  }

                                  notifyDataUpdated();
                              });
    }

    // ==========================================
    // 数据获取 - 3天预报
    // ==========================================

    bool WeatherApp::refreshForecast3D()
    {
        std::string url = build3DApiUrl();
        if (url.empty())
        {
            notifyError("Location not set");
            return false;
        }

        auto response = m_httpClient.get(url);
        if (!response)
        {
            notifyError("Failed to fetch 3-day forecast");
            return false;
        }

        if (!parseForecastResponse(*response, 3))
        {
            notifyError("Failed to parse 3-day forecast data");
            return false;
        }

        notifyDataUpdated();
        return true;
    }

    void WeatherApp::refreshForecast3DAsync()
    {
        std::string url = build3DApiUrl();
        if (url.empty())
        {
            notifyError("Location not set");
            return;
        }

        m_httpClient.getAsync(url,
                              [this](int code, const std::string& body)
                              {
                                  if (code != 200 || body.empty())
                                  {
                                      notifyError("Failed to fetch 3-day forecast (HTTP " + std::to_string(code) + ")");
                                      return;
                                  }

                                  if (!parseForecastResponse(body, 3))
                                  {
                                      notifyError("Failed to parse 3-day forecast data");
                                      return;
                                  }

                                  notifyDataUpdated();
                              });
    }

    // ==========================================
    // 数据获取 - 7天预报
    // ==========================================

    bool WeatherApp::refreshForecast7D()
    {
        std::string url = build7DApiUrl();
        if (url.empty())
        {
            notifyError("Location not set");
            return false;
        }

        auto response = m_httpClient.get(url);
        if (!response)
        {
            notifyError("Failed to fetch 7-day forecast");
            return false;
        }

        if (!parseForecastResponse(*response, 7))
        {
            notifyError("Failed to parse 7-day forecast data");
            return false;
        }

        notifyDataUpdated();
        return true;
    }

    void WeatherApp::refreshForecast7DAsync()
    {
        std::string url = build7DApiUrl();
        if (url.empty())
        {
            notifyError("Location not set");
            return;
        }

        m_httpClient.getAsync(url,
                              [this](int code, const std::string& body)
                              {
                                  if (code != 200 || body.empty())
                                  {
                                      notifyError("Failed to fetch 7-day forecast (HTTP " + std::to_string(code) + ")");
                                      return;
                                  }

                                  if (!parseForecastResponse(body, 7))
                                  {
                                      notifyError("Failed to parse 7-day forecast data");
                                      return;
                                  }

                                  notifyDataUpdated();
                              });
    }

    // ==========================================
    // 数据查询
    // ==========================================

    WeatherData WeatherApp::getCurrentWeather() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_currentWeather;
    }

    std::vector<ForecastData> WeatherApp::getForecast() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_forecast;
    }

    bool WeatherApp::hasValidData() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_currentWeather.valid;
    }

    std::string WeatherApp::getLastUpdateTime() const
    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        return m_currentWeather.updateTime;
    }

    // ==========================================
    // UI 回调设置
    // ==========================================

    void WeatherApp::setOnDataUpdated(DataUpdatedCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onDataUpdated = std::move(callback);
    }

    void WeatherApp::setOnError(ErrorCallback callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        m_onError = std::move(callback);
    }

    // ==========================================
    // 内部方法
    // ==========================================

    void WeatherApp::setupHttpClient()
    {
        m_httpClient.setTimeout(10);
        m_httpClient.setOnError([this](const std::string& msg) { notifyError(msg); });
    }

    bool WeatherApp::parseCurrentWeatherResponse(const std::string& responseBody)
    {
        // 和风天气 API 响应格式：
        // {
        //   "code": "200",
        //   "now": {
        //     "temp": "25",
        //     "text": "晴",
        //     "icon": "100",
        //     "humidity": "60",
        //     "windDir": "北风",
        //     "windSpeed": "10",
        //     ...
        //   },
        //   "updateTime": "2024-01-01T12:00+08:00"
        // }

        try
        {
            json j = json::parse(responseBody);

            // 检查响应码
            if (!j.contains("code") || j["code"] != "200")
            {
                std::cerr << "[WeatherApp] API returned error code: " << j.value("code", "unknown") << std::endl;
                return false;
            }

            // 解析 "now" 对象
            if (!j.contains("now") || !j["now"].is_object())
            {
                std::cerr << "[WeatherApp] Missing 'now' object in response" << std::endl;
                return false;
            }

            const auto& now = j["now"];

            // 辅助函数：安全获取字符串值
            auto getStringValue = [&now](const std::string& key, const std::string& defaultVal = "") -> std::string
            {
                if (now.contains(key) && !now[key].is_null())
                {
                    if (now[key].is_string())
                        return now[key].get<std::string>();
                    else if (now[key].is_number())
                        return std::to_string(now[key].get<float>());
                }
                return defaultVal;
            };

            // 辅助函数：安全获取浮点值（API 返回的是字符串格式的数字）
            auto getFloatValue = [&now](const std::string& key, float defaultVal = 0.0f) -> float
            {
                if (now.contains(key) && !now[key].is_null())
                {
                    try
                    {
                        if (now[key].is_string())
                            return std::stof(now[key].get<std::string>());
                        else if (now[key].is_number())
                            return now[key].get<float>();
                    }
                    catch (...)
                    {
                        return defaultVal;
                    }
                }
                return defaultVal;
            };

            WeatherData parsedWeather;

            // 解析实时天气数据（API 返回字符串格式的数值）
            parsedWeather.temperature = getFloatValue("temp");
            parsedWeather.weather = getStringValue("text");
            parsedWeather.icon = getStringValue("icon");
            parsedWeather.humidity = getFloatValue("humidity");
            parsedWeather.windDirection = getStringValue("windDir");
            parsedWeather.windSpeed = getFloatValue("windSpeed");

            // 设置更新时间
            parsedWeather.updateTime = j.value("updateTime", "");
            if (parsedWeather.updateTime.empty())
            {
                time_t nowTime = time(nullptr);
                char timeStr[64];
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", localtime(&nowTime));
                parsedWeather.updateTime = timeStr;
            }

            {
                std::lock_guard<std::mutex> lock(m_dataMutex);
                parsedWeather.city = m_city;
                parsedWeather.valid = !parsedWeather.weather.empty();
                m_currentWeather = parsedWeather;
            }

            return parsedWeather.valid;
        }
        catch (const json::parse_error& e)
        {
            std::cerr << "[WeatherApp] JSON parse error: " << e.what() << std::endl;
            return false;
        }
        catch (const json::exception& e)
        {
            std::cerr << "[WeatherApp] JSON error: " << e.what() << std::endl;
            return false;
        }
    }

    bool WeatherApp::parseForecastResponse(const std::string& responseBody, int days)
    {
        // 和风天气预报 API 响应格式：
        // {
        //   "code": "200",
        //   "daily": [
        //     {
        //       "fxDate": "2024-01-01",
        //       "textDay": "晴",
        //       "iconDay": "100",
        //       "tempMax": "28",
        //       "tempMin": "18",
        //       "windDirDay": "北风",
        //       "windSpeedDay": "10"
        //     },
        //     ...
        //   ]
        // }

        try
        {
            json j = json::parse(responseBody);

            // 检查响应码
            if (!j.contains("code") || j["code"] != "200")
            {
                std::cerr << "[WeatherApp] Forecast API returned error code: " << j.value("code", "unknown")
                          << std::endl;
                return false;
            }

            // 解析 "daily" 数组
            if (!j.contains("daily") || !j["daily"].is_array())
            {
                std::cerr << "[WeatherApp] Missing 'daily' array in response" << std::endl;
                return false;
            }

            const auto& daily = j["daily"];
            int count = std::min(days, static_cast<int>(daily.size()));
            std::vector<ForecastData> parsedForecast;
            parsedForecast.reserve(static_cast<size_t>(count));

            for (int i = 0; i < count; ++i)
            {
                const auto& day = daily[i];

                // 辅助函数：安全获取字符串值
                auto getStringValue = [&day](const std::string& key, const std::string& defaultVal = "") -> std::string
                {
                    if (day.contains(key) && !day[key].is_null())
                    {
                        if (day[key].is_string())
                            return day[key].get<std::string>();
                        else if (day[key].is_number())
                            return std::to_string(day[key].get<float>());
                    }
                    return defaultVal;
                };

                // 辅助函数：安全获取浮点值（API 返回的是字符串格式的数字）
                auto getFloatValue = [&day](const std::string& key, float defaultVal = 0.0f) -> float
                {
                    if (day.contains(key) && !day[key].is_null())
                    {
                        try
                        {
                            if (day[key].is_string())
                                return std::stof(day[key].get<std::string>());
                            else if (day[key].is_number())
                                return day[key].get<float>();
                        }
                        catch (...)
                        {
                            return defaultVal;
                        }
                    }
                    return defaultVal;
                };

                ForecastData forecast;
                forecast.date = getStringValue("fxDate");
                forecast.weather = getStringValue("textDay");
                forecast.icon = getStringValue("iconDay");
                forecast.tempHigh = getFloatValue("tempMax");
                forecast.tempLow = getFloatValue("tempMin");
                forecast.windDirection = getStringValue("windDirDay");
                forecast.windSpeed = getFloatValue("windSpeedDay");

                parsedForecast.push_back(forecast);
            }

            if (parsedForecast.empty())
            {
                return false;
            }

            {
                std::lock_guard<std::mutex> lock(m_dataMutex);
                m_forecast = std::move(parsedForecast);
            }

            return true;
        }
        catch (const json::parse_error& e)
        {
            std::cerr << "[WeatherApp] JSON parse error: " << e.what() << std::endl;
            return false;
        }
        catch (const json::exception& e)
        {
            std::cerr << "[WeatherApp] JSON error: " << e.what() << std::endl;
            return false;
        }
    }

    void WeatherApp::notifyDataUpdated()
    {
        if (m_dataUpdatePending.exchange(true))
        {
            return;
        }

        if (lv_async_call(asyncDataUpdatedThunk, this) != LV_RES_OK)
        {
            m_dataUpdatePending = false;
            std::cerr << "[WeatherApp] Failed to schedule UI data update" << std::endl;
        }
    }

    void WeatherApp::notifyError(const std::string& msg)
    {
        {
            std::lock_guard<std::mutex> lock(m_errorMutex);
            m_pendingError = msg;
        }

        if (m_errorPending.exchange(true))
        {
            return;
        }

        if (lv_async_call(asyncErrorThunk, this) != LV_RES_OK)
        {
            m_errorPending = false;
            std::cerr << "[WeatherApp] Failed to schedule UI error update" << std::endl;
        }
    }

    void WeatherApp::dispatchDataUpdatedOnUi()
    {
        m_dataUpdatePending = false;

        DataUpdatedCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callback = m_onDataUpdated;
        }

        if (callback)
        {
            callback();
        }
    }

    void WeatherApp::dispatchErrorOnUi()
    {
        m_errorPending = false;

        std::string pendingError;
        {
            std::lock_guard<std::mutex> lock(m_errorMutex);
            pendingError = m_pendingError;
        }

        ErrorCallback callback;
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callback = m_onError;
        }

        if (callback)
        {
            callback(pendingError);
        }
        else
        {
            std::cerr << "[WeatherApp Error] " << pendingError << std::endl;
        }
    }

    void WeatherApp::asyncDataUpdatedThunk(void* userData)
    {
        auto* self = static_cast<WeatherApp*>(userData);
        if (self)
        {
            self->dispatchDataUpdatedOnUi();
        }
    }

    void WeatherApp::asyncErrorThunk(void* userData)
    {
        auto* self = static_cast<WeatherApp*>(userData);
        if (self)
        {
            self->dispatchErrorOnUi();
        }
    }

    // ==========================================
    // 图标转换
    // ==========================================

    std::string WeatherApp::getWeatherIconStr(const std::string& iconCode, bool useFill)
    {
        // 图标代码到 UTF-8 字符的映射表
        // 基于 qweather-icons.ttf 字体文件
        // Unicode 范围: 0xF101 - 0xF314

        static const struct
        {
            int code;
            const char* icon;
            const char* iconFill;
        } iconMap[] = {
            // 晴/多云/阴
            {100, QW_ICON_100, QW_ICON_100_fill}, // 晴
            {101, QW_ICON_101, QW_ICON_101_fill}, // 多云
            {102, QW_ICON_102, QW_ICON_102_fill}, // 少云
            {103, QW_ICON_103, QW_ICON_103_fill}, // 晴间多云
            {104, QW_ICON_104, QW_ICON_104_fill}, // 阴
            {150, QW_ICON_150, QW_ICON_150_fill}, // 晴（夜间）
            {151, QW_ICON_151, QW_ICON_151_fill}, // 多云（夜间）
            {152, QW_ICON_152, QW_ICON_152_fill}, // 少云（夜间）
            {153, QW_ICON_153, QW_ICON_153_fill}, // 晴间多云（夜间）

            // 雨
            {300, QW_ICON_300, QW_ICON_300_fill}, // 阵雨
            {301, QW_ICON_301, QW_ICON_301_fill}, // 强阵雨
            {302, QW_ICON_302, QW_ICON_302_fill}, // 雷阵雨
            {303, QW_ICON_303, QW_ICON_303_fill}, // 强雷阵雨
            {304, QW_ICON_304, QW_ICON_304_fill}, // 雷阵雨伴有冰雹
            {305, QW_ICON_305, QW_ICON_305_fill}, // 小雨
            {306, QW_ICON_306, QW_ICON_306_fill}, // 中雨
            {307, QW_ICON_307, QW_ICON_307_fill}, // 大雨
            {308, QW_ICON_308, QW_ICON_308_fill}, // 极端降雨
            {309, QW_ICON_309, QW_ICON_309_fill}, // 暴雨
            {310, QW_ICON_310, QW_ICON_310_fill}, // 大暴雨
            {311, QW_ICON_311, QW_ICON_311_fill}, // 特大暴雨
            {312, QW_ICON_312, QW_ICON_312_fill}, // 暴雨到大暴雨
            {313, QW_ICON_313, QW_ICON_313_fill}, // 大暴雨到特大暴雨
            {314, QW_ICON_314, QW_ICON_314_fill}, // 小雨-中雨
            {315, QW_ICON_315, QW_ICON_315_fill}, // 中雨-大雨
            {316, QW_ICON_316, QW_ICON_316_fill}, // 大雨-暴雨
            {317, QW_ICON_317, QW_ICON_317_fill}, // 暴雨-大暴雨
            {318, QW_ICON_318, QW_ICON_318_fill}, // 大暴雨-特大暴雨
            {350, QW_ICON_350, QW_ICON_350_fill}, // 阵雨（夜间）
            {351, QW_ICON_351, QW_ICON_351_fill}, // 强阵雨（夜间）
            {399, QW_ICON_399, QW_ICON_399_fill}, // 雨

            // 雪
            {400, QW_ICON_400, QW_ICON_400_fill}, // 小雪
            {401, QW_ICON_401, QW_ICON_401_fill}, // 中雪
            {402, QW_ICON_402, QW_ICON_402_fill}, // 大雪
            {403, QW_ICON_403, QW_ICON_403_fill}, // 暴雪
            {404, QW_ICON_404, QW_ICON_404_fill}, // 雨夹雪
            {405, QW_ICON_405, QW_ICON_405_fill}, // 雨雪天气
            {406, QW_ICON_406, QW_ICON_406_fill}, // 雨夹雪伴有冰雹
            {407, QW_ICON_407, QW_ICON_407_fill}, // 阵雪
            {408, QW_ICON_408, QW_ICON_408_fill}, // 小雪-中雪
            {409, QW_ICON_409, QW_ICON_409_fill}, // 中雪-大雪
            {410, QW_ICON_410, QW_ICON_410_fill}, // 大雪-暴雪
            {456, QW_ICON_456, QW_ICON_456_fill}, // 阵雪（夜间）
            {457, QW_ICON_457, QW_ICON_457_fill}, // 阵雪（夜间）
            {499, QW_ICON_499, QW_ICON_499_fill}, // 雪

            // 雾/霾/风
            {500, QW_ICON_500, QW_ICON_500_fill}, // 轻雾
            {501, QW_ICON_501, QW_ICON_501_fill}, // 雾
            {502, QW_ICON_502, QW_ICON_502},      // 中雾（无fill版本）
            {503, QW_ICON_503, QW_ICON_503_fill}, // 大雾
            {504, QW_ICON_504, QW_ICON_504_fill}, // 特大雾
            {507, QW_ICON_507, QW_ICON_507},      // 霾（无fill版本）
            {508, QW_ICON_508, QW_ICON_508_fill}, // 中霾
            {509, QW_ICON_509, QW_ICON_509_fill}, // 大霾
            {510, QW_ICON_510, QW_ICON_510_fill}, // 特大霾
            {511, QW_ICON_511, QW_ICON_511_fill}, // 浓雾
            {512, QW_ICON_512, QW_ICON_512_fill}, // 强浓雾
            {513, QW_ICON_513, QW_ICON_513_fill}, // 中霾-大霾
            {514, QW_ICON_514, QW_ICON_514_fill}, // 大霾-特大霾
            {515, QW_ICON_515, QW_ICON_515_fill}, // 特大霾

            // 沙尘/浮尘（无fill版本）
            {800, QW_ICON_800, QW_ICON_800}, // 浮尘
            {801, QW_ICON_801, QW_ICON_801}, // 扬沙
            {802, QW_ICON_802, QW_ICON_802}, // 沙尘暴
            {803, QW_ICON_803, QW_ICON_803}, // 强沙尘暴
            {804, QW_ICON_804, QW_ICON_804}, // 特强沙尘暴
            {805, QW_ICON_805, QW_ICON_805}, // 浮尘（夜间）
            {806, QW_ICON_806, QW_ICON_806}, // 扬沙（夜间）
            {807, QW_ICON_807, QW_ICON_807}, // 沙尘暴（夜间）

            // 其他
            {900, QW_ICON_900, QW_ICON_900_fill}, // 热
            {901, QW_ICON_901, QW_ICON_901_fill}, // 冷
            {999, QW_ICON_999, QW_ICON_999_fill}, // 未知
        };

        // 解析图标代码
        int code = 0;
        try
        {
            code = std::stoi(iconCode);
        }
        catch (...)
        {
            return QW_ICON_999; // 返回未知图标
        }

        // 查找匹配的图标
        for (const auto& item : iconMap)
        {
            if (item.code == code)
            {
                return useFill ? item.iconFill : item.icon;
            }
        }

        // 未找到则返回未知图标
        return QW_ICON_999;
    }

} // namespace app
