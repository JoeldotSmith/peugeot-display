#define CONFIG_LVGL_PORT_ROTATION_DEGREE 0
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lv_conf.h"
#include "lvgl_v8_port.h"
#include "driver/twai.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;


Board *board = nullptr;

bool inited = false;
bool canDriverInstalled = false;
bool displayReady = false;
static lv_obj_t *displayBoostLabel = nullptr;
static lv_obj_t *displayMapLabel = nullptr;
static lv_obj_t *displayRpmLabel = nullptr;
static lv_obj_t *displayCoolantLabel = nullptr;
static lv_obj_t *displayIndicatorLabel = nullptr;
static lv_obj_t *displaySpoilerLabel = nullptr;
static lv_obj_t *displayCanLabel = nullptr;

struct Data {
  float coolantTemp;
  float rpm;
  float manifoldPressureKpa;
  float boostPressureKpa;
  uint8_t indicatorRaw;
  uint8_t spoilerRaw;
  int spoilerPercent;
  uint32_t frames;
  uint32_t mappedFrames;
  uint32_t mapRequests;
  uint32_t mapResponses;
  uint32_t lastMappedFrameMs;
  uint32_t lastMapResponseMs;
  bool indicatorsOn;
  bool hasManifoldPressure;
};
Data latest;
Data current;

static const uint32_t CAN_FRAMES_PER_LOOP = 32;
static const uint32_t MAP_REQUEST_INTERVAL_MS = 250;
static const float ATMOSPHERIC_PRESSURE_KPA = 101.0f;

static int spoiler_percent_from_raw(uint8_t raw) {
  if (raw <= 0x10) return 0;
  if (raw >= 0x50) return 100;
  if (raw <= 0x20) return 30;
  if (raw <= 0x40) return 80;
  return 100;
}

static void decode_can_message(const twai_message_t &message) {
  if (message.extd || message.rtr) return;

  latest.frames++;
  switch (message.identifier) {
    case 0x208:
      if (message.data_length_code >= 2) {
        uint16_t raw = ((uint16_t)message.data[0] << 8) | message.data[1];
        latest.rpm = raw * 0.125f;
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
      }
      break;
    case 0x488:
      if (message.data_length_code >= 1) {
        latest.coolantTemp = (message.data[0] * 0.5f) - 40.0f;
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
      }
      break;
    case 0x50D:
      if (message.data_length_code >= 6) {
        latest.indicatorRaw = message.data[5];
        latest.indicatorsOn = message.data[5] == 0x65;
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
      }
      break;
    case 0x612:
      if (message.data_length_code >= 5) {
        latest.spoilerRaw = message.data[4];
        latest.spoilerPercent = spoiler_percent_from_raw(message.data[4]);
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
      }
      break;
    case 0x7E8:
      if (message.data_length_code >= 4 &&
          message.data[0] == 0x03 &&
          message.data[1] == 0x41 &&
          message.data[2] == 0x0B) {
        latest.manifoldPressureKpa = (float)message.data[3];
        latest.boostPressureKpa = latest.manifoldPressureKpa - ATMOSPHERIC_PRESSURE_KPA;
        latest.hasManifoldPressure = true;
        latest.mapResponses++;
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
        latest.lastMapResponseMs = latest.lastMappedFrameMs;
      }
      break;
  }
}

static bool init_can() {
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_15, GPIO_NUM_16, TWAI_MODE_NORMAL);
  g_config.tx_queue_len = 8;
  g_config.rx_queue_len = 64;
  g_config.alerts_enabled = TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_RX_FIFO_OVERRUN |
                            TWAI_ALERT_BUS_ERROR | TWAI_ALERT_ERR_PASS |
                            TWAI_ALERT_BUS_OFF;

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("Failed to install TWAI driver");
    return false;
  }
  if (twai_start() != ESP_OK) {
    Serial.println("Failed to start TWAI driver");
    return false;
  }
  Serial.println("TWAI driver started");
  return true;
}

static void request_manifold_pressure() {
  if (!canDriverInstalled) return;

  static uint32_t lastMapRequestMs = 0;
  uint32_t now = millis();
  if (now - lastMapRequestMs < MAP_REQUEST_INTERVAL_MS) return;
  lastMapRequestMs = now;

  twai_message_t request = {};
  request.identifier = 0x7DF;
  request.data_length_code = 8;
  request.data[0] = 0x02;
  request.data[1] = 0x01;
  request.data[2] = 0x0B;

  if (twai_transmit(&request, 0) == ESP_OK) {
    latest.mapRequests++;
  }
}

static void poll_can() {
  if (!canDriverInstalled) return;

  twai_message_t message;
  uint32_t framesThisLoop = 0;
  while (framesThisLoop < CAN_FRAMES_PER_LOOP && twai_receive(&message, 0) == ESP_OK) {
    decode_can_message(message);
    framesThisLoop++;
  }
}

