#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include "configs/AppConfig.h" // 天气 API 配置

namespace hal
{
    class HttpClient;
}

namespace app
{

    /**
     * @brief 当前天气数据结构
     */
    struct WeatherData
    {
        std::string city;          ///< 城市名
        std::string weather;       ///< 天气状况（晴、多云、雨...）
        std::string icon;          ///< 天气图标代码
        float temperature = 0.0f;  ///< 当前温度（摄氏度）
        float humidity = 0.0f;     ///< 湿度（%）
        float windSpeed = 0.0f;    ///< 风速（km/h）
        std::string windDirection; ///< 风向
        std::string updateTime;    ///< 更新时间
        bool valid = false;        ///< 数据是否有效
    };

    /**
     * @brief 天气预报数据结构
     */
    struct ForecastData
    {
        std::string date;          ///< 日期
        std::string icon;          ///< 天气图标代码
        float tempHigh = 0.0f;     ///< 最高温度
        float tempLow = 0.0f;      ///< 最低温度
        std::string weather;       ///< 天气状况
        std::string windDirection; ///< 风向
        float windSpeed = 0.0f;    ///< 风速
    };

    /**
     * @brief 天气应用 - 管理天气数据获取和显示
     *
     * 职责：
     * - 城市管理
     * - 天气数据获取（通过 HTTP API）
     * - 数据缓存
     * - UI 回调通知
     *
     * 使用示例：
     * @code
     * // 在 eez_actions.cpp 中调用
     * void action_init_weather_screen(lv_event_t* e) {
     *     auto& app = app::WeatherApp::getInstance();
     *
     *     // 设置数据更新回调
     *     app.setOnDataUpdated([]() {
     *         auto weather = app.getCurrentWeather();
     *         lv_label_set_text_fmt(temp_label, "%.1f°C", weather.temperature);
     *         lv_label_set_text(weather_label, weather.weather.c_str());
     *     });
     *
     *     // 设置城市并刷新
     *     app.setCity("广东", "深圳");
     *     app.refreshAsync();
     * }
     *
     * void action_confirm_city_picker(lv_event_t* e) {
     *     auto& app = app::WeatherApp::getInstance();
     *     app.setCity(selectedProvince, selectedCity);
     *     app.refreshAsync();
     * }
     * @endcode
     */
    class WeatherApp
    {
    public:
        // ========== 单例访问 ==========

        /**
         * @brief 获取单例实例
         * @return WeatherApp 单例引用
         */
        static WeatherApp& getInstance();

        // ========== 城市管理 ==========

        /**
         * @brief 设置当前城市（使用城市代码）
         * @param locationId 和风天气城市代码（如：101250412 为衡阳常宁）
         * @param cityName 城市名称（用于显示）
         */
        void setLocation(const std::string& locationId, const std::string& cityName = "");

        /**
         * @brief 设置当前城市（使用省市名称）
         * @param province 省份
         * @param city 城市
         * @note 内部会转换为城市代码
         */
        void setCity(const std::string& province, const std::string& city);

        /**
         * @brief 获取当前城市代码
         * @return 和风天气城市代码
         */
        [[nodiscard]] std::string getLocationId() const;

        /**
         * @brief 获取当前省份
         * @return 省份名称
         */
        [[nodiscard]] std::string getProvince() const;

        /**
         * @brief 获取当前城市
         * @return 城市名称
         */
        [[nodiscard]] std::string getCity() const;

        /**
         * @brief 获取完整城市名（省+市）
         * @return 完整城市名
         */
        [[nodiscard]] std::string getFullCityName() const;

        // ========== 数据获取 ==========

        /**
         * @brief 同步刷新所有天气数据（实时+预报）
         * @return 是否成功
         */
        bool refresh();

        /**
         * @brief 异步刷新所有天气数据
         *
         * 刷新完成后会调用 setOnDataUpdated 设置的回调
         */
        void refreshAsync();

        /**
         * @brief 仅刷新实时天气
         * @return 是否成功
         */
        bool refreshCurrentWeather();

        /**
         * @brief 异步刷新实时天气
         */
        void refreshCurrentWeatherAsync();

