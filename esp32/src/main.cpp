#define CONFIG_LVGL_PORT_ROTATION_DEGREE 0
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include <math.h>
#include "lv_conf.h"
#include "lvgl_v8_port.h"
#include "driver/twai.h"

LV_FONT_DECLARE(mono_14);
LV_FONT_DECLARE(mono_24);
LV_FONT_DECLARE(mono_48);

using namespace esp_panel::drivers;
using namespace esp_panel::board;


Board *board = nullptr;

bool inited = false;
bool canDriverInstalled = false;
bool displayReady = false;
bool startupComplete = false;
static uint32_t startupStartMs = 0;
static lv_obj_t *displayBoostLabel = nullptr;
static lv_obj_t *displayBoostUnitLabel = nullptr;
static lv_obj_t *displayCoolantLabel = nullptr;
static lv_obj_t *displaySpoilerLabel = nullptr;
static lv_obj_t *displayClockLabel = nullptr;
static lv_obj_t *displayCanRateLabel = nullptr;
static lv_obj_t *displayBoostBar = nullptr;
static lv_obj_t *displayDimOverlay = nullptr;
static lv_obj_t *startupScreen = nullptr;

struct Data {
  float coolantTemp;
  // float rpm;
  float manifoldPressureKpa;
  float boostPressureKpa;
  // uint8_t indicatorRaw;
  uint8_t spoilerRaw;
  int spoilerPercent;
  uint32_t frames;
  uint32_t mappedFrames;
  uint32_t mapRequests;
  uint32_t mapResponses;
  uint32_t lastMappedFrameMs;
  uint32_t lastMapResponseMs;
  uint32_t secondsOfDay;
  // bool indicatorsOn;
  bool lightsOn;
  bool hasManifoldPressure;
  bool hasClock;
  bool hasLights;
};
Data latest;
Data current;

static const uint32_t CAN_FRAMES_PER_LOOP = 32;
static const uint32_t MAP_REQUEST_INTERVAL_MS = 250;
static const float ATMOSPHERIC_PRESSURE_KPA = 101.0f;
static const float KPA_TO_PSI = 0.1450377f;
static const float KPA_TO_INHG = 0.295300f;
static const float BOOST_DISPLAY_STEP = 0.1f;
static const uint32_t BOOST_DISPLAY_INTERVAL_MS = 33;
static const float BOOST_DISPLAY_CATCHUP_STEPS =
    (float)MAP_REQUEST_INTERVAL_MS / (float)BOOST_DISPLAY_INTERVAL_MS;
static const int BOOST_BAR_MAX = 200;
static const lv_opa_t DISPLAY_DIM_OPA = 123;

static int spoiler_percent_from_raw(uint8_t raw) {
  if (raw <= 0x10) return 0;
  if (raw >= 0x50) return 100;
  if (raw <= 0x20) return 30;
  if (raw <= 0x40) return 80;
  return 100;
}

static uint32_t decode_clock_seconds(uint8_t d1, uint8_t d2, uint8_t d3) {
  uint32_t raw = ((uint32_t)d1 << 12) | ((uint32_t)d2 << 4) | ((uint32_t)d3 >> 4);
  return (raw + 86400UL - 43200UL + 510UL) % 86400UL;
}

static void format_clock(char *buf, size_t size, uint32_t secondsOfDay) {
  uint32_t hour24 = secondsOfDay / 3600UL;
  uint32_t minute = (secondsOfDay % 3600UL) / 60UL;
  const char *suffix = hour24 < 12 ? "am" : "pm";
  uint32_t hour12 = hour24 % 12;
  if (hour12 == 0) hour12 = 12;
  snprintf(buf, size, "%02lu:%02lu %s", (unsigned long)hour12, (unsigned long)minute, suffix);
}

static int boost_bar_value(float boostPressureKpa) {
  float value = boostPressureKpa < 0.0f ? -boostPressureKpa : boostPressureKpa;
  if (value < 0.0f) value = 0.0f;
  if (value > BOOST_BAR_MAX) value = BOOST_BAR_MAX;
  return (int)(value + 0.5f);
}

static float boost_display_step_kpa(float fromKpa, float toKpa) {
  bool vacuum = fromKpa < 0.0f || toKpa < 0.0f;
  float displayUnitStepKpa = BOOST_DISPLAY_STEP / (vacuum ? KPA_TO_INHG : KPA_TO_PSI);
  float catchupStepKpa = fabsf(toKpa - fromKpa) / BOOST_DISPLAY_CATCHUP_STEPS;
  return catchupStepKpa > displayUnitStepKpa ? catchupStepKpa : displayUnitStepKpa;
}

