/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /Config.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/Config
 * Description:
 * -----
 * Created Date: 2023-08-12 20:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2024-04-27 03:15
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <ArduinoJson.h>  // weiß der Geier warum man das nicht in die HWConfig.h eintagen kann.
#include <HMConfig.h>

extern ESPFS espfs;

const char* HMConfig::runmod2string(RunModus modus) {
  switch (modus) {
    case RUN_MODUS_AUTO:
      return "RUN_MODUS_AUTO";
    case RUN_MODUS_HAND:
      return "RUN_MODUS_HAND";
    case RUN_MODUS_STOPPED:
      return "RUN_MODUS_STOPPED";
    case RUN_MODUS_TEST:
      return "RUN_MODUS_TEST";
    default:
      return "Unknown modus";
  }
}

const char* HMConfig::fillingstatus2string(FillingStatus status) {
  switch (status) {
    case FILLING_STATUS_FINE:
      return "FINE";
    case FILLING_STATUS_OPEN:
      return "OPEN";
    case FILLING_STATUS_CLOSED:
      return "CLOSED";
    case FILLING_STATUS_FOLLOW_UP:
      return "FOLLOW_UP";
    case FILLING_STATUS_STOPPED:
      return "STOPPED";
    case FILLING_STATUS_STANDBY:
      return "STANDBY";
    default:
      return "unknown filling status";
  }
}

void HMConfig::writeJsonConfig() {
  String output;
  StaticJsonDocument<1024> doc_json;

  JsonObject servodata_json = doc_json.createNestedObject("servodata");
  servodata_json["angle_max_hard"] = servodata.angle_max_hard;
  servodata_json["angle_min_hard"] = servodata.angle_min_hard;
  servodata_json["angle_max"] = servodata.angle_max;
  servodata_json["angle_min"] = servodata.angle_min;
  servodata_json["angle_fine"] = servodata.angle_fine;

  doc_json["los_number"] = los_number;
  doc_json["date_filling"] = date_filling;
  doc_json["weight_filling"] = weight_filling;
  doc_json["weight_fine"] = weight_fine;
  doc_json["glass_empty"] = glass_empty;
  doc_json["glass_tolerance"] = glass_tolerance;
  doc_json["glass_count"] = glass_count;
  doc_json["boot_count"] = boot_count;
  doc_json["OFFSET"] = OFFSET;
  doc_json["SCALE"] = SCALE;

  JsonObject mqtt_server_json = doc_json.createNestedObject("mqtt_server");
  mqtt_server_json["server_user"] = mqtt_server.server_user;
  mqtt_server_json["server_passwd"] = mqtt_server.server_passwd;
  mqtt_server_json["server_ip"] = mqtt_server.server_ip;
  mqtt_server_json["server_port"] = mqtt_server.server_port;
  mqtt_server_json["server_token"] = mqtt_server.server_token;
  mqtt_server_json["server_tls"] = mqtt_server.server_tls;

  JsonObject api_server_json = doc_json.createNestedObject("api_server");
  api_server_json["server_user"] = api_server.server_user;
  api_server_json["server_passwd"] = api_server.server_passwd;
  api_server_json["server_ip"] = api_server.server_ip;
  api_server_json["server_port"] = api_server.server_port;
  api_server_json["server_token"] = api_server.server_token;
  api_server_json["server_tls"] = api_server.server_tls;

  serializeJson(doc_json, output);
  espfs.writeString("hmconfig.json", output);
}

