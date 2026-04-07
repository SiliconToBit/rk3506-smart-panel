/**
 * @file Dht11.h
 * @brief DHT11 温湿度传感器驱动
 *
 * 通过字符设备节点 /dev/dht11 读取数据
 * DHT11 规格：温度 0-50°C，湿度 20-80%
 */

#pragma once

#include "hal/DeviceNode.h"
#include <optional>
#include <string>
#include <string_view>
#include <chrono>

namespace sensor
{

    /**
     * @brief DHT数据结构
     */
    struct DhtData
    {
        int temperature{0}; // 温度 (°C，整数)
        int humidity{0};    // 湿度 (%，整数)
        bool valid{false};  // 数据有效标志
    };

    /**
     * @brief DHT11 传感器类
     *
     * 通过字符设备节点读取数据，如 /dev/dht11
     */
    class Dht11
    {
    public:
        /**
         * @brief 构造函数
         * @param devPath 字符设备路径，如 /dev/dht11
         */
        explicit Dht11(std::string_view devPath = "/dev/dht11");

        ~Dht11();

        // 禁止拷贝和移动
        Dht11(const Dht11&) = delete;
        Dht11& operator=(const Dht11&) = delete;
        Dht11(Dht11&&) = delete;
        Dht11& operator=(Dht11&&) = delete;

        /**
         * @brief 初始化传感器（打开设备节点）
         * @return true 成功
         */
        [[nodiscard]] bool init() noexcept;

        /**
         * @brief 反初始化（关闭设备节点）
         */
        void deinit() noexcept;

        /**
         * @brief 是否已初始化
         */
        [[nodiscard]] bool isInit() const noexcept;

        /**
         * @brief 读取温湿度
         * @param data 输出数据
         * @return true 成功
         */
        [[nodiscard]] bool read(DhtData& data) noexcept;

        /**
         * @brief 获取温度（摄氏度）
         * @return 温度值，失败返回 std::nullopt
         */
        [[nodiscard]] std::optional<float> getTemperature() const noexcept;

        /**
         * @brief 获取湿度（百分比）
         * @return 湿度值，失败返回 std::nullopt
         */
        [[nodiscard]] std::optional<float> getHumidity() const noexcept;

        /**
         * @brief 获取设备路径
         */
        [[nodiscard]] const std::string& devicePath() const noexcept;

    private:
        hal::DeviceNode m_devNode;
        std::string m_devPath;
        DhtData m_lastData;
        std::chrono::steady_clock::time_point m_lastReadTime{};
        bool m_init{false};
    };

} // namespace sensor
