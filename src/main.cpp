#include <csignal>
#include <cstring>
#include <cstdio>
#include <iostream>

#include "hal/CpuAffinity.h"

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

int main()
{

    signal(2, sigterm_handler);

    // ========== CPU 亲和性设置 ==========
    // 主线程（UI）多核机器优先绑定到 CPU1，避免与常见中断热点 CPU0 竞争。
    const int uiCpu = hal::selectUiCpu();
    if (hal::bindThreadToCpu(pthread_self(), uiCpu, "MainThread") != 0)
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