        /**
         * @brief 刷新3天预报
         * @return 是否成功
         */
        bool refreshForecast3D();

        /**
         * @brief 异步刷新3天预报
         */
        void refreshForecast3DAsync();

        /**
         * @brief 刷新7天预报
         * @return 是否成功
         */
        bool refreshForecast7D();

        /**
         * @brief 异步刷新7天预报
         */
        void refreshForecast7DAsync();

        // ========== 数据查询 ==========

        /**
         * @brief 获取当前天气数据
         * @return 当前天气数据
         */
        [[nodiscard]] WeatherData getCurrentWeather() const;

        /**
         * @brief 获取天气预报数据
         * @return 预报数据列表（通常3-7天）
         */
        [[nodiscard]] std::vector<ForecastData> getForecast() const;

        /**
         * @brief 检查数据是否有效
         * @return true 表示数据有效
         */
        [[nodiscard]] bool hasValidData() const;

        /**
         * @brief 获取上次更新时间
         * @return 更新时间字符串
         */
        [[nodiscard]] std::string getLastUpdateTime() const;

        // ========== 图标转换 ==========

        /**
         * @brief 根据天气图标代码获取对应的图标字符串
         * @param iconCode 和风天气图标代码（如 "100", "101", "305"）
         * @param useFill 是否使用填充样式图标（默认 false）
         * @return 图标字符串（UTF-8 编码），可直接用于 LVGL label
         *
         * @note 需要配合 qweather-icons.ttf 字体使用
         *
         * 使用示例：
         * @code
         * // 在 UI 中显示天气图标
         * auto weather = WeatherApp::getInstance().getCurrentWeather();
         * std::string iconStr = WeatherApp::getWeatherIconStr(weather.icon);
         * lv_label_set_text(icon_label, iconStr.c_str());
         * // 设置字体为 qweather-icons
         * lv_obj_set_style_text_font(icon_label, &font_qweather_48, 0);
         * @endcode
         */
        static std::string getWeatherIconStr(const std::string& iconCode, bool useFill = false);

        // ========== UI 回调 ==========

        /**
         * @brief 数据更新回调类型
         */
        using DataUpdatedCallback = std::function<void()>;

        /**
         * @brief 错误回调类型
         * @param errorMsg 错误消息
         */
        using ErrorCallback = std::function<void(const std::string& errorMsg)>;

        /**
         * @brief 设置数据更新回调
         * @param callback 回调函数
         */
        void setOnDataUpdated(DataUpdatedCallback callback);

        /**
         * @brief 设置错误回调
         * @param callback 回调函数
         */
        void setOnError(ErrorCallback callback);

    private:
        WeatherApp();
        ~WeatherApp();

        // 禁止拷贝
        WeatherApp(const WeatherApp&) = delete;
        WeatherApp& operator=(const WeatherApp&) = delete;

        // 内部方法
        void setupHttpClient();
        bool parseCurrentWeatherResponse(const std::string& responseBody);
        bool parseForecastResponse(const std::string& responseBody, int days);
        void notifyDataUpdated();
        void notifyError(const std::string& msg);
        void dispatchDataUpdatedOnUi();
        void dispatchErrorOnUi();
        static void asyncDataUpdatedThunk(void* userData);
        static void asyncErrorThunk(void* userData);

        // URL 构建
        std::string buildNowApiUrl() const;
        std::string build3DApiUrl() const;
        std::string build7DApiUrl() const;

        // 成员变量
        hal::HttpClient& m_httpClient; // 引用 HttpClient 单例

        std::string m_province;
        std::string m_city;
        std::string m_locationId; ///< 和风天气城市代码

        mutable std::mutex m_dataMutex;
        WeatherData m_currentWeather;
        std::vector<ForecastData> m_forecast;

        // 回调
        std::mutex m_callbackMutex;
        DataUpdatedCallback m_onDataUpdated;
        ErrorCallback m_onError;

        std::atomic<bool> m_dataUpdatePending{false};
        std::atomic<bool> m_errorPending{false};
        std::mutex m_errorMutex;
        std::string m_pendingError;
    };

} // namespace app
