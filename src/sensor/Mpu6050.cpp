/**
 * @file Mpu6050.cpp
 * @brief MPU6050 六轴传感器实现 - 字符设备节点方式
 */

#include "sensor/Mpu6050.h"
#include <fcntl.h>
#include <cmath>
#include <cstring>

namespace sensor
{

    Mpu6050::Mpu6050(const std::string& devicePath)
        : devicePath_(devicePath)
    {
    }

    Mpu6050::~Mpu6050()
    {
        deinit();
    }

    bool Mpu6050::init() noexcept
    {
        if (init_)
        {
            return true;
        }

        auto err = dev_.open(devicePath_, O_RDONLY);
        init_ = !err.has_value();
        return init_;
    }

    void Mpu6050::deinit() noexcept
    {
        if (init_)
        {
            dev_.close();
            init_ = false;
        }
    }

    bool Mpu6050::isInit() const noexcept
    {
        return init_;
    }

    bool Mpu6050::read(Mpu6050Data& data)
    {
        if (!init_)
        {
            data.valid = false;
            return false;
        }

        // MPU6050 数据格式（14字节）：
        // accelX(2) + accelY(2) + accelZ(2) + temp(2) + gyroX(2) + gyroY(2) + gyroZ(2)
        uint8_t buffer[14];

        ssize_t bytesRead = 0;
        auto err = dev_.read(buffer, sizeof(buffer), bytesRead);
        if (err || bytesRead != sizeof(buffer))
        {
            data.valid = false;
            return false;
        }

        // 解析数据（大端序转小端序）
        data.accelX = static_cast<int16_t>((buffer[0] << 8) | buffer[1]);
        data.accelY = static_cast<int16_t>((buffer[2] << 8) | buffer[3]);
        data.accelZ = static_cast<int16_t>((buffer[4] << 8) | buffer[5]);
        data.temp = static_cast<int16_t>((buffer[6] << 8) | buffer[7]);
        data.gyroX = static_cast<int16_t>((buffer[8] << 8) | buffer[9]);
        data.gyroY = static_cast<int16_t>((buffer[10] << 8) | buffer[11]);
        data.gyroZ = static_cast<int16_t>((buffer[12] << 8) | buffer[13]);
        data.valid = true;

        // 保存最近的数据
        lastData_ = data;

        return true;
    }

    bool Mpu6050::getAccel(float& x, float& y, float& z, float scale) const noexcept
    {
        if (!lastData_.valid)
        {
            return false;
        }

        x = static_cast<float>(lastData_.accelX) / scale;
        y = static_cast<float>(lastData_.accelY) / scale;
        z = static_cast<float>(lastData_.accelZ) / scale;
        return true;
    }

    bool Mpu6050::getGyro(float& x, float& y, float& z, float scale) const noexcept
    {
        if (!lastData_.valid)
        {
            return false;
        }

        x = static_cast<float>(lastData_.gyroX) / scale;
        y = static_cast<float>(lastData_.gyroY) / scale;
        z = static_cast<float>(lastData_.gyroZ) / scale;
        return true;
    }

    float Mpu6050::getTemperature() const noexcept
    {
        if (!lastData_.valid)
        {
            return std::nanf("");
        }

        // MPU6050 温度公式：Temperature = temp / 340.0 + 36.53
        return static_cast<float>(lastData_.temp) / 340.0F + 36.53F;
    }

    const std::string& Mpu6050::devicePath() const noexcept
    {
        return devicePath_;
    }

} // namespace sensor