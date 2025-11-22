/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPHelper.h
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-17 00:02
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_ESPHELPER_ESPHELPER_H_
#define LIB_ESPHELPER_ESPHELPER_H_

#include <Arduino.h>
#include <Esp.h>
#include <HMConfig.h>
#include <SPIFFS.h>
#include <appconfig.h>
#include <rom/rtc.h>

struct Table2RData {
  String label;
  String value;
};

class ESPHelper {
 public:
  static String getChipId(void);
  static String getBuildDateAndTime(void);
  static Table2RData* getSystemInfoTable(void);
  static String getResetReason(RESET_REASON reason);
  //   static void reboot(AsyncWebServerRequest *request);
  static void disconnect(void);
  static void restartESP();
  static String getFlashMode();

  // Extended chip / platform helpers (ESP32 only - safe no-ops or simple
  // fallbacks elsewhere)
  static void printChipInfo(Stream& out = Serial);
  static String getChipModelString();
  static String getFeatureSummary();  // e.g. "WiFi BT BLE extFlash PSRAM"
  static String getShortId();         // condensed 6-byte MAC as hex
};

#endif  // LIB_ESPHELPER_ESPHELPER_H_
