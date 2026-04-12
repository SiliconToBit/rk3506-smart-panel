#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>
#include "CpuAffinity.h"

namespace hal
{

    class ThreadPool
    {
    public:
        /**
         * @brief 构造函数
         * @param threadCount 线程数量，0表示使用CPU核心数
         * @param cpuAffinity CPU亲和性，-1表示不设置
         * @param threadName 线程名称前缀
         */
        explicit ThreadPool(size_t threadCount = 0, int cpuAffinity = -1, const std::string& threadName = "PoolThread");

        ~ThreadPool();

        // 禁止拷贝和移动
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        /**
         * @brief 提交任务到线程池
         * @param f 可调用对象
         * @param args 参数
         * @return std::future<ReturnType> 用于获取任务结果
         */
        template <class F, class... Args>
        auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

        /**
         * @brief 获取线程池大小
         */
        [[nodiscard]] size_t size() const
        {
            return m_workers.size();
        }

        /**
         * @brief 获取待执行任务数量
         */
        [[nodiscard]] size_t pendingTasks() const;

        /**
         * @brief 等待所有任务完成
         */
        void waitAll();

    private:
        // 工作线程需要知道何时停止
        std::vector<std::thread> m_workers;

        // 任务队列
        std::queue<std::function<void()>> m_tasks;

        // 同步原语
        mutable std::mutex m_queueMutex;
        std::condition_variable m_condition;
        std::condition_variable m_completionCondition;

        // 状态标志
        std::atomic<bool> m_stop{false};
        std::atomic<size_t> m_activeTasks{0};

        // 配置
        int m_cpuAffinity;
        std::string m_threadName;
    };

    // ==========================================
    // 模板实现（必须在头文件中）
    // ==========================================

    template <class F, class... Args>
    auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using ReturnType = typename std::invoke_result<F, Args...>::type;

        // 包装任务，添加计数和通知
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            if (m_stop)
            {
                throw std::runtime_error("submit on stopped ThreadPool");
            }

            m_tasks.emplace([task]() { (*task)(); });
        }

        m_condition.notify_one();
        return result;
    }

} // namespace hal