static void decode_can_message(const twai_message_t &message) {
  if (message.extd || message.rtr) return;

  latest.frames++;
  switch (message.identifier) {
    // RPM was useful while proving the CAN stream, but it is not needed on the dash.
    // case 0x208:
    //   if (message.data_length_code >= 2) {
    //     uint16_t raw = ((uint16_t)message.data[0] << 8) | message.data[1];
    //     latest.rpm = raw * 0.125f;
    //     latest.mappedFrames++;
    //     latest.lastMappedFrameMs = millis();
    //   }
    //   break;
    case 0x488:
      if (message.data_length_code >= 1) {
        latest.coolantTemp = (float)message.data[0] - 40.0f;
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
      }
      break;
    case 0x552:
      if (message.data_length_code >= 3) {
        latest.secondsOfDay = decode_clock_seconds(message.data[0], message.data[1], message.data[2]);
        latest.hasClock = true;
        latest.mappedFrames++;
        latest.lastMappedFrameMs = millis();
      }
      break;
    case 0x517:
      if (message.data_length_code >= 3) {
        if (message.data[0] == 0xEB && message.data[1] == 0x7A && message.data[2] == 0x80) {
          latest.lightsOn = true;
          latest.hasLights = true;
          latest.mappedFrames++;
          latest.lastMappedFrameMs = millis();
        } else if (message.data[0] == 0x69 && message.data[1] == 0x5A && message.data[2] == 0x80) {
          latest.lightsOn = false;
          latest.hasLights = true;
          latest.mappedFrames++;
          latest.lastMappedFrameMs = millis();
        }
      }
      break;
    // Indicator state was only for signal validation and is intentionally unused now.
    // case 0x50D:
    //   if (message.data_length_code >= 6) {
    //     latest.indicatorRaw = message.data[5];
    //     latest.indicatorsOn = message.data[5] == 0x65;
    //     latest.mappedFrames++;
    //     latest.lastMappedFrameMs = millis();
    //   }
    //   break;
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

static lv_obj_t *make_panel(lv_obj_t *parent, int x, int y, int w, int h) {
  lv_obj_t *panel = lv_obj_create(parent);
  lv_obj_remove_style_all(panel);
  lv_obj_set_pos(panel, x, y);
  lv_obj_set_size(panel, w, h);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x071018), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x116A93), 0);
  lv_obj_set_style_border_width(panel, 1, 0);
  lv_obj_set_style_radius(panel, 2, 0);
  lv_obj_set_style_pad_all(panel, 0, 0);
  return panel;
}

static void make_wire_line(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color) {
  lv_obj_t *line = lv_obj_create(parent);
  lv_obj_remove_style_all(line);
  lv_obj_set_pos(line, x, y);
  lv_obj_set_size(line, w, h);
  lv_obj_set_style_bg_color(line, lv_color_hex(color), 0);
  lv_obj_set_style_bg_opa(line, LV_OPA_80, 0);
}

static lv_obj_t *make_data_value(lv_obj_t *parent, const char *name, const char *value, int x, int y) {
  make_label(parent, name, x, y, 160, 24, &mono_14, lv_color_hex(0x5288A1));
  return make_label(parent, value, x, y + 26, 220, 34, &mono_24, lv_color_hex(0xE8F8FF));
}

