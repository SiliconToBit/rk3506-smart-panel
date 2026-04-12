#include "HttpClient.h"
#include "hal/CpuAffinity.h"
#include <curl/curl.h>
#include <chrono>
#include <iostream>
#include <algorithm>

namespace hal
{

    // ==========================================
    // CURL 回调函数
    // ==========================================

    /**
     * @brief CURL 写数据回调
     */
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        size_t totalSize = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    // ==========================================
    // 构造与析构
    // ==========================================

    HttpClient::HttpClient()
    {
        // 全局初始化 libcurl（只需调用一次）
        static bool curlInitialized = false;
        if (!curlInitialized)
        {
            curl_global_init(CURL_GLOBAL_DEFAULT);
            curlInitialized = true;
        }

        // 创建线程池（2个线程，绑定到HTTP推荐CPU）
        m_threadPool = std::make_unique<ThreadPool>(2, selectHttpCpu(), "HttpWorker");
    }

    HttpClient::~HttpClient()
    {
        m_shutdown = true;
        // 线程池析构时会自动等待所有任务完成
        m_threadPool.reset();
    }

    // ==========================================
    // 单例访问
    // ==========================================

    HttpClient& HttpClient::getInstance()
    {
        static HttpClient instance;
        return instance;
    }

    // ==========================================
    // 同步请求
    // ==========================================

    std::optional<std::string> HttpClient::get(const std::string& url)
    {
        return performRequest(url, "GET");
    }

    std::optional<std::string> HttpClient::post(const std::string& url, const std::string& body,
                                                const std::string& contentType)
    {
        return performRequest(url, "POST", body, contentType);
    }

    // ==========================================
    // 异步请求
    // ==========================================

    void HttpClient::getAsync(const std::string& url, ResponseCallback callback)
    {
        m_threadPool->submit(
            [this, url, callback = std::move(callback)]() mutable
            {
                if (m_shutdown.load())
                {
                    return;
                }

                auto response = get(url);

                if (m_shutdown.load())
                {
                    return;
                }

                if (response)
                {
                    callback(200, *response);
                }
                else
                {
                    callback(-1, "");
                }
            });
    }

    void HttpClient::postAsync(const std::string& url, const std::string& body, ResponseCallback callback,
                               const std::string& contentType)
    {
        m_threadPool->submit(
            [this, url, body, contentType, callback = std::move(callback)]() mutable
            {
                if (m_shutdown.load())
                {
                    return;
                }

                auto response = post(url, body, contentType);

                if (m_shutdown.load())
                {
                    return;
                }

                if (response)
                {
                    callback(200, *response);
                }
                else
                {
                    callback(-1, "");
                }
            });
    }

    // ==========================================
    // 配置
    // ==========================================

    void HttpClient::setTimeout(int seconds)
    {
        m_timeout = seconds;
    }

    int HttpClient::getTimeout() const
    {
        return m_timeout;
    }

    void HttpClient::setHeader(const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(m_headerMutex);
        m_headers[key] = value;
    }

    void HttpClient::clearHeaders()
    {
        std::lock_guard<std::mutex> lock(m_headerMutex);
        m_headers.clear();
    }

    void HttpClient::setOnError(std::function<void(const std::string&)> callback)
    {
        m_onError = std::move(callback);
    }

    // ==========================================
    // 内部方法
    // ==========================================

    std::optional<std::string> HttpClient::performRequest(const std::string& url, const std::string& method,
                                                          const std::string& body, const std::string& contentType)
    {
        // 创建 CURL 句柄
        CURL* curl = curl_easy_init();
        if (!curl)
        {
            notifyError("Failed to initialize CURL");
            return std::nullopt;
        }

        std::string response;
        struct curl_slist* headers = nullptr;

        // 设置 URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置写回调
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // 设置超时
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_timeout);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, m_timeout);

        // 设置方法
        if (method == "POST")
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());
        }

        // 设置自定义请求头
        {
            std::lock_guard<std::mutex> lock(m_headerMutex);
            for (const auto& [key, value] : m_headers)
            {
                std::string header = key + ": " + value;
                headers = curl_slist_append(headers, header.c_str());
            }
        }

        // 添加 Content-Type（POST 请求）
        if (!contentType.empty())
        {
            std::string ctHeader = "Content-Type: " + contentType;
            headers = curl_slist_append(headers, ctHeader.c_str());
        }

        // 设置 User-Agent
        headers = curl_slist_append(headers, "User-Agent: RK3506-SmartPanel/1.0");

        if (headers)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        // 跟随重定向
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

        // 自动处理 Gzip/Deflate 压缩响应
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

        // SSL 验证（嵌入式设备可能需要禁用）
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        // 执行请求
        CURLcode res = curl_easy_perform(curl);

        // 获取 HTTP 状态码
        long httpCode = 0;
        if (res == CURLE_OK)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        }

        // 清理
        if (headers)
        {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);

        // 检查结果
        if (res != CURLE_OK)
        {
            notifyError("CURL error: " + std::string(curl_easy_strerror(res)));
            return std::nullopt;
        }

        if (httpCode >= 400)
        {
            notifyError("HTTP error: " + std::to_string(httpCode));
            return std::nullopt;
        }

        return response;
    }

    void HttpClient::notifyError(const std::string& msg)
    {
        if (m_onError)
        {
            m_onError(msg);
        }
        else
        {
            std::cerr << "[HttpClient Error] " << msg << std::endl;
        }
    }

} // namespace hal
