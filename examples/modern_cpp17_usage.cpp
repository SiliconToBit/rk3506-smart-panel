/**
 * @file sensor_usage_example.cpp
 * @brief 演示如何使用现代C++17风格的DeviceNode API
 *
 * 展示旧API和新API的对比
 */

#include "hal/DeviceNode.h"
#include <iostream>
#include <vector>
#include <memory>

namespace examples
{

    // ============================================================================
    // 示例 1: 基础使用 - 读取传感器数据
    // ============================================================================

    void example_basic_read()
    {
        hal::DeviceNode temp_sensor;

        // 新 API：使用 std::optional<std::error_code> 获取具体错误
        if (auto err = temp_sensor.open("/sys/class/hwmon/hwmon0/temp1_input"))
        {
            std::cerr << "打开失败: " << err->message() << " (errno: " << err->value() << ")\n";
            return;
        }

        // 读取温度值，使用 std::optional<std::string>
        if (auto temp = temp_sensor.readString())
        {
            std::cout << "当前温度: " << *temp << " (单位: 毫摄氏度)\n";
        }
        else
        {
            std::cerr << "读取温度失败\n";
        }

        // RAII 自动关闭文件描述符
    } // temp_sensor 析构时自动调用 close()

    // ============================================================================
    // 示例 2: 写入数据 - 控制 LED
    // ============================================================================

    void example_led_control()
    {
        hal::DeviceNode led;

        std::string_view led_path = "/sys/class/leds/led0/brightness";

        if (auto err = led.open(led_path))
        {
            std::cerr << "打开 LED 失败: " << err->message() << "\n";
            return;
        }

        // 打开 LED（写入 "1"）
        if (auto err = led.writeString("1"))
        {
            std::cerr << "LED 打开失败: " << err->message() << "\n";
            return;
        }
        std::cout << "LED 已打开\n";

        // 关闭 LED（写入 "0"）
        if (auto err = led.writeString("0"))
        {
            std::cerr << "LED 关闭失败: " << err->message() << "\n";
            return;
        }
        std::cout << "LED 已关闭\n";
    }

    // ============================================================================
    // 示例 3: Move 语义 - 传递设备所有权
    // ============================================================================

    // 创建并返回一个打开的设备（利用 Move 语义避免拷贝）
    hal::DeviceNode create_and_open_device(std::string_view path)
    {
        hal::DeviceNode device;
        if (auto err = device.open(path))
        {
            std::cerr << "设备打开失败: " << err->message() << "\n";
        }
        // 使用 Move 构造函数返回，无额外拷贝
        return device;
    }

    void example_move_semantics()
    {
        // Move 返回值，设备所有权转移到 sensor
        auto sensor = create_and_open_device("/dev/sensor0");

        if (sensor.isOpen())
        {
            std::cout << "传感器已打开: " << sensor.path() << "\n";
            // 可以继续使用 sensor...
        }
    } // sensor 析构时安全释放资源

    // ============================================================================
    // 示例 4: 多设备管理 - 利用容器和 Move 语义
    // ============================================================================

    class SensorManager
    {
    private:
        std::vector<hal::DeviceNode> sensors;

    public:
        // 添加新传感器（使用 Move 传递所有权）
        bool add_sensor(std::string_view path)
        {
            hal::DeviceNode device;

            if (auto err = device.open(path))
            {
                std::cerr << "打开 " << path << " 失败: " << err->message() << "\n";
                return false;
            }

            // Move 语义：高效地转移设备所有权
            sensors.push_back(std::move(device));
            std::cout << "已添加传感器: " << path << "\n";
            return true;
        }

        // 读取所有传感器的值
        void read_all()
        {
            for (size_t i = 0; i < sensors.size(); ++i)
            {
                if (auto value = sensors[i].readString())
                {
                    std::cout << "传感器 [" << i << "] " << sensors[i].path() << " = " << *value << "\n";
                }
                else
                {
                    std::cerr << "读取传感器 [" << i << "] 失败\n";
                }
            }
        }
    };