static void create_startup_ui() {
  lv_obj_t *screen = lv_obj_create(nullptr);
  startupScreen = screen;
  lv_obj_remove_style_all(screen);
  lv_obj_set_size(screen, 1024, 340);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x01060A), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  make_wire_line(screen, 28, 24, 968, 1, 0x0AA6E8);
  make_wire_line(screen, 28, 316, 968, 1, 0x0AA6E8);
  make_wire_line(screen, 28, 24, 1, 292, 0x0AA6E8);
  make_wire_line(screen, 995, 24, 1, 292, 0x0AA6E8);

  make_wire_line(screen, 50, 44, 170, 2, 0x34CFFF);
  make_wire_line(screen, 50, 44, 2, 42, 0x34CFFF);
  make_wire_line(screen, 804, 44, 170, 2, 0x34CFFF);
  make_wire_line(screen, 972, 44, 2, 42, 0x34CFFF);
  make_wire_line(screen, 50, 294, 170, 2, 0x34CFFF);
  make_wire_line(screen, 50, 254, 2, 42, 0x34CFFF);
  make_wire_line(screen, 804, 294, 170, 2, 0x34CFFF);
  make_wire_line(screen, 972, 254, 2, 42, 0x34CFFF);

  make_wire_line(screen, 318, 82, 388, 1, 0x116A93);
  make_wire_line(screen, 318, 258, 388, 1, 0x116A93);
  make_wire_line(screen, 438, 128, 148, 1, 0x116A93);
  make_wire_line(screen, 438, 214, 148, 1, 0x116A93);

  for (int i = 0; i < 8; i++) {
    uint32_t color = (i == 2 || i == 5) ? 0x34CFFF : 0x116A93;
    make_wire_line(screen, 424 + i * 24, 238, 12, 2, color);
  }

  lv_obj_t *boot = make_label(screen, "BOOT SEQUENCE", 0, 110, 1024, 24, &mono_14, lv_color_hex(0x5288A1));
  lv_obj_set_style_text_align(boot, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *status = make_label(screen, "SYSTEM INITIALIZING", 0, 146, 1024, 34, &mono_24, lv_color_hex(0xE8F8FF));
  lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *substatus = make_label(screen, "CAN INTERFACE", 0, 186, 1024, 22, &mono_14, lv_color_hex(0x34CFFF));
  lv_obj_set_style_text_align(substatus, LV_TEXT_ALIGN_CENTER, 0);

  make_wire_line(screen, 268, 170, 54, 1, 0x116A93);
  make_wire_line(screen, 702, 170, 54, 1, 0x116A93);

  lv_screen_load(screen);
}

