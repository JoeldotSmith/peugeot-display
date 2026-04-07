#define CONFIG_LVGL_PORT_ROTATION_DEGREE 0
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lv_conf.h"
#include "lvgl_v8_port.h"
#include "ui.h"

#include <esp_now.h>
#include <WiFi.h>


using namespace esp_panel::drivers;
using namespace esp_panel::board;


Board *board = nullptr;
static lv_chart_series_t *series;

bool inited = false;

struct Data {
  float voltage;
  float boost;
  float coolantTemp;
  float oilPressure;
};
volatile Data latest;
volatile Data current;
volatile bool dataReady = false;

// 0 = disconnected from ESPNOW, 1 = disconnected from ELM, 2 = fully connected 
int previousConnected = 0; 
int connected = 0; 
unsigned long lastReceive = 0;


void handle_connection() {
  if (previousConnected != connected) {
    switch (connected) {
      case 0:
        lv_obj_set_style_bg_color(ui_Indicator, lv_palette_main(LV_PALETTE_RED), 0);
        break;
      case 1:
        lv_obj_set_style_bg_color(ui_Indicator, lv_palette_main(LV_PALETTE_AMBER), 0);
        break;
      case 2:
        lv_obj_set_style_bg_color(ui_Indicator, lv_palette_main(LV_PALETTE_GREEN), 0);
        break;
    }

    previousConnected = connected;
  }
} 

void handle_boost() {
    if (current.boost != latest.boost) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", latest.boost);
        lv_label_set_text(boostLabel, buf);
        if(boostBar) {
            lv_bar_set_value(boostBar, (int)(latest.boost * 10), LV_ANIM_OFF);
        } else {
            Serial.println("boostBar is NULL!");
        }
    }
}

void handle_voltage(){ 
  if (current.voltage != latest.voltage) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f   V", latest.voltage);
    lv_label_set_text(ui_Label1, buf);
  }
} 

void handle_coolant(){ 
  if (current.coolantTemp != latest.coolantTemp) {
    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%.1f  °C", latest.coolantTemp);
    lv_label_set_text(ui_Label2, buf1);
  }
} 

void handle_oil_pressure(){ 
  if (current.oilPressure != latest.oilPressure) {
    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%.1f PSI", latest.oilPressure);
    lv_label_set_text(oilPressureLabel, buf1);
  }
} 
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
    float v, c, b;
    if (sscanf((const char*)data, "%f-%f-%f", &v, &c, &b) == 3) {
        if (v == -1.0f && c == -1.0f && b == -1.0f) {
          connected = 1; // connected to ESPNOW but not connected to ELM
        } else {
            latest.voltage = v;
            latest.coolantTemp = c;
            latest.boost = b;
            dataReady = true;
            connected = 2;
        }
        lastReceive = millis();
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing board");

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
      Serial.println("ESP-NOW init failed");
      return;
    }
    esp_now_register_recv_cb(onReceive);
    
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
    
    assert(board->begin());
    
    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());
    
    lvgl_port_lock(-1);
    ui_init();
    lvgl_port_unlock();
    
    Serial.println("Setup complete");
    inited = true;
}

void loop() {
  if (!inited) return;
  lv_timer_handler(); 
  lvgl_port_lock(-1);

  latest.voltage += 0.1f;
  latest.boost += 0.1f;
  latest.boost = latest.boost * 1.2f;
  latest.oilPressure += 0.1f;
  latest.coolantTemp += 0.1f;
  if (latest.voltage > 10.0f) latest.voltage = 0.0f;
  if (latest.boost > 15.0f) latest.boost = 0.0f;
  if (latest.coolantTemp > 150.0f) latest.coolantTemp = 0.0f;
  if (latest.oilPressure > 125.0f) latest.oilPressure = 0.0f;

  handle_voltage();
  handle_boost();
  handle_coolant();
  handle_oil_pressure();
  memcpy((void*)&current, (const void*)&latest, sizeof(Data));
  
  lvgl_port_unlock();
  delay(5);
}
