#include <csignal>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include <lvgl/lv_conf.h>
#include <lvgl/lvgl.h>
#include <lvgl/src/extra/libs/fsdrv/lv_fsdrv.h>
#include "lv_port_init.h"

#include "ui.h"

static int quit = 0;

static void sigterm_handler(int sig)
{
    std::cerr << "signal " << sig << '\n';
    quit = 1;
}

// 绑定线程到指定 CPU 核心
int bind_thread_to_cpu(pthread_t thread, int cpu_id)
{
    const long online_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (online_cpus <= 0)
    {
        fprintf(stderr, "[CPU] sysconf(_SC_NPROCESSORS_ONLN) failed\n");
        return -1;
    }

    if (cpu_id < 0 || cpu_id >= online_cpus)
    {
        fprintf(stderr, "[CPU] Invalid cpu_id=%d, online_cpus=%ld\n", cpu_id, online_cpus);
        return -1;
    }

    cpu_set_t allowed_set;
    CPU_ZERO(&allowed_set);
    int ret = pthread_getaffinity_np(thread, sizeof(allowed_set), &allowed_set);
    if (ret != 0)
    {
        fprintf(stderr, "[CPU] pthread_getaffinity_np failed: %s\n", strerror(ret));
        return -1;
    }

    if (!CPU_ISSET(cpu_id, &allowed_set))
    {
        fprintf(stderr, "[CPU] CPU %d is not in allowed affinity mask\n", cpu_id);
        return -1;
    }

    cpu_set_t target_set;
    CPU_ZERO(&target_set);
    CPU_SET(cpu_id, &target_set);

    ret = pthread_setaffinity_np(thread, sizeof(target_set), &target_set);
    if (ret != 0)
    {
        fprintf(stderr, "[CPU] pthread_setaffinity_np failed: %s\n", strerror(ret));
        return -1;
    }

    // 验证绑定结果
    CPU_ZERO(&target_set);
    ret = pthread_getaffinity_np(thread, sizeof(target_set), &target_set);
    if (ret != 0)
    {
        fprintf(stderr, "[CPU] verify pthread_getaffinity_np failed: %s\n", strerror(ret));
        return -1;
    }

    const int max_cpu_to_print = (online_cpus < CPU_SETSIZE) ? static_cast<int>(online_cpus) : CPU_SETSIZE;
    printf("[CPU] Thread bound to CPU %d (actual: ", cpu_id);
    for (int i = 0; i < max_cpu_to_print; i++)
    {
        if (CPU_ISSET(i, &target_set))
        {
            printf("%d ", i);
        }
    }
    printf(")\n");

    return 0;
}

// 设置线程调度策略和优先级
int set_thread_priority(pthread_t thread, int policy, int priority)
{
    const int min_priority = sched_get_priority_min(policy);
    const int max_priority = sched_get_priority_max(policy);
    if (min_priority == -1 || max_priority == -1)
    {
        fprintf(stderr, "[CPU] Invalid scheduling policy=%d\n", policy);
        return -1;
    }

    if (priority < min_priority || priority > max_priority)
    {
        fprintf(stderr, "[CPU] Invalid priority=%d for policy=%d (range: %d..%d)\n", priority, policy, min_priority,
                max_priority);
        return -1;
    }

    struct sched_param param{};
    param.sched_priority = priority;

    int ret = pthread_setschedparam(thread, policy, &param);
    if (ret != 0)
    {
        fprintf(stderr, "[CPU] pthread_setschedparam failed: %s\n", strerror(ret));
        if (ret == EPERM)
        {
            fprintf(stderr, "[CPU] Hint: RT policy needs CAP_SYS_NICE or root privilege\n");
        }
        return -1;
    }

    const char* policy_str = "SCHED_OTHER";
    if (policy == SCHED_FIFO)
    {
        policy_str = "SCHED_FIFO";
    }
    else if (policy == SCHED_RR)
    {
        policy_str = "SCHED_RR";
    }
    printf("[CPU] Thread priority set: %s, prio=%d\n", policy_str, priority);
    return 0;
}

int main()
{

    signal(2, sigterm_handler);

    // ========== CPU 亲和性设置 ==========
    // 主线程（UI）多核机器优先绑定到 CPU1，避免与常见中断热点 CPU0 竞争。
    const long online_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    const int ui_cpu = (online_cpus > 1) ? 1 : 0;
    if (bind_thread_to_cpu(pthread_self(), ui_cpu) != 0)
    {
        fprintf(stderr, "[CPU] Continue without strict affinity binding\n");
    }

    lv_port_init();
    lv_fs_posix_init();

    ui_init(); // 初始化UI

    while (quit == 0)
    {
        ui_tick();         // UI刷新
        lv_task_handler(); // LVGL v8 用这个
        usleep(1000);      // 1ms，EEZ Studio 推荐的延时
    }

    return 0;
}