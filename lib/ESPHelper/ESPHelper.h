/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPHelper.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/ESPHelper
 * Description:
 * -----
 * Created Date: 2023-08-17 00:02
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-17 03:02
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_ESPHELPER_ESPHELPER_H_
#define LIB_ESPHELPER_ESPHELPER_H_

#include <Arduino.h>
#include <HMConfig.h>
#include <config.h>
#include <rom/rtc.h>

extern HMConfig cfg;

struct SystemData {
  const char *label;
  String value;
};

class ESPHelper {
 public:
  String getChipId(void);
  String getBuildDateAndTime(void);
  SystemData *getSystemInfoTable(void);
  String table2DGenerator(SystemData systemdata[], uint8_t size, boolean bold);
  String getResetReason(RESET_REASON reason);
  static void reboot(AsyncWebServerRequest *request);
  static void disconnect(void);
  static void restartESP();
  static String getFlashMode();
};

#endif  // LIB_ESPHELPER_ESPHELPER_H_