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
 * Last Modified: 2023-09-04 19:56
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <ArduinoJson.h>  // weiß der Geier warum man das nicht in die HWConfig.h eintagen kann.
#include <HMConfig.h>

String HMConfig::runmod2string(RunModus modus) {
  switch (modus) {
    case RUN_MODUS_AUTO:
      return String("RUN_MODUS_AUTO");
    case RUN_MODUS_HAND:
      return String("RUN_MODUS_HAND");
    case RUN_MODUS_STOPPED:
      return String("RUN_MODUS_STOPPED");
    default:
      return String("Unknown modus");
  }
}

String HMConfig::fillingstatus2string(FillingStatus status) {
  switch (status) {
    case FILLING_STATUS_FINE:
      return String("FINE");
    case FILLING_STATUS_OPEN:
      return String("OPEN");
    case FILLING_STATUS_CLOSED:
      return String("CLOSED");
    case FILLING_STATUS_FOLLOW_UP:
      return String("FOLLOW_UP");
    case FILLING_STATUS_STOPPED:
      return String("STOPPED");
    case FILLING_STATUS_STANDBY:
      return String("STANDBY");
    default:
      String retvar = String("unknown filling status: ");
      retvar += String(status);
      return retvar;
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
  ESPHelper::writeString2SPIFFS("hmconfig.json", output);
}

void HMConfig::readJsonConfig() {
  String input = ESPHelper::readSPIFFS2String("hmconfig.json");
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

  los_number = doc_json["los_number"].as<String>();
  date_filling = doc_json["date_filling"].as<String>();  // "2023-09-01"
  weight_filling = doc_json["weight_filling"].as<unsigned int>();    // 500
  weight_fine = doc_json["weight_fine"].as<unsigned int>();          // 400
  glass_empty = doc_json["glass_empty"].as<unsigned int>();          // 222
  glass_tolerance = doc_json["glass_tolerance"].as<unsigned int>();  // 22
  glass_count = doc_json["glass_count"].as<unsigned int>();          // 2000
  boot_count = doc_json["boot_count"].as<unsigned long>();           // 12000
  OFFSET = doc_json["OFFSET"].as<unsigned long>();                   // 456789
  SCALE = doc_json["SCALE"].as<double>();  // 346.02359

  JsonObject mqtt_server_json = doc_json["mqtt_server"];
  mqtt_server.server_user =
      mqtt_server_json["server_user"].as<String>();  // "asdfghjkloiuz"
  mqtt_server.server_passwd =
      mqtt_server_json["server_passwd"].as<String>();  // "asdfghjkloiuz"
  mqtt_server.server_ip =
      mqtt_server_json["server_ip"].as<String>();  // "100.100.100.100"
  mqtt_server.server_port =
      mqtt_server_json["server_port"].as<String>();  // "65000"
  mqtt_server.server_token = mqtt_server_json["server_token"].as<String>();
  mqtt_server.server_tls = mqtt_server_json["server_tls"].as<bool>();

  JsonObject api_server_json = doc_json["api_server"];
  api_server.server_user =
      api_server_json["server_user"].as<String>();  // "asdfghjkloiuz"
  api_server.server_passwd =
      api_server_json["server_passwd"].as<String>();  // "asdfghjkloiuz"
  api_server.server_ip =
      api_server_json["server_ip"].as<String>();  // "100.100.100.100"
  api_server.server_port =
      api_server_json["server_port"].as<String>();  // "65000"
  api_server.server_token = api_server_json["server_token"].as<String>();
  api_server.server_tls = api_server_json["server_tls"].as<bool>();

  JsonObject wlan_json = doc_json["wlan"];
  wlan.ip_address = wlan_json["ip_address"].as<String>();  // "100.100.100.100"
  wlan.net_mask = wlan_json["net_mask"].as<String>();      // "255.255.255.0"
  wlan.gw = wlan_json["gw"].as<String>();                  // "100.100.100.100"
  wlan.dns1 = wlan_json["dns1"].as<String>();              // "100.100.100.100"
  wlan.dns2 = wlan_json["dns2"].as<String>();              // "100.100.100.100"
}

// HMConfig::HMConfig(void) { log_e("Bad, very bad!!!"); }
