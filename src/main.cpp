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

#include <MQTTHelper.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <main.h>
// LED_BUILTIN optional: not required in normal mode

#include "cooperative_loop.h"

ESPFS espfs;
HX711 scale;
Servo servo;
Glass glass;
// DNSServer dns;
Ticker ticker;

#ifdef ESP32
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
#if CORE_DEBUG_LEVEL > 0
  uint16_t rawBaseline = touchRead(PIN_STOP_TOUCH_CHANNEL);
  log_i("Touch STOP baseline (T7/GPIO27) raw=%u threshold=%u", rawBaseline,
        touchThreshold);
#else
  (void)touchRead(PIN_STOP_TOUCH_CHANNEL);  // Dummy read to stabilize
#endif
#endif
  esp_reset_reason_t rr = esp_reset_reason();
  log_i("BOOT: reason=%d", (int)rr);
  (void)rr;  // Suppress unused warning when logging disabled
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
  log_i("SETUP: MQTTHelper init");
  MQTTHelper::instance().begin();
  log_i("SETUP: WebserverStart()");
  WebserverStart();
  log_i("SETUP: setupLoadcell()");
  setupLoadcell();
  log_i("SETUP: setupServo()");
  setupServo();
  pinMode(PIN_BUZZER, OUTPUT);
  log_i("SETUP: beekeeping=%s", HMConfig::instance().beekeeping);
  log_i("SETUP: startSystemTasks()");
  initCooperativeLoop();
  log_i("SETUP: Done - cooperative loop initialized");

  // OTA Rollback Protection: Validate firmware after successful boot
  // Wait for critical systems to initialize (WiFi, WebServer, MQTT, Scale,
  // Servo)
  delay(10000);  // 10 seconds for system stabilization

  // Check if this is first boot after OTA update
  const esp_partition_t* running = esp_ota_get_running_partition();
  esp_ota_img_states_t ota_state;
  if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
      log_i("OTA: First boot after update - validating firmware...");

      // Validate critical systems
      bool systemsOK = true;

      // Check WiFi - but be lenient: WiFi might be temporarily unavailable
      // Only fail if WiFi system itself crashed (not just no connection)
      if (WiFi.status() == WL_NO_SHIELD || WiFi.getMode() == WIFI_MODE_NULL) {
        log_e("OTA Validation FAILED: WiFi system not initialized");
        systemsOK = false;
      } else if (WiFi.status() == WL_CONNECTED) {
        log_i("OTA Validation: WiFi connected - OK");
      } else {
        log_w(
            "OTA Validation: WiFi not connected (router down?) - accepting "
            "anyway");
        // Don't fail validation just because router is offline
        // Firmware WiFi stack is working if we get here
      }

      // Check Scale
      if (!scale.is_ready()) {
        log_w("OTA Validation WARNING: Scale not ready (non-critical)");
        // Don't fail on scale, might be temporarily unavailable
      }

      // Additional checks could be added here (WebServer, Servo, etc.)

      if (systemsOK) {
        log_i("OTA: Firmware validation PASSED - marking as valid");
        esp_ota_mark_app_valid_cancel_rollback();
      } else {
        log_e(
            "OTA: Firmware validation FAILED - rollback will occur on next "
            "reboot!");
        // Don't mark as valid - rollback will happen automatically
      }
    } else if (ota_state == ESP_OTA_IMG_VALID) {
      log_i("OTA: Running validated firmware (partition: %s)", running->label);
    } else if (ota_state == ESP_OTA_IMG_INVALID) {
      log_w("OTA: Running partition marked as INVALID");
    }
  }
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
  tickSensor();
  tickServo();
  tickWiFi();
  tickBuzzer();
  tickWsDispatch();
  tickHousekeeping();
  yield();
  delay(5);
}

#endif
