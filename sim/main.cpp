#include "lvgl.h"
#include "lv_conf.h"
#include "ui.h"
#include <SDL2/SDL.h>

#define SIM_WIDTH  1024
#define SIM_HEIGHT 600

static float sim_voltage = 0.0f;
static float sim_boost = 0.0f;

static void handle_voltage(float voltage) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f V", voltage);
    lv_label_set_text(ui_Label1, buf);
}

static void handle_boost(float boost) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", boost);
    lv_label_set_text(boostLabel, buf);
    lv_bar_set_value(boostBar, (int)(boost * 10), LV_ANIM_OFF);
}

int main(void) {
    lv_init();

    // LVGL's built-in SDL driver handles window, flush, and input
    lv_display_t *disp = lv_sdl_window_create(SIM_WIDTH, SIM_HEIGHT);
    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_display(mouse, disp);

    ui_init();

    uint32_t last_update = SDL_GetTicks();

    while(1) {
        lv_timer_handler();

        uint32_t now = SDL_GetTicks();
        if (now - last_update >= 16) {
            last_update = now;
            sim_voltage += 0.1f;
            sim_boost += 0.1f;

            if (sim_voltage > 10.0f) sim_voltage = 0.0f;
            if (sim_boost > 15.0f) sim_boost = 0.0f;
            handle_voltage(sim_voltage);
            handle_boost(sim_boost);
        }

        SDL_Delay(5);
    }

    return 0;
}
