#include "ThreadPool.h"
#include <iostream>

namespace hal
{

    ThreadPool::ThreadPool(size_t threadCount, int cpuAffinity, const std::string& threadName)
        : m_cpuAffinity(cpuAffinity)
        , m_threadName(threadName)
    {
        if (threadCount == 0)
        {
            threadCount = getOnlineCpus();
        }

        for (size_t i = 0; i < threadCount; ++i)
        {
            m_workers.emplace_back(
                [this, i]()
                {
                    // 绑定CPU亲和性
                    if (m_cpuAffinity >= 0)
                    {
                        bindCurrentThreadToCpu(m_cpuAffinity, (m_threadName + "_" + std::to_string(i)).c_str());
                    }

                    while (true)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->m_queueMutex);

                            // 等待任务或停止信号
                            this->m_condition.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });

                            // 停止且队列为空，退出
                            if (this->m_stop && this->m_tasks.empty())
                            {
                                return;
                            }

                            // 取出任务
                            task = std::move(this->m_tasks.front());
                            this->m_tasks.pop();
                            ++this->m_activeTasks;
                        }

                        // 执行任务
                        task();

                        // 更新计数并通知
                        --this->m_activeTasks;
                        this->m_completionCondition.notify_all();
                    }
                });
        }
    }

    ThreadPool::~ThreadPool()
    {
        // 设置停止标志
        m_stop = true;

        // 唤醒所有等待的线程
        m_condition.notify_all();

        // 等待所有工作线程退出
        for (std::thread& worker : m_workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    size_t ThreadPool::pendingTasks() const
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        return m_tasks.size();
    }

    void ThreadPool::waitAll()
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_completionCondition.wait(lock, [this] { return m_tasks.empty() && m_activeTasks == 0; });
    }

} // namespace hal
