/**
 * @file Mpu6050.h
 * @brief MPU6050 六轴传感器驱动
 *
 * 通过字符设备节点 /dev/mpu6050 读取数据
 */

#pragma once

#include "hal/DeviceNode.h"
#include <cstdint>
#include <string>

namespace sensor
{

    /**
     * @brief MPU6050 数据结构（内核驱动返回格式）
     */
    struct Mpu6050Data
    {
        int16_t accelX{0}; // 加速度 X 原始值
        int16_t accelY{0}; // 加速度 Y 原始值
        int16_t accelZ{0}; // 加速度 Z 原始值
        int16_t gyroX{0};  // 陀螺仪 X 原始值
        int16_t gyroY{0};  // 陀螺仪 Y 原始值
        int16_t gyroZ{0};  // 陀螺仪 Z 原始值
        int16_t temp{0};   // 温度原始值
        bool valid{false}; // 数据有效标志
    };

    /**
     * @brief MPU6050 传感器类
     *
     * 通过字符设备节点读取数据，如 /dev/mpu6050
     */
    class Mpu6050
    {
    public:
        /**
         * @brief 构造函数
         * @param devicePath 字符设备路径，如 /dev/mpu6050
         */
        explicit Mpu6050(const std::string& devicePath = "/dev/mpu6050");

        ~Mpu6050();

        // 禁止拷贝和移动
        Mpu6050(const Mpu6050&) = delete;
        Mpu6050& operator=(const Mpu6050&) = delete;
        Mpu6050(Mpu6050&&) = delete;
        Mpu6050& operator=(Mpu6050&&) = delete;

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
         * @brief 读取传感器原始数据
         * @param data 输出数据
         * @return true 成功
         */
        [[nodiscard]] bool read(Mpu6050Data& data);

        /**
         * @brief 获取加速度 (g)
         * @param x,y,z 输出加速度值
         * @param scale 缩放因子，默认16384 (±2g)
         * @return true 成功
         */
        [[nodiscard]] bool getAccel(float& x, float& y, float& z, float scale = 16384.0f) const noexcept;

        /**
         * @brief 获取陀螺仪 (°/s)
         * @param x,y,z 输出陀螺仪值
         * @param scale 缩放因子，默认131 (±250°/s)
         * @return true 成功
         */
        [[nodiscard]] bool getGyro(float& x, float& y, float& z, float scale = 131.0f) const noexcept;

        /**
         * @brief 获取温度 (°C)
         * @return 温度值，失败返回NaN
         */
        [[nodiscard]] float getTemperature() const noexcept;

        /**
         * @brief 获取设备路径
         */
        [[nodiscard]] const std::string& devicePath() const noexcept;

    private:
        hal::DeviceNode dev_;
        std::string devicePath_;
        Mpu6050Data lastData_; // 最近一次读取的数据
        bool init_{false};
    };

} // namespace sensor
