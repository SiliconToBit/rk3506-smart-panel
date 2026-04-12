#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <map>
#include <atomic>
#include "ThreadPool.h"

namespace hal
{

    /**
     * @brief HTTP 客户端 - 基于 libcurl 封装
     *
     * 职责：
     * - HTTP GET/POST 请求
     * - 同步/异步请求支持
     * - 请求头管理
     *
     * 使用示例：
     * @code
     * // 同步请求
     * auto& client = hal::HttpClient::getInstance();
     * auto response = client.get("https://api.example.com/data");
     * if (response) {
     *     printf("Response: %s\n", response->c_str());
     * }
     *
     * // 异步请求
     * client.getAsync("https://api.example.com/data", [](int code, const std::string& body) {
     *     if (code == 200) {
     *         printf("Success: %s\n", body.c_str());
     *     }
     * });
     * @endcode
     */
    class HttpClient
    {
    public:
        // ========== 回调类型 ==========

        /**
         * @brief 响应回调类型
         * @param code HTTP 状态码，-1 表示网络错误
         * @param body 响应体
         */
        using ResponseCallback = std::function<void(int code, const std::string& body)>;

        // ========== 单例访问 ==========

        /**
         * @brief 获取单例实例
         * @return HttpClient 单例引用
         */
        static HttpClient& getInstance();

        // ========== 同步请求 ==========

        /**
         * @brief 发送 GET 请求（同步）
         * @param url 请求 URL
         * @return 响应体，失败返回 nullopt
         */
        [[nodiscard]] std::optional<std::string> get(const std::string& url);

        /**
         * @brief 发送 POST 请求（同步）
         * @param url 请求 URL
         * @param body 请求体
         * @param contentType 内容类型，默认 "application/json"
         * @return 响应体，失败返回 nullopt
         */
        [[nodiscard]] std::optional<std::string> post(const std::string& url, const std::string& body,
                                                      const std::string& contentType = "application/json");

        // ========== 异步请求 ==========

        /**
         * @brief 发送 GET 请求（异步）
         * @param url 请求 URL
         * @param callback 响应回调
         */
        void getAsync(const std::string& url, ResponseCallback callback);

        /**
         * @brief 发送 POST 请求（异步）
         * @param url 请求 URL
         * @param body 请求体
         * @param callback 响应回调
         * @param contentType 内容类型
         */
        void postAsync(const std::string& url, const std::string& body, ResponseCallback callback,
                       const std::string& contentType = "application/json");

        // ========== 配置 ==========

        /**
         * @brief 设置请求超时时间
         * @param seconds 超时秒数
         */
        void setTimeout(int seconds);

        /**
         * @brief 获取当前超时时间
         * @return 超时秒数
         */
        [[nodiscard]] int getTimeout() const;

        /**
         * @brief 设置请求头
         * @param key 头字段名
         * @param value 头字段值
         */
        void setHeader(const std::string& key, const std::string& value);

        /**
         * @brief 清除所有自定义请求头
         */
        void clearHeaders();

        /**
         * @brief 设置错误回调
         * @param callback 错误回调函数
         */
        void setOnError(std::function<void(const std::string&)> callback);

    private:
        HttpClient();
        ~HttpClient();

        // 禁止拷贝
        HttpClient(const HttpClient&) = delete;
        HttpClient& operator=(const HttpClient&) = delete;

        // 内部方法
        std::optional<std::string> performRequest(const std::string& url, const std::string& method,
                                                  const std::string& body = "", const std::string& contentType = "");

        void notifyError(const std::string& msg);

        // 成员变量
        int m_timeout = 10;
        std::map<std::string, std::string> m_headers;
        std::mutex m_headerMutex;
        std::function<void(const std::string&)> m_onError;

        // 线程池管理异步请求
        std::unique_ptr<ThreadPool> m_threadPool;
        std::atomic<bool> m_shutdown{false};
    };

} // namespace hal
