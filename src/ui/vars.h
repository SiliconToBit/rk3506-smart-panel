#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

typedef enum {
    SettingsTab_TAB_BACKLIGHT = 0,
    SettingsTab_TAB_WIFI = 1,
    SettingsTab_TAB_SOUND = 2
} SettingsTab;

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_SETTINGS_TAB_ID = 0
};

// Native global variables

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/