void HMConfig::readJsonConfig() {
  String input = espfs.readString("hmconfig.json");
  if (input.length() <= 0) {
    return;
  }

  StaticJsonDocument<1536> doc_json;

  DeserializationError error = deserializeJson(doc_json, input);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject servodata_json = doc_json["servodata"];
  servodata.angle_max_hard =
      servodata_json["angle_max_hard"].as<unsigned int>();  // 180
  servodata.angle_min_hard =
      servodata_json["angle_min_hard"].as<unsigned int>();               // 0
  servodata.angle_max = servodata_json["angle_max"].as<unsigned int>();  // 180
  servodata.angle_min = servodata_json["angle_min"].as<unsigned int>();  // 10
  servodata.angle_fine = servodata_json["angle_fine"].as<unsigned int>();  // 45

  const char* temp_los = doc_json["los_number"] | "L001-02";
  strlcpy(los_number, temp_los, sizeof(los_number));

  const char* temp_date = doc_json["date_filling"] | "";
  strlcpy(date_filling, temp_date, sizeof(date_filling));
  weight_filling = doc_json["weight_filling"].as<unsigned int>();    // 500
  weight_fine = doc_json["weight_fine"].as<unsigned int>();          // 400
  glass_empty = doc_json["glass_empty"].as<unsigned int>();          // 222
  glass_tolerance = doc_json["glass_tolerance"].as<unsigned int>();  // 22
  glass_count = doc_json["glass_count"].as<unsigned int>();          // 2000
  boot_count = doc_json["boot_count"].as<unsigned long>();           // 12000
  OFFSET = doc_json["OFFSET"].as<unsigned long>();                   // 456789
  SCALE = doc_json["SCALE"].as<double>();  // 346.02359

  JsonObject mqtt_server_json = doc_json["mqtt_server"];
  const char* temp_user = mqtt_server_json["server_user"] | "";
  strlcpy(mqtt_server.server_user, temp_user, sizeof(mqtt_server.server_user));

  const char* temp_passwd = mqtt_server_json["server_passwd"] | "";
  strlcpy(mqtt_server.server_passwd, temp_passwd,
          sizeof(mqtt_server.server_passwd));

  const char* temp_ip = mqtt_server_json["server_ip"] | "";
  strlcpy(mqtt_server.server_ip, temp_ip, sizeof(mqtt_server.server_ip));

  const char* temp_port = mqtt_server_json["server_port"] | "";
  strlcpy(mqtt_server.server_port, temp_port, sizeof(mqtt_server.server_port));

  const char* temp_token = mqtt_server_json["server_token"] | "";
  strlcpy(mqtt_server.server_token, temp_token,
          sizeof(mqtt_server.server_token));

  mqtt_server.server_tls = mqtt_server_json["server_tls"].as<bool>();

  JsonObject api_server_json = doc_json["api_server"];
  const char* temp_api_user = api_server_json["server_user"] | "";
  strlcpy(api_server.server_user, temp_api_user,
          sizeof(api_server.server_user));

  const char* temp_api_passwd = api_server_json["server_passwd"] | "";
  strlcpy(api_server.server_passwd, temp_api_passwd,
          sizeof(api_server.server_passwd));

  const char* temp_api_ip = api_server_json["server_ip"] | "";
  strlcpy(api_server.server_ip, temp_api_ip, sizeof(api_server.server_ip));

  const char* temp_api_port = api_server_json["server_port"] | "";
  strlcpy(api_server.server_port, temp_api_port,
          sizeof(api_server.server_port));

  const char* temp_api_token = api_server_json["server_token"] | "";
  strlcpy(api_server.server_token, temp_api_token,
          sizeof(api_server.server_token));

  api_server.server_tls = api_server_json["server_tls"].as<bool>();

  JsonObject wlan_json = doc_json["wlan"];
  const char* temp_wlan_ip = wlan_json["ip_address"] | "";
  strlcpy(wlan.ip_address, temp_wlan_ip, sizeof(wlan.ip_address));

  const char* temp_wlan_mask = wlan_json["net_mask"] | "";
  strlcpy(wlan.net_mask, temp_wlan_mask, sizeof(wlan.net_mask));

  const char* temp_wlan_gw = wlan_json["gw"] | "";
  strlcpy(wlan.gw, temp_wlan_gw, sizeof(wlan.gw));

  const char* temp_wlan_dns1 = wlan_json["dns1"] | "";
  strlcpy(wlan.dns1, temp_wlan_dns1, sizeof(wlan.dns1));

  const char* temp_wlan_dns2 = wlan_json["dns2"] | "";
  strlcpy(wlan.dns2, temp_wlan_dns2, sizeof(wlan.dns2));
}

// HMConfig::HMConfig(void) { log_e("Bad, very bad!!!"); }
