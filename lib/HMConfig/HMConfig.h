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
 * Last Modified: 2023-09-05 13:29
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_HMCONFIG_HMCONFIG_H_
#define LIB_HMCONFIG_HMCONFIG_H_

#include <Arduino.h>
#include <ESPFS.h>
#include <appconfig.h>

#include "esp_log.h"

typedef enum { RUN_MODUS_STOPPED, RUN_MODUS_HAND, RUN_MODUS_AUTO } RunModus;

struct ServerData {
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

/// @brief all data relatet servo
struct ServoData {
  /// @brief max degree of Servo
  uint8_t angle_max_hard = 180;

  /// @brief min degree of Servo
  uint8_t angle_min_hard = 0;

  /// @brief max degree of MHMKI full open
  uint8_t angle_max = 120;

  /// @brief min degree of MHMKI closed
  uint8_t angle_min = 10;

  /// @brief degree for fine filling
  uint8_t angle_fine = 45;
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
  String beekeeping = "Beekeeping Germany";  // TODO(janusz)

  /// @brief holds servo config
  ServoData servodata;

  /// @brief from [Start] - Button
  bool start = false;

  /// @brief Los Number for this honey and filling
  String los_number = "L001-23";

  /// @brief current date
  String date_filling = "";

  /// @brief netto target weight of glass
  uint16_t weight_filling = 500;

  /// @brief actual weight from scale
  int16_t weight_current = 0;

  /// @brief starts fine filling at this weight
  uint16_t weight_fine = static_cast<uint16_t>((weight_filling / 3));

  /// @brief  weight of an empty glass
  uint16_t glass_empty = 222;

  /// @brief toleranc for automatic detection
  uint8_t glass_tolerance = glass_empty / 10;

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

  ServerData mqtt_server;

  ServerData api_server;

  WlanConfig wlan;

  void writeJsonConfig();
  void readJsonConfig();

 private:
  HMConfig(){};
  HMConfig(const HMConfig&);
  HMConfig& operator=(const HMConfig&);
};

#endif  // LIB_HMCONFIG_HMCONFIG_H_
