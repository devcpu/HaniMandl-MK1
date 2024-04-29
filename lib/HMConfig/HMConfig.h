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
 * Last Modified: 2024-04-28 14:26
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
  String server_user;
  String server_passwd;
  String server_ip;
  String server_port;
  String server_token;
  bool server_tls;
};

struct WlanConfig {
  String ip_address;
  String net_mask;
  String gw;
  String dns1;
  String dns2;
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
class HMConfig {
 public:
  static HMConfig& instance() {
    static HMConfig _instance;
    return _instance;
  }
  static String runmod2string(RunModus modus);
  static String fillingstatus2string(FillingStatus status);
  String version = SOFTWARE_VERSION;
  String beekeeping = PROGRAMM_NAME;  // TODO(janusz)

  /// @brief holds servo config
  ServoData servodata;

  HandMode hm = HAND_MODE_CLOSED;

  /// @brief Los Number for this honey and filling
  String los_number = "L001-02";

  /// @brief current date
  String date_filling = "";

  /// @brief netto target weight of glass
  uint16_t weight_filling = 500;

  /// @brief actual weight from scale
  int16_t weight_current = 0;

  /// @brief weight of honey in glass
  int16_t weight_honey = 0;

  /// @brief starts fine filling at this weight
  uint16_t weight_fine = static_cast<uint16_t>((weight_filling * 0.9));

  /// @brief  weight of an empty glass
  uint16_t glass_empty = 222;

  /// @brief toleranc for automatic detection
  uint8_t glass_tolerance = 5;

  /// @brief  how many glasses we are filling in this run
  uint16_t glass_count = 0;

  /// @brief how many this device were are booting
  uint32_t boot_count = 0;

  /// @brief local IP, comes from route or static configured
  String localIP;

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
