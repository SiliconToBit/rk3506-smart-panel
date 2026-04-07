/**
 * @file DeviceNode.cpp
 * @brief HAL层 - 设备节点实现
 */

#include "hal/DeviceNode.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>

namespace hal
{

    DeviceNode::DeviceNode() noexcept
        : m_fd(-1)
    {
    }

    DeviceNode::~DeviceNode() noexcept
    {
        close();
    }

    DeviceNode::DeviceNode(DeviceNode&& other) noexcept
        : m_fd(other.m_fd)
        , m_path(std::move(other.m_path))
    {
        other.m_fd = -1;
        other.m_path.clear();
    }

    DeviceNode& DeviceNode::operator=(DeviceNode&& other) noexcept
    {
        if (this != &other)
        {
            close();
            m_fd = other.m_fd;
            m_path = std::move(other.m_path);
            other.m_fd = -1;
            other.m_path.clear();
        }
        return *this;
    }

    std::optional<std::error_code> DeviceNode::open(std::string_view path, int flags) noexcept
    {
        if (isOpen())
        {
            close();
        }

        m_fd = ::open(path.data(), flags);
        if (m_fd < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        m_path = path;
        return std::nullopt; // 成功
    }

    void DeviceNode::close() noexcept
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
            m_fd = -1;
            m_path.clear();
        }
    }

    bool DeviceNode::isOpen() const noexcept
    {
        return m_fd >= 0;
    }

    int DeviceNode::fd() const noexcept
    {
        return m_fd;
    }

    std::optional<std::error_code> DeviceNode::read(void* buf, size_t size, ssize_t& bytesRead) noexcept
    {
        if (!isOpen())
        {
            bytesRead = -1;
            return std::error_code(EBADF, std::system_category());
        }

        bytesRead = ::read(m_fd, buf, size);
        if (bytesRead < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        return std::nullopt; // 成功
    }

    std::optional<std::error_code> DeviceNode::write(const void* data, size_t size, ssize_t& bytesWritten) noexcept
    {
        if (!isOpen())
        {
            bytesWritten = -1;
            return std::error_code(EBADF, std::system_category());
        }

        bytesWritten = ::write(m_fd, data, size);
        if (bytesWritten < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        return std::nullopt; // 成功
    }

    std::optional<std::string> DeviceNode::readString() noexcept
    {
        if (!isOpen())
            return std::nullopt;

        char buf[256] = {0};
        ssize_t len = 0;
        auto err = read(buf, sizeof(buf) - 1, len);
        if (err)
            return std::nullopt;

        buf[len] = '\0';
        std::string result(buf);

        // 去除尾部换行
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        {
            result.pop_back();
        }
        return result;
    }

    std::optional<std::error_code> DeviceNode::writeString(std::string_view str) noexcept
    {
        if (!isOpen())
        {
            return std::error_code(EBADF, std::system_category());
        }

        ssize_t written = 0;
        auto err = write(str.data(), str.size(), written);
        if (err)
        {
            return err;
        }
        if (written != static_cast<ssize_t>(str.size()))
        {
            return std::error_code(EIO, std::system_category());
        }
        return std::nullopt; // 成功
    }

    std::optional<std::error_code> DeviceNode::ioctl(unsigned long request, void* arg) noexcept
    {
        if (!isOpen())
        {
            return std::error_code(EBADF, std::system_category());
        }

        int ret = ::ioctl(m_fd, request, arg);
        if (ret < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        return std::nullopt; // 成功
    }

    std::string_view DeviceNode::path() const noexcept
    {
        return m_path;
    }

} // namespace hal