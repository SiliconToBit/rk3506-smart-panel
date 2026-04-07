/**
 * @file DeviceNode.h
 * @brief HAL层 - 设备节点封装
 *
 * 封装Linux设备文件的 open/read/write/ioctl 操作
 * 支持 sysfs 和 /dev/ 设备节点
 */

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <system_error>
#include <fcntl.h>
#include <unistd.h>

namespace hal
{

    /**
     * @brief 设备节点类
     *
     * 封装Linux设备文件的基础操作，使用RAII管理文件描述符
     * C++17现代设计：使用std::optional处理错误
     */
    class DeviceNode
    {
    public:
        DeviceNode() noexcept;
        ~DeviceNode() noexcept;

        // 禁止拷贝，支持move
        DeviceNode(const DeviceNode&) = delete;
        DeviceNode& operator=(const DeviceNode&) = delete;

        DeviceNode(DeviceNode&& other) noexcept;
        DeviceNode& operator=(DeviceNode&& other) noexcept;

        /**
         * @brief 打开设备节点
         * @param path 设备路径，如 /dev/i2c-0, /sys/class/xxx/value
         * @param flags 打开标志 (O_RDONLY, O_WRONLY, O_RDWR)
         * @return std::optional<std::error_code> - 成功返回空，失败返回错误码
         */
        [[nodiscard]] std::optional<std::error_code> open(std::string_view path, int flags = O_RDWR) noexcept;

        /**
         * @brief 关闭设备节点
         */
        void close() noexcept;

        /**
         * @brief 是否已打开
         */
        [[nodiscard]] bool isOpen() const noexcept;

        /**
         * @brief 获取文件描述符
         */
        [[nodiscard]] int fd() const noexcept;

        /**
         * @brief 读取数据
         * @param buf 缓冲区
         * @param size 大小
         * @param bytesRead 输出参数：实际读取的字节数
         * @return 成功返回空，失败返回错误码
         */
        [[nodiscard]] std::optional<std::error_code> read(void* buf, size_t size, ssize_t& bytesRead) noexcept;

        /**
         * @brief 写入数据
         * @param data 数据
         * @param size 大小
         * @param bytesWritten 输出参数：实际写入的字节数
         * @return 成功返回空，失败返回错误码
         */
        [[nodiscard]] std::optional<std::error_code> write(const void* data, size_t size,
                                                           ssize_t& bytesWritten) noexcept;

        /**
         * @brief 读取字符串（适用于sysfs）
         * @return 成功返回字符串，失败返回std::nullopt
         */
        [[nodiscard]] std::optional<std::string> readString() noexcept;

        /**
         * @brief 写入字符串（适用于sysfs）
         * @param str 字符串
         * @return std::optional<std::error_code> - 成功返回空，失败返回错误码
         */
        [[nodiscard]] std::optional<std::error_code> writeString(std::string_view str) noexcept;

        /**
         * @brief IO控制
         * @param request 命令
         * @param arg 参数
         * @return std::optional<std::error_code> - 成功返回空，失败返回错误码
         */
        [[nodiscard]] std::optional<std::error_code> ioctl(unsigned long request, void* arg) noexcept;

        /**
         * @brief 获取设备路径
         */
        [[nodiscard]] std::string_view path() const noexcept;

    private:
        int m_fd;
        std::string m_path;
    };

} // namespace hal