static void create_display_ui() {
  lv_obj_t *screen = lv_obj_create(nullptr);
  lv_obj_remove_style_all(screen);
  lv_obj_set_size(screen, 1024, 340);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x02070C), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  make_wire_line(screen, 18, 18, 988, 1, 0x0AA6E8);
  make_wire_line(screen, 18, 318, 988, 1, 0x0AA6E8);
  make_wire_line(screen, 18, 18, 1, 300, 0x0AA6E8);
  make_wire_line(screen, 1005, 18, 1, 300, 0x0AA6E8);
  make_wire_line(screen, 420, 38, 1, 248, 0x104B68);
  make_wire_line(screen, 746, 62, 1, 198, 0x104B68);

  make_panel(screen, 34, 38, 360, 248);
  make_panel(screen, 446, 62, 274, 198);
  make_panel(screen, 772, 62, 206, 198);

  make_label(screen, "BOOST PRESSURE", 58, 58, 260, 28, &mono_24, lv_color_hex(0x34CFFF));
  displayBoostLabel = make_label(screen, "--", 44, 104, 270, 70, &mono_48, lv_color_hex(0xE8F8FF));
  lv_obj_set_style_text_align(displayBoostLabel, LV_TEXT_ALIGN_RIGHT, 0);
  displayBoostUnitLabel = make_label(screen, "PSI", 320, 135, 64, 30, &mono_24, lv_color_hex(0x34CFFF));
  make_label(screen, "VAC", 58, 222, 64, 24, &mono_14, lv_color_hex(0x5288A1));
  make_label(screen, "BOOST", 286, 222, 88, 24, &mono_14, lv_color_hex(0x5288A1));

  displayBoostBar = lv_bar_create(screen);
  lv_obj_set_pos(displayBoostBar, 58, 250);
  lv_obj_set_size(displayBoostBar, 306, 14);
  lv_bar_set_range(displayBoostBar, 0, BOOST_BAR_MAX);
  lv_bar_set_value(displayBoostBar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(displayBoostBar, lv_color_hex(0x061018), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(displayBoostBar, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(displayBoostBar, lv_color_hex(0x116A93), LV_PART_MAIN);
  lv_obj_set_style_border_width(displayBoostBar, 1, LV_PART_MAIN);
  lv_obj_set_style_radius(displayBoostBar, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(displayBoostBar, lv_color_hex(0x34CFFF), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_color(displayBoostBar, lv_color_hex(0xE3313D), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_dir(displayBoostBar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
  lv_obj_set_style_radius(displayBoostBar, 0, LV_PART_INDICATOR);

  make_label(screen, "ENGINE", 470, 82, 120, 24, &mono_24, lv_color_hex(0x34CFFF));
  displayCanRateLabel = make_data_value(screen, "CANBUS RATE", "-- fps", 470, 122);
  displayCoolantLabel = make_data_value(screen, "COOLANT", "---.- C", 470, 188);

  make_label(screen, "AERO", 796, 82, 120, 24, &mono_24, lv_color_hex(0x34CFFF));
  displaySpoilerLabel = make_data_value(screen, "SPOILER", "---", 796, 122);
  displayClockLabel = make_data_value(screen, "CLOCK", "--:--", 796, 180);

  displayDimOverlay = lv_obj_create(screen);
  lv_obj_remove_style_all(displayDimOverlay);
  lv_obj_set_pos(displayDimOverlay, 0, 0);
  lv_obj_set_size(displayDimOverlay, 1024, 340);
  lv_obj_set_style_bg_color(displayDimOverlay, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(displayDimOverlay, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(displayDimOverlay, LV_OBJ_FLAG_CLICKABLE);

  lv_screen_load(screen);
}

// void handle_rpm() {
//   if (current.rpm != latest.rpm) {
//     char buf[16];
//     snprintf(buf, sizeof(buf), "%.0f rpm", latest.rpm);
//     lv_label_set_text(displayRpmLabel, buf);
//   }
// }

void handle_boost() {
  static bool hasDisplayBoostPressure = false;
  static float displayBoostPressureKpa = 0.0f;
  static float targetBoostPressureKpa = 0.0f;
  static float boostDisplayStepKpa = 0.0f;
  static float renderedBoostPressureKpa = 1000000.0f;
  static bool renderedHasManifoldPressure = false;
  static uint32_t lastBoostDisplayStepMs = 0;

  char buf[48];
  if (latest.hasManifoldPressure) {
    uint32_t now = millis();
    if (!hasDisplayBoostPressure || current.hasManifoldPressure != latest.hasManifoldPressure) {
      displayBoostPressureKpa = latest.boostPressureKpa;
      targetBoostPressureKpa = latest.boostPressureKpa;
      boostDisplayStepKpa = boost_display_step_kpa(displayBoostPressureKpa, targetBoostPressureKpa);
      hasDisplayBoostPressure = true;
      lastBoostDisplayStepMs = now;
    } else {
      if (latest.boostPressureKpa != targetBoostPressureKpa) {
        targetBoostPressureKpa = latest.boostPressureKpa;
        boostDisplayStepKpa = boost_display_step_kpa(displayBoostPressureKpa, targetBoostPressureKpa);
      }

      if (now - lastBoostDisplayStepMs < BOOST_DISPLAY_INTERVAL_MS) {
        return;
      }

      lastBoostDisplayStepMs = now;
      float delta = targetBoostPressureKpa - displayBoostPressureKpa;
      if (fabsf(delta) <= boostDisplayStepKpa) {
        displayBoostPressureKpa = targetBoostPressureKpa;
      } else {
        displayBoostPressureKpa += delta > 0.0f ? boostDisplayStepKpa : -boostDisplayStepKpa;
      }
    }

    if (!renderedHasManifoldPressure || renderedBoostPressureKpa != displayBoostPressureKpa) {
      float boostPsi = displayBoostPressureKpa * KPA_TO_PSI;
      float vacuumInHg = displayBoostPressureKpa < 0.0f ? -displayBoostPressureKpa * KPA_TO_INHG : 0.0f;
      bool inVacuum = displayBoostPressureKpa < 0.0f;
      snprintf(buf, sizeof(buf), "%.1f", inVacuum ? vacuumInHg : boostPsi);
      lv_label_set_text(displayBoostLabel, buf);
      lv_label_set_text(displayBoostUnitLabel, inVacuum ? "inHg" : "PSI");
      lv_bar_set_value(displayBoostBar, boost_bar_value(displayBoostPressureKpa), LV_ANIM_OFF);
      lv_obj_set_style_bg_color(displayBoostBar, lv_color_hex(inVacuum ? 0xE3313D : 0x29E675), LV_PART_INDICATOR);
      lv_obj_set_style_bg_grad_color(displayBoostBar, lv_color_hex(inVacuum ? 0xFF6B76 : 0x34CFFF), LV_PART_INDICATOR);
      lv_obj_set_style_shadow_color(displayBoostBar, lv_color_hex(inVacuum ? 0xE3313D : 0x34CFFF), LV_PART_INDICATOR);
      lv_obj_set_style_text_color(displayBoostUnitLabel, lv_color_hex(inVacuum ? 0xE3313D : 0x34CFFF), 0);
    }
  } else if (renderedHasManifoldPressure || current.hasManifoldPressure != latest.hasManifoldPressure) {
    hasDisplayBoostPressure = false;
    displayBoostPressureKpa = 0.0f;
    targetBoostPressureKpa = 0.0f;
    boostDisplayStepKpa = 0.0f;
    lv_label_set_text(displayBoostLabel, "--");
    lv_label_set_text(displayBoostUnitLabel, "PSI");
    lv_bar_set_value(displayBoostBar, 0, LV_ANIM_OFF);
  }

  renderedHasManifoldPressure = latest.hasManifoldPressure;
  renderedBoostPressureKpa = displayBoostPressureKpa;
}

void handle_clock() {
  if (current.secondsOfDay != latest.secondsOfDay ||
      current.hasClock != latest.hasClock) {
    char buf[16];
    if (latest.hasClock) {
      format_clock(buf, sizeof(buf), latest.secondsOfDay);
      lv_label_set_text(displayClockLabel, buf);
    } else {
      lv_label_set_text(displayClockLabel, "--:--");
    }
  }
}

void handle_lights_dim() {
  if (displayDimOverlay == nullptr) return;
  if (current.lightsOn != latest.lightsOn ||
      current.hasLights != latest.hasLights) {
    lv_obj_set_style_bg_opa(displayDimOverlay, latest.lightsOn ? DISPLAY_DIM_OPA : LV_OPA_TRANSP, 0);
    lv_obj_move_foreground(displayDimOverlay);
  }
}

void handle_coolant(){ 
  if (current.coolantTemp != latest.coolantTemp) {
    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%.1f C", latest.coolantTemp);
    lv_label_set_text(displayCoolantLabel, buf1);
  }
} 

// void handle_indicators() {
//   if (current.indicatorRaw != latest.indicatorRaw) {
//     char buf[18];
//     snprintf(buf, sizeof(buf), "%s 0x%02X", latest.indicatorsOn ? "ON" : "OFF", latest.indicatorRaw);
//     lv_label_set_text(displayIndicatorLabel, buf);
//   }
// }

void handle_spoiler() {
  if (current.spoilerRaw != latest.spoilerRaw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%d%%", latest.spoilerPercent);
    lv_label_set_text(displaySpoilerLabel, buf);
  }
}

void handle_can_status() {
  static uint32_t lastStatusRefreshMs = 0;
  static uint32_t lastRateMs = 0;
  static uint32_t lastRateFrames = 0;
  static float canFramesPerSecond = 0.0f;
  uint32_t now = millis();
  if (current.frames != latest.frames || current.mappedFrames != latest.mappedFrames || now - lastStatusRefreshMs > 500) {
    if (lastRateMs == 0) {
      lastRateMs = now;
      lastRateFrames = latest.frames;
    } else if (now - lastRateMs >= 1000) {
      uint32_t elapsed = now - lastRateMs;
      uint32_t frameDelta = latest.frames - lastRateFrames;
      canFramesPerSecond = (frameDelta * 1000.0f) / elapsed;
      lastRateMs = now;
      lastRateFrames = latest.frames;
    }

    lastStatusRefreshMs = now;
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f fps", canFramesPerSecond);
    lv_label_set_text(displayCanRateLabel, buf);
  }
}

static void finish_startup_if_ready() {
  if (startupComplete || millis() - startupStartMs < 500) return;

  if (displayReady) {
    lvgl_port_lock(-1);
    lv_obj_t *oldStartupScreen = startupScreen;
    create_display_ui();
    if (oldStartupScreen != nullptr) {
      lv_obj_delete(oldStartupScreen);
      startupScreen = nullptr;
    }
    lv_timer_handler();
    lvgl_port_unlock();
  }

  canDriverInstalled = init_can();

  startupComplete = true;
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
      Serial.println("LVGL init failed; delaying CAN startup for serial-only output");
      startupStartMs = millis();
      inited = true;
      return;
    }
    
    lvgl_port_lock(-1);

    create_startup_ui();
    lv_timer_handler();
    
    lvgl_port_unlock();
    
    startupStartMs = millis();
    Serial.println("Setup complete");
    inited = true;
}

void loop() {
  if (!inited) return;
  finish_startup_if_ready();
  if (!startupComplete) {
    delay(5);
    return;
  }

  request_manifold_pressure();
  poll_can();
  log_can_status();
  if (!displayReady) {
    log_displayless_status();
    delay(5);
    return;
  }
  lvgl_port_lock(-1);
  // handle_rpm();
  handle_boost();
  handle_clock();
  handle_lights_dim();
  handle_coolant();
  // handle_indicators();
  handle_spoiler();
  handle_can_status();
  memcpy(&current, &latest, sizeof(Data));
  lvgl_port_unlock();
  delay(5);
}

#endif
