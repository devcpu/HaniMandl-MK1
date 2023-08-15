/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /Config.h
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/Config
 * Description:
 * -----
 * Created Date: 2023-08-12 20:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-15 02:43
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_HMCONFIG_HMCONFIG_H_
#define LIB_HMCONFIG_HMCONFIG_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWiFiManager.h>
#include <config.h>

class HMConfig {
 public:
  String version = SOFTWARE_VERSION;
  String beekeeping = "Beekeeping Germany";
  uint8_t angle_max_hard = 180;
  uint8_t angle_min_hard = 0;
  uint8_t angle_max = 90;
  uint8_t angle_min = 10;
  uint8_t angle_fine = 45;
  uint8_t weight_fine = int(weight_filling / 3);
  uint8_t glass_tolerance = 0;
  uint16_t weight_filling = 330;
  uint8_t weight_empty = 30;
  String los_number = "leer";
  String date_filling = "";
  uint16_t glass_count = 0;
  uint32_t boot_count = 0;
  String localIP;
  uint32_t weight_calibrate = 0;
  // AsyncWiFiManager wifiManager = NULL;

  HMConfig();
  void write_config();
  void read_config();
};

#endif  // LIB_HMCONFIG_HMCONFIG_H_
