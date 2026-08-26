#pragma once

#include "sdkconfig.h"
#ifdef CONFIG_ARDUINO_RUNNING_CORE
#include <Arduino.h>
#endif
#include "esp_display_panel.hpp"
#include "lvgl.h"

#define LVGL_PORT_TICK_PERIOD_MS        (2)
#define LVGL_PORT_TASK_MAX_DELAY_MS     (500)
#define LVGL_PORT_TASK_MIN_DELAY_MS     (2)
#define LVGL_PORT_TASK_STACK_SIZE       (10 * 1024)
#define LVGL_PORT_TASK_PRIORITY         (5)
#define LVGL_PORT_TASK_CORE             (1)

// Keep this so main.cpp #if LVGL_PORT_AVOID_TEARING_MODE still compiles
#define LVGL_PORT_AVOID_TEARING_MODE    (3)
#define LVGL_PORT_DISP_BUFFER_NUM       (2)

#ifdef __cplusplus
extern "C" {
#endif

bool lvgl_port_init(esp_panel::drivers::LCD *lcd, esp_panel::drivers::Touch *tp);
bool lvgl_port_deinit(void);
bool lvgl_port_lock(int timeout_ms);
bool lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif
