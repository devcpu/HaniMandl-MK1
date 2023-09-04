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
 * Last Modified: 2023-09-04 19:57
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
  static Table2RData *getSystemInfoTable(void);
  static String getResetReason(RESET_REASON reason);
  //   static void reboot(AsyncWebServerRequest *request);
  static void disconnect(void);
  static void restartESP();
  static String getFlashMode();
  static String readSPIFFS2String(const String &path);
  static void writeString2SPIFFS(const String &path, const String &data);
};

#endif  // LIB_ESPHELPER_ESPHELPER_H_
