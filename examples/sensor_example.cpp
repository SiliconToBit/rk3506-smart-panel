/**
 * @file sensor_example.cpp
 * @brief 传感器使用示例
 */

#include "hal/DeviceNode.h"
#include "hal/I2cBus.h"
#include "sensor/Mpu6050.h"
#include "sensor/Dht11.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace hal;
using namespace sensor;

/**
 * @brief 示例1：使用DeviceNode读取sysfs温度
 */
void example_sysfs_temperature()
{
    std::cout << "=== 示例1：读取sysfs温度 ===" << std::endl;

    DeviceNode dev;

    // 打开CPU温度传感器
    if (dev.open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY))
    {
        std::string value = dev.readString();
        int millidegrees = std::stoi(value);
        float celsius = millidegrees / 1000.0f;
        std::cout << "CPU温度: " << celsius << "°C" << std::endl;
        dev.close();
    }
    else
    {
        std::cout << "无法打开温度传感器" << std::endl;
    }
}

/**
 * @brief 示例2：使用I2cBus读取I2C设备
 */
void example_i2c_raw()
{
    std::cout << "\n=== 示例2：I2C原始读写 ===" << std::endl;

    I2cBus i2c(0); // I2C总线0

    if (i2c.open())
    {
        // 设置MPU6050地址
        if (i2c.setSlaveAddr(0x68))
        {
            // 读取WHO_AM_I寄存器 (0x75)
            int whoAmI = i2c.readByte(0x75);
            std::cout << "WHO_AM_I: 0x" << std::hex << whoAmI << std::dec << std::endl;
            // MPU6050应该返回0x68
        }
        i2c.close();
    }
    else
    {
        std::cout << "无法打开I2C总线" << std::endl;
    }
}

/**
 * @brief 示例3：使用MPU6050传感器类
 */
void example_mpu6050()
{
    std::cout << "\n=== 示例3：MPU6050传感器 ===" << std::endl;

    Mpu6050 mpu;

    Mpu6050Config config;
    config.i2cBus = 0;
    config.addr = Mpu6050::ADDR_AD0_LOW;
    config.accelRange = 2; // ±8g
    config.gyroRange = 1;  // ±500°/s

    if (mpu.init(config))
    {
        std::cout << "MPU6050初始化成功" << std::endl;

        // 读取数据
        Mpu6050Data data;
        for (int i = 0; i < 5; i++)
        {
            if (mpu.read(data))
            {
                std::cout << "加速度: X=" << data.accelX << "g, "
                          << "Y=" << data.accelY << "g, "
                          << "Z=" << data.accelZ << "g" << std::endl;
                std::cout << "陀螺仪: X=" << data.gyroX << "°/s, "
                          << "Y=" << data.gyroY << "°/s, "
                          << "Z=" << data.gyroZ << "°/s" << std::endl;
                std::cout << "温度: " << data.temp << "°C" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        mpu.deinit();
    }
    else
    {
        std::cout << "MPU6050初始化失败" << std::endl;
    }
}

/**
 * @brief 示例4：使用DHT11温湿度传感器
 */
void example_dht11()
{
    std::cout << "\n=== 示例4：DHT11温湿度传感器 ===" << std::endl;

    // 使用默认设备路径 /dev/dht11
    Dht11 dht;

    if (dht.init())
    {
        std::cout << "DHT11初始化成功" << std::endl;

        // DHT传感器两次读取间隔至少1秒
        for (int i = 0; i < 3; i++)
        {
            DhtData data;
            if (dht.read(data))
            {
                std::cout << "温度: " << data.temperature << "°C, "
                          << "湿度: " << data.humidity << "%" << std::endl;
            }
            else
            {
                std::cout << "读取失败" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        dht.deinit();
    }
    else
    {
        std::cout << "DHT11初始化失败" << std::endl;
    }
}

/**
 * @brief 示例5：读取ADC值
 */
void example_adc()
{
    std::cout << "\n=== 示例5：读取ADC ===" << std::endl;

    DeviceNode adc;

    // IIO ADC设备路径
    if (adc.open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY))
    {
        std::string value = adc.readString();
        int rawValue = std::stoi(value);
        std::cout << "ADC原始值: " << rawValue << std::endl;

        // 转换为电压（假设12位ADC，参考电压3.3V）
        float voltage = rawValue * 3.3f / 4095.0f;
        std::cout << "电压: " << voltage << "V" << std::endl;

        adc.close();
    }
    else
    {
        std::cout << "无法打开ADC" << std::endl;
    }
}

int main()
{
    std::cout << "RK3506 传感器示例\n" << std::endl;

    example_sysfs_temperature();
    example_i2c_raw();
    // example_mpu6050();    // 需要实际硬件
    // example_dht11();      // 需要实际硬件
    example_adc();

    return 0;
}