    void example_multiple_sensors()
    {
        SensorManager manager;

        // 添加多个传感器
        manager.add_sensor("/sys/class/hwmon/hwmon0/temp1_input");
        manager.add_sensor("/sys/class/hwmon/hwmon0/temp2_input");
        manager.add_sensor("/sys/class/hwmon/hwmon0/temp3_input");

        // 读取所有传感器
        manager.read_all();

        // 构造函数析构时，所有 DeviceNode 对象自动清理资源
    }

    // ============================================================================
    // 示例 5: 错误处理最佳实践
    // ============================================================================

    bool safe_device_operation(std::string_view path)
    {
        hal::DeviceNode device;

        // 使用习语：if (auto err = operation()) 检查错误
        if (auto err = device.open(path))
        {
            // err 包含完整的错误信息
            std::cerr << "设备操作失败\n"
                      << "  路径: " << path << "\n"
                      << "  错误: " << err->message() << "\n"
                      << "  错误代码: " << err->value() << "\n";
            return false;
        }

        if (auto data = device.readString())
        {
            std::cout << "成功读取: " << *data << "\n";
            return true;
        }
        else
        {
            std::cerr << "读取失败 (errno: " << errno << ")\n";
            return false;
        }
    }

    // ============================================================================
    // 示例 6: 对比：旧 API vs 新 API
    // ============================================================================

    // 这个函数展示旧 API 的问题（仅作参考，演示用）
    /*
    void old_api_problems()
    {
        // 问题 1: 布尔返回不提供错误信息
        device.open("/dev/invalid");  // 失败，但不知道原因

        // 问题 2: 空字符串可能表示读取失败或真的是空字符串？
        std::string result = device.readString();
        if (result.empty()) {
            // 这是真的空？还是读取失败？我们无法区分！
        }

        // 问题 3: 无法使用 Move 语义
        // 容器不能安全地存储不可移动的对象
    }
    */

    void demonstrate_old_vs_new()
    {
        std::cout << "\n=== 新 API 优势 ===\n";

        hal::DeviceNode dev;

        // 优势 1: 清晰的错误处理
        std::cout << "1. 错误处理:\n";
        if (auto err = dev.open("/nonexistent"))
        {
            std::cout << "   - 具体错误: " << err->message() << "\n";
            std::cout << "   - 错误代码: " << err->value() << "\n";
        }

        // 优势 2: 无歧义的可选值
        std::cout << "2. 可选返回值:\n";
        if (auto result = dev.readString())
        {
            std::cout << "   - 有值: " << *result << "\n";
        }
        else
        {
            std::cout << "   - 明确的空值（读取失败）\n";
        }

        // 优势 3: 零拷贝参数
        std::cout << "3. 零拷贝参数 (std::string_view):\n";
        const char* c_str = "/dev/i2c-0";
        std::string cpp_str = "/dev/spi-0";
        // 无论是什么字符串源，都不会有额外拷贝
        dev.open(c_str);       // 直接使用 C 字符串
        dev.open(cpp_str);     // 直接使用 std::string
        dev.open("/dev/gpio"); // 直接使用字符串字面量

        // 优势 4: Move 语义
        std::cout << "4. Move 语义支持:\n";
        std::vector<hal::DeviceNode> v;
        v.push_back(std::move(dev)); // 高效转移
        std::cout << "   - 设备成功存入容器\n";
    }

} // namespace examples

// 主函数（可选）
int main()
{
    std::cout << "=== DeviceNode C++17 现代 API 示例 ===\n\n";

    examples::demonstrate_old_vs_new();

    // 取消注释以运行其他示例（需要实际的设备）
    // examples::example_basic_read();
    // examples::example_led_control();
    // examples::example_move_semantics();
    // examples::example_multiple_sensors();

    return 0;
}
