/**
 * @file Dht11.cpp
 * @brief DHT11 温湿度传感器实现
 */

#include "Dht11.h"
#include <fcntl.h>
#include <cstdio>
#include <array>

namespace sensor
{

    Dht11::Dht11(std::string_view devPath)
        : m_devPath(devPath)
    {
    }

    Dht11::~Dht11()
    {
        deinit();
    }

    bool Dht11::init() noexcept
    {
        if (m_init)
        {
            return true;
        }

        m_init = !m_devNode.open(m_devPath, O_RDONLY).has_value();
        return m_init;
    }

    void Dht11::deinit() noexcept
    {
        if (m_init)
        {
            m_devNode.close();
            m_init = false;
        }
    }

    bool Dht11::isInit() const noexcept
    {
        return m_init;
    }

    bool Dht11::read(DhtData& data) noexcept
    {
        if (!m_init)
        {
            data.valid = false;
            return false;
        }

        // 检查缓存是否有效（1秒内且数据有效）
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastReadTime);
        if (elapsed.count() < 1 && m_lastData.valid)
        {
            data = m_lastData;
            return true;
        }
        m_lastReadTime = now;

        // 从设备文件读取数据
        std::array<char, 32> buf{};
        ssize_t bytesRead = 0;
        auto err = m_devNode.read(buf.data(), buf.size() - 1, bytesRead);

        if (err || bytesRead <= 0)
        {
            data.valid = false;
            return false;
        }

        // 解析 "温度 湿度" 格式
        int temp = 0;
        int humi = 0;
        if (std::sscanf(buf.data(), "%d %d", &temp, &humi) == 2)
        {
            data.temperature = temp;
            data.humidity = humi;
            data.valid = true;
            m_lastData = data;
            return true;
        }

        data.valid = false;
        return false;
    }

    std::optional<float> Dht11::getTemperature() const noexcept
    {
        if (!m_lastData.valid)
        {
            return std::nullopt;
        }

        // DHT11 输出整数温度值
        return static_cast<float>(m_lastData.temperature);
    }

    std::optional<float> Dht11::getHumidity() const noexcept
    {
        if (!m_lastData.valid)
        {
            return std::nullopt;
        }

        // DHT11 输出整数湿度值
        return static_cast<float>(m_lastData.humidity);
    }

    const std::string& Dht11::devicePath() const noexcept
    {
        return m_devPath;
    }

} // namespace sensor