/**
 * @file main.h
 * @brief Main header file for the application
 *
 * This header file provides common definitions and declarations
 * for the RK3506 Smart Panel application.
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

    // 仅保留主程序必需的头文件

#include <lvgl/lvgl.h>

// 宏定义
#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

    /* Add your common definitions here */

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */