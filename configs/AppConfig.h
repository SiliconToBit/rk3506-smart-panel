/**
 * @file AppConfig.h
 * @brief 应用配置管理类
 * @details 负责应用的硬件设备路径、应用配置和字体路径的定义
 */

#ifndef LVGL_APP_CONFIG_H
#define LVGL_APP_CONFIG_H

/* ======================== 硬件设备路径 ======================== */
#define APP_DEV_DHT11 "/dev/dht11"
#define APP_DEV_MPU6050 "/dev/mpu6050"
#define APP_DEV_BACKLIGHT "/sys/class/backlight/backlight/"
#define APP_DEV_IR "/dev/ttyS1"

/* ======================== GPIO 设备路径 ======================= */
#define APP_GPIO_LED1 "/dev/led1"
#define APP_GPIO_LED2 "/dev/led2"
#define APP_GPIO_BUZZER "/dev/buzzer"

/* ======================== MQTT 配置 =========================== */
#define APP_MQTT_HOST "192.168.1.87"
#define APP_MQTT_PORT 1883
#define APP_MQTT_CLIENT_ID "rk3506_device"

/* ======================== 传感器上报配置 ====================== */
#define APP_SENSOR_REPORT_INTERVAL 30

/* ======================== 应用配置 ============================ */
#define APP_MUSIC_DIR "/root/Music"
#define APP_DEFAULT_CITY "衡阳/常宁"

/* ======================== 和风天气API配置 ===================== */
#define APP_WEATHER_API_HOST "pu3qqpnwdn.re.qweatherapi.com"
#define APP_WEATHER_API_KEY "4661897787bc49c793207316caf28304"

// 实时天气API URL模板，使用时需要替换 {location} 为城市代码
#define APP_WEATHER_NOW_API_URL "https://" APP_WEATHER_API_HOST "/v7/weather/now?location=%s&key=" APP_WEATHER_API_KEY

// 3天预报API URL模板，使用时需要替换 {location} 为城市代码
#define APP_WEATHER_3D_API_URL "https://" APP_WEATHER_API_HOST "/v7/weather/3d?location=%s&key=" APP_WEATHER_API_KEY

// 7天预报API URL模板（备用）
#define APP_WEATHER_7D_API_URL "https://" APP_WEATHER_API_HOST "/v7/weather/7d?location=%s&key=" APP_WEATHER_API_KEY

// 示例：衡阳常宁（城市代码：101250412）
// 实时天气：https://pu3qqpnwdn.re.qweatherapi.com/v7/weather/now?location=101250412&key=YOUR_KEY
// 3天预报：https://pu3qqpnwdn.re.qweatherapi.com/v7/weather/3d?location=101250412&key=YOUR_KEY

/* ======================== 字体路径 ============================ */
#define APP_FONT_PATH "/usr/share/fonts/AlibabaPuHuiTi-3-115-Black.ttf"
#define APP_WEATHER_FONT_PATH "/usr/share/fonts/qweather-icons.ttf"

#endif