static void log_can_status() {
  if (!canDriverInstalled) return;

  static uint32_t lastLogMs = 0;
  uint32_t now = millis();
  if (now - lastLogMs < 1000) return;
  lastLogMs = now;

  twai_status_info_t status;
  if (twai_get_status_info(&status) != ESP_OK) return;

  uint32_t alerts = 0;
  twai_read_alerts(&alerts, 0);

  Serial.printf(
    "CAN state=%d rxq=%lu missed=%lu overrun=%lu buserr=%lu rxerr=%lu alerts=0x%08lX frames=%lu mapped=%lu\n",
    (int)status.state,
    (unsigned long)status.msgs_to_rx,
    (unsigned long)status.rx_missed_count,
    (unsigned long)status.rx_overrun_count,
    (unsigned long)status.bus_error_count,
    (unsigned long)status.rx_error_counter,
    (unsigned long)alerts,
    (unsigned long)latest.frames,
    (unsigned long)latest.mappedFrames
  );
}

#if DIAG_CONSOLE_ONLY

void setup()
{
    Serial.begin(1000000);
    uint32_t startMs = millis();
    while (!Serial && millis() - startMs < 3000) {
      delay(10);
    }

    Serial.println();
    Serial.println("peugeot-display console diagnostic boot");
    Serial.println("LVGL/display init is disabled in this build");
    Serial.printf("millis=%lu\n", (unsigned long)millis());

    canDriverInstalled = init_can();
    Serial.printf("TWAI init: %s\n", canDriverInstalled ? "ok" : "failed");
}

void loop()
{
  request_manifold_pressure();
  poll_can();

  static uint32_t lastDiagMs = 0;
  uint32_t now = millis();
  if (now - lastDiagMs >= 1000) {
    lastDiagMs = now;

    twai_status_info_t status = {};
    esp_err_t statusResult = canDriverInstalled ? twai_get_status_info(&status) : ESP_FAIL;

    Serial.printf(
      "diag ms=%lu can=%s state=%d txq=%lu rxq=%lu txerr=%lu rxerr=%lu frames=%lu mapped=%lu map=%lu/%lu boost=%.0f abs=%.0f\n",
      (unsigned long)now,
      canDriverInstalled ? "ok" : "off",
      statusResult == ESP_OK ? (int)status.state : -1,
      statusResult == ESP_OK ? (unsigned long)status.msgs_to_tx : 0UL,
      statusResult == ESP_OK ? (unsigned long)status.msgs_to_rx : 0UL,
      statusResult == ESP_OK ? (unsigned long)status.tx_error_counter : 0UL,
      statusResult == ESP_OK ? (unsigned long)status.rx_error_counter : 0UL,
      (unsigned long)latest.frames,
      (unsigned long)latest.mappedFrames,
      (unsigned long)latest.mapResponses,
      (unsigned long)latest.mapRequests,
      latest.boostPressureKpa,
      latest.manifoldPressureKpa
    );
  }

  delay(5);
}

#else

static void log_displayless_status() {
  static uint32_t lastDiagMs = 0;
  uint32_t now = millis();
  if (now - lastDiagMs < 1000) return;
  lastDiagMs = now;

  Serial.printf(
    "display-off ms=%lu frames=%lu mapped=%lu map=%lu/%lu boost=%.0f abs=%.0f\n",
    (unsigned long)now,
    (unsigned long)latest.frames,
    (unsigned long)latest.mappedFrames,
    (unsigned long)latest.mapResponses,
    (unsigned long)latest.mapRequests,
    latest.boostPressureKpa,
    latest.manifoldPressureKpa
  );
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, int x, int y, int w, int h, const lv_font_t *font, lv_color_t color) {
  lv_obj_t *label = lv_label_create(parent);
  lv_obj_set_pos(label, x, y);
  lv_obj_set_size(label, w, h);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color, 0);
  lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(label, 0, 0);
  return label;
}

static lv_obj_t *make_row(lv_obj_t *parent, const char *name, const char *value, int y) {
  make_label(parent, name, 500, y, 170, 32, LV_FONT_DEFAULT, lv_color_hex(0x888888));
  return make_label(parent, value, 680, y, 330, 32, LV_FONT_DEFAULT, lv_color_hex(0xFFFFFF));
}

