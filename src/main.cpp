/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /main.cpp
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-12 15:55
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef UNIT_TEST

// Isolation mode removed (was used for debugging reboot loop)

#include <esp_system.h>
#include <main.h>
// LED_BUILTIN optional: not required in normal mode

#include "freertos_setup.h"

ESPFS espfs;
HX711 scale;
Servo servo;
Glass glass;
// DNSServer dns;
Ticker ticker;

#ifdef ESP32
#ifndef TOUCH_THRESHOLD_DEFAULT
#define TOUCH_THRESHOLD_DEFAULT 40  // empirical default; adjust in field
#endif
static const uint16_t touchThreshold = TOUCH_THRESHOLD_DEFAULT;
// Forward declaration of ISR (placed before use for clarity)
void IRAM_ATTR isrStop();
#endif

void setup() {
  Serial.begin(921600);
  delay(1000);
#ifdef ESP32
  // Re-enable touch interrupt for emergency stop (T7 -> GPIO27)
  touchAttachInterrupt(PIN_STOP_TOUCH_CHANNEL, isrStop, touchThreshold);
  // Baseline read (single) for potential adaptive threshold (future use)
  uint16_t rawBaseline = touchRead(PIN_STOP_TOUCH_CHANNEL);
  log_i("Touch STOP baseline (T7/GPIO27) raw=%u threshold=%u", rawBaseline,
        touchThreshold);
#endif
  esp_reset_reason_t rr = esp_reset_reason();
  log_i("BOOT: reason=%d", (int)rr);
  log_i("SETUP: Start (build %s %s)", __DATE__, __TIME__);
  HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
  log_i("SETUP: espfs.setup()");
  espfs.setup();
  log_i("SETUP: readJsonConfig()");
  HMConfig::instance().readJsonConfig();
  // Reset glass after config is loaded to initialize cutoff_weight correctly
  glass.reset();
  log_i("SETUP: glass.reset() after config load - cutoff_weight=%d",
        glass.cutoff_weight);
  log_i("SETUP: setupWifi()");
  setupWifi();
  log_i("SETUP: WebserverStart()");
  WebserverStart();
  log_i("SETUP: setupLoadcell()");
  setupLoadcell();
  log_i("SETUP: setupServo()");
  setupServo();
  pinMode(PIN_BUZZER, OUTPUT);
  log_i("SETUP: beekeeping=%s", HMConfig::instance().beekeeping);
  log_i("SETUP: startSystemTasks()");
  startSystemTasks();
  log_i("SETUP: Done - tasks started");
}

float weight_current = 0;

#ifdef ESP32
void IRAM_ATTR isrStop() {
  static uint32_t last = 0;
  uint32_t now = millis();                    // safe in ISR on ESP32
  if (now - last < STOP_DEBOUNCE_MS) return;  // debounce
  last = now;
  HMConfig::instance().emergency_stop = true;
}
#endif

void loop() {
  yield();
  delay(2);
}

#endif
