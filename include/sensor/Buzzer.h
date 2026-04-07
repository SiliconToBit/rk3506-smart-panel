#pragma once

#include <optional>
#include <string>
#include <string_view>
#include "hal/DeviceNode.h"

namespace sensor
{
    class Buzzer
    {
    public:
        explicit Buzzer(std::string_view devPath = "/dev/buzzer");
        ~Buzzer();

        Buzzer(const Buzzer&) = delete;
        Buzzer& operator=(const Buzzer&) = delete;
        Buzzer(Buzzer&&) = delete;
        Buzzer& operator=(Buzzer&&) = delete;

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
         * @brief 打开蜂鸣器
         * @return true 成功
         */
        [[nodiscard]] bool setOn();

        /**
         * @brief 关闭蜂鸣器
         * @return true 成功
         */
        [[nodiscard]] bool setOff();

        /**
         * @brief 切换蜂鸣器状态
         * @return true 成功
         */
        [[nodiscard]] bool toggle();

        /**
         * @brief 检查蜂鸣器是否开启
         * @return true 开启
         */
        [[nodiscard]] bool isOn() const;

        /**
         * @brief 蜂鸣器响指定时间
         * @param durationMs 响铃持续时间（毫秒）
         * @return true 成功
         */
        [[nodiscard]] bool beep(int durationMs) noexcept;

        /**
         * @brief 蜂鸣器按照指定模式响铃
         * @param onMs 响铃持续时间（毫秒）
         * @param offMs 间隔时间（毫秒）
         * @param count 响铃次数
         * @return true 成功
         */
        [[nodiscard]] bool beepPattern(int onMs, int offMs, int count) noexcept;

    private:
        hal::DeviceNode m_devNode;
        std::string m_devPath;
        bool m_init{false};
        bool m_isOn{false};

        bool writeState(int value) noexcept;
    };
} // namespace sensor