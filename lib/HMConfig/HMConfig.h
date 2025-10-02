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
 * Last Modified: 2025-10-02 21:14
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_HMCONFIG_HMCONFIG_H_
#define LIB_HMCONFIG_HMCONFIG_H_

#include <Arduino.h>
#include <ESPFS.h>
#include <appconfig.h>

#include "esp_log.h"

typedef enum {
  RUN_MODUS_STOPPED,
  RUN_MODUS_HAND,
  RUN_MODUS_AUTO,
  RUN_MODUS_TEST
} RunModus;

struct MQTTServerData {
  char server_user[WEB_INPUT_MAX_LENGTH];
  char server_passwd[WEB_INPUT_MAX_LENGTH];
  char server_ip[16];     // IPv4: xxx.xxx.xxx.xxx
  char server_port[6];    // Port: 65535
  char server_token[64];  // Token can be longer
  bool server_tls;
};

struct WlanConfig {
  char ip_address[16];  // IPv4: xxx.xxx.xxx.xxx
  char net_mask[16];    // IPv4: xxx.xxx.xxx.xxx
  char gw[16];          // IPv4: xxx.xxx.xxx.xxx
  char dns1[16];        // IPv4: xxx.xxx.xxx.xxx
  char dns2[16];        // IPv4: xxx.xxx.xxx.xxx
};

typedef enum {
  FILLING_STATUS_STANDBY,
  FILLING_STATUS_OPEN,
  FILLING_STATUS_FINE,
  FILLING_STATUS_CLOSED,
  FILLING_STATUS_FOLLOW_UP,
  FILLING_STATUS_STOPPED,
} FillingStatus;

typedef enum {
  SERVO_STATUS_CLOSED,
  SERVO_STATUS_OPEN,
  SERVO_STATUS_FINE,
  SERVO_STATUS_STOPPED,
} ServoStatus;

typedef enum {
  SCALE_STATUS_STANDBY,
  SCALE_STATUS_TARE,
  SCALE_STATUS_CALIBRATE,
  SCALE_STATUS_MEASURE,
  SCALE_STATUS_STOPPED,
} ScaleStatus;

typedef enum {
  SCALE_MODE_AUTO,
  SCALE_MODE_MANUAL,
  SCALE_MODE_CALIBRATE,
  SCALE_MODE_STOPPED,
} ScaleMode;

typedef enum {
  HAND_MODE_CLOSED,
  HAND_MODE_OPEN,
  HAND_MODE_FINE,
} HandMode;

/// @brief all data relatet servo
struct ServoData {
  /// @brief max degree of Servo
  uint8_t angle_max_hard = 120;

  /// @brief min degree of Servo
  uint8_t angle_min_hard = 0;

  /// @brief max degree of MHMKI full open
  uint8_t angle_max = 110;

  /// @brief min degree of MHMKI closed
  uint8_t angle_min = 3;

  /// @brief degree for fine filling
  uint8_t angle_fine = 45;

  /// @brief test angle
  uint16_t angle_test = 0;
};

// TODO - change into subclasses instand of structs?
// TODO - read and write from json
class HMConfig {
 public:
  static HMConfig& instance() {
    static HMConfig _instance;
    return _instance;
  }
  static const char* runmod2string(RunModus modus);
  static const char* fillingstatus2string(FillingStatus status);
  const char* version = SOFTWARE_VERSION;  // TODO(janusz)
  char beekeeping[64] = PROGRAMM_NAME;     // Made changeable char array

  /// @brief holds servo config
  ServoData servodata;

  HandMode hm = HAND_MODE_CLOSED;

  /// @brief Los Number for this honey and filling
  char los_number[16] = "L001-02";

  /// @brief current date
  char date_filling[32] = "";

  /// @brief netto target weight of glass
  uint16_t weight_filling = 500;

  /// @brief actual weight from scale
  int16_t weight_current = 0;

  /// @brief weight of honey in glass
  int16_t weight_honey = 0;

  /// @brief starts fine filling at this weight
  // FIXME - one place only see WebServerX.cpp -> WebServer->on("/setupfilling"
  uint16_t weight_fine = static_cast<uint16_t>((weight_filling - 50));

  /// @brief  weight of an empty glass
  uint16_t glass_empty = 222;

  /// @brief toleranc for automatic detection
  uint8_t glass_tolerance = 5;

  /// @brief  how many glasses we are filling in this run
  uint16_t glass_count = 0;

  /// @brief how many this device were are booting
  uint32_t boot_count = 0;

  /// @brief local IP, comes from route or static configured
  char localIP[16] = "";  // IPv4: xxx.xxx.xxx.xxx

  /// @brief weight for calibration
  uint32_t weight_calibrate = 509;

  /// @brief offset for this particular load cell
  int32_t OFFSET = -92840;

  /// @brief scale for this particular load cell
  int32_t SCALE = 346.023590;

  /// @brief modus now auto / hand / stopped
  RunModus run_modus = RUN_MODUS_STOPPED;

  // AsyncWiFiManager wifiManager = NULL;

  /// @brief status servo
  FillingStatus fs = FILLING_STATUS_CLOSED;

  MQTTServerData mqtt_server;

  MQTTServerData api_server;

  WlanConfig wlan;

  void writeJsonConfig();
  void readJsonConfig();

 private:
  HMConfig(){};
  HMConfig(const HMConfig&);
  HMConfig& operator=(const HMConfig&);
};

#endif  // LIB_HMCONFIG_HMCONFIG_H_
