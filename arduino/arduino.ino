#define CONFIG_LVGL_PORT_ROTATION_DEGREE 0
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lv_conf.h"
#include <Adafruit_ADS1X15.h>
#include "lvgl_v8_port.h"
#include "ui.h"
#include "driver/twai.h"



using namespace esp_panel::drivers;
using namespace esp_panel::board;


Board *board = nullptr;

bool inited = false;
bool canDriverInstalled = false;

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
// Adafruit_ADS1115 ads;

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
  while (twai_receive(&message, 0) == ESP_OK) {
    decode_can_message(message);
  }
}


void handle_rpm() {
  if (current.rpm != latest.rpm) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f rpm", latest.rpm);
    lv_label_set_text(rpmLabel, buf);
  }
}

void handle_boost() {
  if (current.boostPressureKpa != latest.boostPressureKpa ||
      current.manifoldPressureKpa != latest.manifoldPressureKpa ||
      current.hasManifoldPressure != latest.hasManifoldPressure) {
    char buf[48];
    if (latest.hasManifoldPressure) {
      snprintf(buf, sizeof(buf), "%.0f", latest.boostPressureKpa);
      lv_label_set_text(boostLabel, buf);
      snprintf(buf, sizeof(buf), "%.0f kPa abs", latest.manifoldPressureKpa);
      lv_label_set_text(manifoldPressureLabel, buf);
      lv_bar_set_value(boostBar, (int)latest.boostPressureKpa, LV_ANIM_OFF);
    } else {
      lv_label_set_text(boostLabel, "--");
      lv_label_set_text(manifoldPressureLabel, "waiting");
      lv_bar_set_value(boostBar, 0, LV_ANIM_OFF);
    }
  }
}

void handle_coolant(){ 
  if (current.coolantTemp != latest.coolantTemp) {
    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%.1f C", latest.coolantTemp);
    lv_label_set_text(ui_Label2, buf1);
  }
} 

void handle_indicators() {
  if (current.indicatorRaw != latest.indicatorRaw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%s 0x%02X", latest.indicatorsOn ? "ON" : "OFF", latest.indicatorRaw);
    lv_label_set_text(indicatorLabel, buf);
  }
}

void handle_spoiler() {
  if (current.spoilerRaw != latest.spoilerRaw) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%d%% 0x%02X", latest.spoilerPercent, latest.spoilerRaw);
    lv_label_set_text(spoilerLabel, buf);
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
    lv_label_set_text(canStatusLabel, buf);
  }
}

void setup()
{
    Serial.begin(1000000);

    Serial.println("Initializing board");

    // Wire.begin(8, 9);
    // ads.begin();
    // ads.setGain(0);

    board = new Board();
    board->init();
    // static_cast<esp_panel::drivers::BusI2C *>(board->getTouch()->getBus())->configI2C_HostSkipInit();
    // board->getIO_Expander()->skipInitHost();
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
    
    assert(board->begin());
    
    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());
    
    lvgl_port_lock(-1);
    ui_init();
    lv_label_set_text(boostLabel, "--");
    lv_label_set_text(manifoldPressureLabel, "waiting");
    lv_label_set_text(canStatusLabel, "CAN starting");
    lvgl_port_unlock();

    canDriverInstalled = init_can();
    lvgl_port_lock(-1);
    lv_label_set_text(canStatusLabel, canDriverInstalled ? "waiting for CAN" : "TWAI failed");
    lvgl_port_unlock();
    
    Serial.println("Setup complete");
    inited = true;
}

void loop() {
  if (!inited) return;
  lv_timer_handler(); 
  request_manifold_pressure();
  poll_can();
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
