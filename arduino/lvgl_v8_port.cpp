#include "esp_timer.h"
#include "esp_lib_utils.h"
#include "lvgl_v8_port.h"

using namespace esp_panel::drivers;

static SemaphoreHandle_t lvgl_mux = nullptr;
static TaskHandle_t lvgl_task_handle = nullptr;
static esp_timer_handle_t lvgl_tick_timer = NULL;

// ============================================================
// Flush callback - v9 API
// ============================================================
static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LCD *lcd = (LCD *)lv_display_get_user_data(disp);

    if (lv_display_flush_is_last(disp)) {
        lcd->switchFrameBufferTo(px_map);

        // Wait for vsync
        ulTaskNotifyValueClear(NULL, ULONG_MAX);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    lv_display_flush_ready(disp);
}

// ============================================================
// Vsync callback
// ============================================================
IRAM_ATTR bool onLcdVsyncCallback(void *user_data)
{
    BaseType_t need_yield = pdFALSE;
    TaskHandle_t task_handle = (TaskHandle_t)user_data;
    xTaskNotifyFromISR(task_handle, ULONG_MAX, eNoAction, &need_yield);
    return (need_yield == pdTRUE);
}

// ============================================================
// Display init - v9 API
// ============================================================
static lv_display_t *display_init(LCD *lcd)
{
    auto lcd_width  = lcd->getFrameWidth();
    auto lcd_height = 340; // Only use top 340 pixels

    // Create v9 display
    lv_display_t *disp = lv_display_create(lcd_width, lcd_height);
    lv_display_set_user_data(disp, (void *)lcd);
    lv_display_set_flush_cb(disp, flush_callback);

    // Use frame buffers from LCD directly (direct/avoid-tearing mode)
    void *buf0 = lcd->getFrameBufferByIndex(0);
    void *buf1 = lcd->getFrameBufferByIndex(1);

    lv_display_set_buffers(
        disp,
        buf0,
        buf1,
        lcd_width * lcd_height * sizeof(lv_color_t),
        LV_DISPLAY_RENDER_MODE_DIRECT
    );

    return disp;
}

// ============================================================
// Touch input - v9 API
// ============================================================
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    Touch *tp = (Touch *)lv_indev_get_user_data(indev);
    TouchPoint point;

    int result = tp->readPoints(&point, 1, 0);
    if (result > 0) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static lv_indev_t *indev_init(Touch *tp)
{
    if (tp == nullptr || tp->getPanelHandle() == nullptr) return nullptr;

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);
    lv_indev_set_user_data(indev, (void *)tp);

    return indev;
}

// ============================================================
// Tick timer
// ============================================================
static void tick_increment(void *arg)
{
    lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS);
}

static bool tick_init(void)
{
    const esp_timer_create_args_t args = {
        .callback = &tick_increment,
        .name = "lvgl_tick"
    };
    if (esp_timer_create(&args, &lvgl_tick_timer) != ESP_OK) return false;
    if (esp_timer_start_periodic(lvgl_tick_timer, LVGL_PORT_TICK_PERIOD_MS * 1000) != ESP_OK) return false;
    return true;
}

static bool tick_deinit(void)
{
    esp_timer_stop(lvgl_tick_timer);
    esp_timer_delete(lvgl_tick_timer);
    return true;
}

// ============================================================
// LVGL task
// ============================================================
static void lvgl_port_task(void *arg)
{
    uint32_t task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
    while (1) {
        if (lvgl_port_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }
        task_delay_ms = (task_delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS) ? LVGL_PORT_TASK_MAX_DELAY_MS :
                        (task_delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS) ? LVGL_PORT_TASK_MIN_DELAY_MS :
                        task_delay_ms;
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

// ============================================================
// Public API
// ============================================================
bool lvgl_port_init(LCD *lcd, Touch *tp)
{
    if (lcd == nullptr) return false;

    lv_init();
    tick_init();

    lv_display_t *disp = display_init(lcd);
    if (disp == nullptr) return false;

    if (tp != nullptr) {
        indev_init(tp);
    }

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    if (lvgl_mux == nullptr) return false;

    BaseType_t core_id = (LVGL_PORT_TASK_CORE < 0) ? tskNO_AFFINITY : LVGL_PORT_TASK_CORE;
    BaseType_t ret = xTaskCreatePinnedToCore(
        lvgl_port_task, "lvgl",
        LVGL_PORT_TASK_STACK_SIZE, NULL,
        LVGL_PORT_TASK_PRIORITY,
        &lvgl_task_handle, core_id
    );
    if (ret != pdPASS) return false;

    lcd->attachRefreshFinishCallback(onLcdVsyncCallback, (void *)lvgl_task_handle);

    return true;
}

bool lvgl_port_lock(int timeout_ms)
{
    if (lvgl_mux == nullptr) return false;
    const TickType_t ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, ticks) == pdTRUE;
}

bool lvgl_port_unlock(void)
{
    if (lvgl_mux == nullptr) return false;
    xSemaphoreGiveRecursive(lvgl_mux);
    return true;
}

bool lvgl_port_deinit(void)
{
    tick_deinit();
    lvgl_port_lock(-1);
    if (lvgl_task_handle != nullptr) {
        vTaskDelete(lvgl_task_handle);
        lvgl_task_handle = nullptr;
    }
    lvgl_port_unlock();
    lv_deinit();
    if (lvgl_mux != nullptr) {
        vSemaphoreDelete(lvgl_mux);
        lvgl_mux = nullptr;
    }
    return true;
}
