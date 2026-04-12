#pragma once

#include <thread>
#include <iostream>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

namespace hal
{

    /**
     * @brief 获取在线CPU数量
     * @return 在线CPU数量，失败返回1
     */
    inline int getOnlineCpus()
    {
        const long online = sysconf(_SC_NPROCESSORS_ONLN);
        return online > 0 ? static_cast<int>(online) : 1;
    }

    /**
     * @brief 将std::thread绑定到指定CPU核心
     * @param thread std::thread对象
     * @param cpuId CPU核心ID
     * @param threadName 线程名称（用于日志）
     * @return 0成功，-1失败
     */
    inline int bindThreadToCpu(std::thread& thread, int cpuId, const std::string& threadName)
    {
        if (!thread.joinable())
        {
            return -1;
        }

        const int onlineCpus = getOnlineCpus();
        if (cpuId < 0 || cpuId >= onlineCpus)
        {
            std::cerr << "[CPU] Invalid cpu_id=" << cpuId << " for " << threadName << " (online_cpus=" << onlineCpus
                      << ")\n";
            return -1;
        }

        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpuId, &set);

        const int ret = pthread_setaffinity_np(thread.native_handle(), sizeof(set), &set);
        if (ret != 0)
        {
            std::cerr << "[CPU] Failed to bind " << threadName << " to CPU " << cpuId << ": " << ret << '\n';
            return -1;
        }

        return 0;
    }

    /**
     * @brief 将pthread_t绑定到指定CPU核心（用于main线程等原生线程）
     * @param thread pthread_t线程句柄
     * @param cpuId CPU核心ID
     * @param threadName 线程名称（用于日志）
     * @return 0成功，-1失败
     */
    inline int bindThreadToCpu(pthread_t thread, int cpuId, const std::string& threadName)
    {
        const int onlineCpus = getOnlineCpus();
        if (onlineCpus <= 0)
        {
            std::cerr << "[CPU] sysconf failed for " << threadName << '\n';
            return -1;
        }

        if (cpuId < 0 || cpuId >= onlineCpus)
        {
            std::cerr << "[CPU] Invalid cpu_id=" << cpuId << " for " << threadName << " (online_cpus=" << onlineCpus
                      << ")\n";
            return -1;
        }

        // 检查CPU是否在允许的掩码中
        cpu_set_t allowedSet;
        CPU_ZERO(&allowedSet);
        int ret = pthread_getaffinity_np(thread, sizeof(allowedSet), &allowedSet);
        if (ret != 0)
        {
            std::cerr << "[CPU] pthread_getaffinity_np failed for " << threadName << ": " << ret << '\n';
            return -1;
        }

        if (!CPU_ISSET(cpuId, &allowedSet))
        {
            std::cerr << "[CPU] CPU " << cpuId << " is not in allowed affinity mask for " << threadName << '\n';
            return -1;
        }

        cpu_set_t targetSet;
        CPU_ZERO(&targetSet);
        CPU_SET(cpuId, &targetSet);

        ret = pthread_setaffinity_np(thread, sizeof(targetSet), &targetSet);
        if (ret != 0)
        {
            std::cerr << "[CPU] pthread_setaffinity_np failed for " << threadName << ": " << ret << '\n';
            return -1;
        }

        return 0;
    }

    /**
     * @brief 将当前线程绑定到指定CPU核心（用于std::async创建的线程内部调用）
     * @param cpuId CPU核心ID
     * @param threadName 线程名称（用于日志）
     * @return 0成功，-1失败
     */
    inline int bindCurrentThreadToCpu(int cpuId, const std::string& threadName)
    {
        const int onlineCpus = getOnlineCpus();
        if (cpuId < 0 || cpuId >= onlineCpus)
        {
            std::cerr << "[CPU] Invalid cpu_id=" << cpuId << " for " << threadName << " (online_cpus=" << onlineCpus
                      << ")\n";
            return -1;
        }

        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpuId, &set);

        const int ret = pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
        if (ret != 0)
        {
            std::cerr << "[CPU] Failed to bind " << threadName << " to CPU " << cpuId << ": " << ret << '\n';
            return -1;
        }

        return 0;
    }

    /**
     * @brief 设置线程调度策略和优先级
     * @param thread pthread_t线程句柄
     * @param policy 调度策略（SCHED_FIFO, SCHED_RR, SCHED_OTHER）
     * @param priority 优先级
     * @param threadName 线程名称（用于日志）
     * @return 0成功，-1失败
     */
    inline int setThreadPriority(pthread_t thread, int policy, int priority, const std::string& threadName)
    {
        const int minPriority = sched_get_priority_min(policy);
        const int maxPriority = sched_get_priority_max(policy);
        if (minPriority == -1 || maxPriority == -1)
        {
            std::cerr << "[CPU] Invalid scheduling policy=" << policy << " for " << threadName << '\n';
            return -1;
        }

        if (priority < minPriority || priority > maxPriority)
        {
            std::cerr << "[CPU] Invalid priority=" << priority << " for " << threadName << " (range: " << minPriority
                      << ".." << maxPriority << ")\n";
            return -1;
        }

        struct sched_param param{};
        param.sched_priority = priority;

        const int ret = pthread_setschedparam(thread, policy, &param);
        if (ret != 0)
        {
            std::cerr << "[CPU] pthread_setschedparam failed for " << threadName << ": " << ret << '\n';
            if (ret == EPERM)
            {
                std::cerr << "[CPU] Hint: RT policy needs CAP_SYS_NICE or root privilege\n";
            }
            return -1;
        }

        return 0;
    }

    /**
     * @brief 获取音频解码线程推荐的CPU核心
     * @return 推荐的CPU核心ID
     */
    inline int selectAudioCpu()
    {
        const int onlineCpus = getOnlineCpus();
        if (onlineCpus >= 3)
        {
            return 2;
        }
        if (onlineCpus >= 2)
        {
            return 0;
        }
        return 0;
    }

    /**
     * @brief 获取视频解码线程推荐的CPU核心
     * @return 推荐的CPU核心ID
     */
    inline int selectVideoCpu()
    {
        const int onlineCpus = getOnlineCpus();
        if (onlineCpus >= 3)
        {
            return 2;
        }
        if (onlineCpus >= 2)
        {
            return 0;
        }
        return 0;
    }

    /**
     * @brief 获取UI线程推荐的CPU核心
     * @return 推荐的CPU核心ID
     */
    inline int selectUiCpu()
    {
        const int onlineCpus = getOnlineCpus();
        return (onlineCpus > 1) ? 1 : 0;
    }

    /**
     * @brief 获取HTTP异步线程推荐的CPU核心
     * @return 推荐的CPU核心ID
     */
    inline int selectHttpCpu()
    {
        return 0;
    }

} // namespace hal
