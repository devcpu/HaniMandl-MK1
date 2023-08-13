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
 * Last Modified: 2023-08-13 03:42
 * Modified By: Johannes G.  Arlt
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
  volatile uint8_t angle_max_hard = 180;
  uint8_t angle_min_hard = 0;
  volatile uint8_t angle_max = 90;
  volatile uint8_t angle_min = 10;
  volatile uint8_t angle_fine = 45;
  uint8_t glass_tolerance = 0;
  uint16_t weights_filling = 330;
  uint8_t weight_empty = 30;
  String los_number = "";
  String date_filling = "";
  volatile uint16_t glass_count = 0;
  uint32_t boot_count = 0;
  IPAddress myIP;
  // AsyncWiFiManager wifiManager = NULL;

  HMConfig();
  void write_config();
  void read_config();
};

#endif  // LIB_HMCONFIG_HMCONFIG_H_