static void create_display_ui() {
  lv_obj_t *screen = lv_obj_create(nullptr);
  lv_obj_remove_style_all(screen);
  lv_obj_set_size(screen, 1024, 340);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  make_label(screen, "BOOST", 24, 16, 240, 42, LV_FONT_DEFAULT, lv_color_hex(0xFAAE00));
  displayBoostLabel = make_label(screen, "--", 24, 70, 280, 72, LV_FONT_DEFAULT, lv_color_hex(0xFFFFFF));
  make_label(screen, "kPa gauge", 300, 104, 180, 34, LV_FONT_DEFAULT, lv_color_hex(0x888888));

  displayRpmLabel = make_row(screen, "RPM", "---- rpm", 24);
  displayMapLabel = make_row(screen, "MAP", "waiting", 68);
  displayCoolantLabel = make_row(screen, "Coolant", "---.- C", 112);
  displayIndicatorLabel = make_row(screen, "Indicators", "--", 156);
  displaySpoilerLabel = make_row(screen, "Spoiler", "---", 200);
  displayCanLabel = make_row(screen, "CAN", "starting", 244);

  lv_screen_load(screen);
}

void handle_rpm() {
  if (current.rpm != latest.rpm) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f rpm", latest.rpm);
    lv_label_set_text(displayRpmLabel, buf);
  }
}

void handle_boost() {
  if (current.boostPressureKpa != latest.boostPressureKpa ||
      current.manifoldPressureKpa != latest.manifoldPressureKpa ||
      current.hasManifoldPressure != latest.hasManifoldPressure) {
    char buf[48];
    if (latest.hasManifoldPressure) {
      snprintf(buf, sizeof(buf), "%.0f", latest.boostPressureKpa);
      lv_label_set_text(displayBoostLabel, buf);
      snprintf(buf, sizeof(buf), "%.0f kPa abs", latest.manifoldPressureKpa);
      lv_label_set_text(displayMapLabel, buf);
    } else {
      lv_label_set_text(displayBoostLabel, "--");
      lv_label_set_text(displayMapLabel, "waiting");
    }
  }
}

void handle_coolant(){ 
  if (current.coolantTemp != latest.coolantTemp) {
    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%.1f C", latest.coolantTemp);
    lv_label_set_text(displayCoolantLabel, buf1);
  }
} 

void handle_indicators() {
  if (current.indicatorRaw != latest.indicatorRaw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%s 0x%02X", latest.indicatorsOn ? "ON" : "OFF", latest.indicatorRaw);
    lv_label_set_text(displayIndicatorLabel, buf);
  }
}

void handle_spoiler() {
  if (current.spoilerRaw != latest.spoilerRaw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%d pct 0x%02X", latest.spoilerPercent, latest.spoilerRaw);
    lv_label_set_text(displaySpoilerLabel, buf);
  }
}

void handle_can_status() {
  static uint32_t lastStatusRefreshMs = 0;
  uint32_t now = millis();
  if (current.frames != latest.frames || current.mappedFrames != latest.mappedFrames || now - lastStatusRefreshMs > 500) {
    lastStatusRefreshMs = now;
    char buf[48];
    bool stale = latest.lastMappedFrameMs == 0 || now - latest.lastMappedFrameMs > 2000;
    snprintf(buf, sizeof(buf), "%s %lu/%lu MAP %lu/%lu", stale ? "waiting" : "live",
             (unsigned long)latest.mappedFrames, (unsigned long)latest.frames,
             (unsigned long)latest.mapResponses, (unsigned long)latest.mapRequests);
    lv_label_set_text(displayCanLabel, buf);
  }
}

void setup()
{
    Serial.begin(1000000);

    Serial.println("Initializing board");

    board = new Board();
    board->init();
    
    #if LVGL_PORT_AVOID_TEARING_MODE
      auto lcd = board->getLCD();
      lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
      #if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
          auto lcd_bus = lcd->getBus();
          if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
              static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
          }
      #endif
    #endif
    
    if (!board->begin()) {
      Serial.println("Board begin failed");
      while (true) delay(1000);
    }
    
    Serial.println("Initializing LVGL");
    displayReady = lvgl_port_init(board->getLCD(), board->getTouch());
    if (!displayReady) {
      Serial.println("LVGL init failed; continuing with serial-only output");
      canDriverInstalled = init_can();
      inited = true;
      return;
    }
    
    lvgl_port_lock(-1);

    create_display_ui();
    lv_timer_handler();
    
    lvgl_port_unlock();

    canDriverInstalled = init_can();
    lvgl_port_lock(-1);
    lv_label_set_text(displayCanLabel, canDriverInstalled ? "waiting for CAN" : "TWAI failed");
    lvgl_port_unlock();
    
    Serial.println("Setup complete");
    inited = true;
}

void loop() {
  if (!inited) return;
  request_manifold_pressure();
  poll_can();
  log_can_status();
  if (!displayReady) {
    log_displayless_status();
    delay(5);
    return;
  }
  lvgl_port_lock(-1);
  handle_rpm();
  handle_boost();
  handle_coolant();
  handle_indicators();
  handle_spoiler();
  handle_can_status();
  memcpy(&current, &latest, sizeof(Data));
  lvgl_port_unlock();
  delay(5);
}

#endif
