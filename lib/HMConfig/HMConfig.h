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
 * Last Modified: 2023-08-22 18:53
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_HMCONFIG_HMCONFIG_H_
#define LIB_HMCONFIG_HMCONFIG_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWiFiManager.h>
#include <appconfig.h>

#include "esp_log.h"

typedef enum { RUN_MODUS_STOPPED, RUN_MODUS_HAND, RUN_MODUS_AUTO } RunModus;
class HMConfig {
 public:
  static HMConfig& instance() {
    static HMConfig _instance;
    log_e("Init HMConfig::instance");
    return _instance;
  }
  String version = SOFTWARE_VERSION;
  String beekeeping = "Beekeeping Germany";  // TODO(janusz)
  uint8_t angle_max_hard = 180;
  uint8_t angle_min_hard = 0;
  uint8_t angle_max = 90;
  uint8_t angle_min = 10;
  uint8_t angle_fine = 45;
  uint8_t weight_fine = static_cast<uint8_t>(weight_filling / 3);
  uint8_t glass_tolerance = 40;
  uint16_t weight_filling = 330;
  uint16_t weight_empty = 30;
  float weight_current = 30;
  String los_number = "leer";
  String date_filling = "";
  uint16_t glass_count = 0;
  uint32_t boot_count = 0;
  String localIP;
  uint32_t weight_calibrate = 0;
  RunModus run_modus = RUN_MODUS_STOPPED;
  // AsyncWiFiManager wifiManager = NULL;

  void write_config();
  void read_config();

 private:
  HMConfig(){};
  HMConfig(const HMConfig&);
  HMConfig& operator=(const HMConfig&);
};

#endif  // LIB_HMCONFIG_HMCONFIG_H_
