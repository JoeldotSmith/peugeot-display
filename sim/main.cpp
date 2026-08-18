#include "lvgl.h"
#include "lv_conf.h"
#include "ui.h"
#include <SDL2/SDL.h>

#define SIM_WIDTH  1024
#define SIM_HEIGHT 600

static float sim_coolant = 0.0f;
static float sim_rpm = 0.0f;
static float sim_manifold_kpa = 101.0f;
static uint8_t sim_indicator_raw = 0x66;
static uint8_t sim_spoiler_raw = 0x10;
static uint32_t sim_frames = 0;
static uint32_t sim_mapped_frames = 0;

static void handle_rpm(float rpm) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f rpm", rpm);
    lv_label_set_text(rpmLabel, buf);
}

static void handle_boost(float manifold_kpa) {
    char buf[24];
    float boost_kpa = manifold_kpa - 101.0f;
    snprintf(buf, sizeof(buf), "%.0f", boost_kpa);
    lv_label_set_text(boostLabel, buf);
    snprintf(buf, sizeof(buf), "%.0f kPa abs", manifold_kpa);
    lv_label_set_text(manifoldPressureLabel, buf);
    lv_bar_set_value(boostBar, (int)boost_kpa, LV_ANIM_OFF);
}

static void handle_coolant(float coolant) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f C", coolant);
    lv_label_set_text(ui_Label2, buf);
}

static int spoiler_percent_from_raw(uint8_t raw) {
    if (raw <= 0x10) return 0;
    if (raw >= 0x50) return 100;
    if (raw <= 0x20) return 30;
    if (raw <= 0x40) return 80;
    return 100;
}

static void handle_indicators(uint8_t raw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%s 0x%02X", raw == 0x65 ? "ON" : "OFF", raw);
    lv_label_set_text(indicatorLabel, buf);
}

static void handle_spoiler(uint8_t raw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%d%% 0x%02X", spoiler_percent_from_raw(raw), raw);
    lv_label_set_text(spoilerLabel, buf);
}

static void handle_can_status(void) {
    char buf[32];
    snprintf(buf, sizeof(buf), "sim %u/%u", (unsigned)sim_mapped_frames, (unsigned)sim_frames);
    lv_label_set_text(canStatusLabel, buf);
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
            sim_rpm += 72.0f;
            sim_coolant += 0.1f;
            sim_manifold_kpa = 101.0f + ((sim_rpm / 8000.0f) * 180.0f) - 40.0f;
            sim_frames += 4;
            sim_mapped_frames += 4;

            if (sim_rpm > 8000.0f) sim_rpm = 0.0f;
            if (sim_coolant > 150.0f) sim_coolant = 0.0f;
            if ((sim_frames / 120) % 2 == 0) {
                sim_indicator_raw = 0x65;
            } else {
                sim_indicator_raw = 0x66;
            }
            switch ((sim_frames / 240) % 4) {
                case 0: sim_spoiler_raw = 0x10; break;
                case 1: sim_spoiler_raw = 0x20; break;
                case 2: sim_spoiler_raw = 0x40; break;
                default: sim_spoiler_raw = 0x50; break;
            }

            handle_rpm(sim_rpm);
            handle_boost(sim_manifold_kpa);
            handle_coolant(sim_coolant);
            handle_indicators(sim_indicator_raw);
            handle_spoiler(sim_spoiler_raw);
            handle_can_status();
        }

        SDL_Delay(5);
    }

    return 0;
}
