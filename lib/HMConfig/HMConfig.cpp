/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /Config.cpp
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-12 20:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2025-11-17 22:30
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
  StaticJsonDocument<1536> doc_json;
  doc_json["config_version"] = config_version;

  JsonObject servodata_json = doc_json.createNestedObject("servodata");
  servodata_json["angle_max_hard"] = servodata.angle_max_hard;
  servodata_json["angle_min_hard"] = servodata.angle_min_hard;
  servodata_json["angle_max"] = servodata.angle_max;
  servodata_json["angle_min"] = servodata.angle_min;
  servodata_json["angle_fine"] = servodata.angle_fine;

  doc_json["bucket_number"] = bucket_number;
  doc_json["harvest_date"] = harvest_date;
  doc_json["harvest_number"] = harvest_number;
  doc_json["date_filling"] = date_filling;
  doc_json["weight_filling"] = weight_filling;
  doc_json["weight_fine"] = weight_fine;
  doc_json["glass_empty"] = glass_empty;
  doc_json["glass_tolerance"] = glass_tolerance;
  doc_json["boot_count"] = boot_count;
  doc_json["OFFSET"] = OFFSET;
  doc_json["SCALE"] = SCALE;

  JsonObject mqtt_server_json = doc_json.createNestedObject("mqtt_server");
  mqtt_server_json["server_user"] = mqtt_server.server_user;
  mqtt_server_json["server_passwd"] = mqtt_server.server_passwd;
  mqtt_server_json["server_ip"] = mqtt_server.server_ip;
  mqtt_server_json["server_port"] = mqtt_server.server_port;
  mqtt_server_json["server_token"] = mqtt_server.server_token;
  mqtt_server_json["server_topic"] = mqtt_server.server_topic;
  mqtt_server_json["server_tls"] = mqtt_server.server_tls;

  JsonObject api_server_json = doc_json.createNestedObject("api_server");
  api_server_json["server_user"] = api_server.server_user;
  api_server_json["server_passwd"] = api_server.server_passwd;
  api_server_json["server_ip"] = api_server.server_ip;
  api_server_json["server_port"] = api_server.server_port;
  api_server_json["server_token"] = api_server.server_token;
  api_server_json["server_tls"] = api_server.server_tls;

  serializeJson(doc_json, output);
  espfs.writeString("/hmconfig.json", output);
}

void HMConfig::readJsonConfig() {
  String input = espfs.readString("/hmconfig.json");
  if (input.length() <= 0) {
    // Try template fallback
    String tmpl = espfs.readString("/hmconfig_template.json");
    if (tmpl.length() > 0) {
      log_w("hmconfig.json missing -> seeding from hmconfig_template.json");
      input =
          tmpl;  // parse template, then persisted file will be written later
    } else {
      log_w(
          "hmconfig.json & template missing -> using compiled defaults and "
          "writing new file");
      config_version = CONFIG_VERSION;
      writeJsonConfig();
      return;
    }
  }

  StaticJsonDocument<1536> doc_json;

  DeserializationError error = deserializeJson(doc_json, input);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    // Corrupt file: keep current in-memory defaults; attempt template fallback
    // once
    String tmpl = espfs.readString("/hmconfig_template.json");
    if (tmpl.length() > 0 && tmpl != input) {
      DeserializationError err2 = deserializeJson(doc_json, tmpl);
      if (!err2) {
        log_w("Recovered config from template after corrupt hmconfig.json");
      } else {
        log_w("Template also invalid; writing defaults");
        writeJsonConfig();
        return;
      }
    } else {
      writeJsonConfig();
      return;
    }
  }

  bool upgradeNeeded = false;
  uint16_t fileVersion = doc_json["config_version"] | 0;  // 0 => missing field
  if (fileVersion == 0) {
    // pre-versioned file
    upgradeNeeded = true;
    config_version = CONFIG_VERSION;
  } else if (fileVersion < CONFIG_VERSION) {
    upgradeNeeded = true;
    config_version = CONFIG_VERSION;
  } else {
    config_version = fileVersion;
  }

  JsonObject servodata_json = doc_json["servodata"];
  servodata.angle_max_hard =
      servodata_json["angle_max_hard"].as<unsigned int>();  // 180
  servodata.angle_min_hard =
      servodata_json["angle_min_hard"].as<unsigned int>();               // 0
  servodata.angle_max = servodata_json["angle_max"].as<unsigned int>();  // 180
  servodata.angle_min = servodata_json["angle_min"].as<unsigned int>();  // 10
  servodata.angle_fine = servodata_json["angle_fine"].as<unsigned int>();  // 45

  bucket_number = doc_json["bucket_number"] | -1;

  const char* temp_harvest_date = doc_json["harvest_date"] | "";
  strlcpy(harvest_date, temp_harvest_date, sizeof(harvest_date));

  const char* temp_harvest_number = doc_json["harvest_number"] | "";
  strlcpy(harvest_number, temp_harvest_number, sizeof(harvest_number));

  const char* temp_date = doc_json["date_filling"] | "";
  strlcpy(date_filling, temp_date, sizeof(date_filling));
  weight_filling = doc_json["weight_filling"].as<unsigned int>();    // 500
  weight_fine = doc_json["weight_fine"].as<unsigned int>();          // 400
  glass_empty = doc_json["glass_empty"].as<unsigned int>();          // 222
  glass_tolerance = doc_json["glass_tolerance"].as<unsigned int>();  // 22
  boot_count = doc_json["boot_count"].as<unsigned long>();           // 12000
  // OFFSET can be negative (tare baseline). Use signed long extraction.
  OFFSET = doc_json["OFFSET"].as<long>();  // may be negative
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

  const char* temp_topic = mqtt_server_json["server_topic"] | "bienen/sahfm";
  strlcpy(mqtt_server.server_topic, temp_topic,
          sizeof(mqtt_server.server_topic));

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

  bool corrected = validateAndFix();
  if (upgradeNeeded || corrected ||
      input == espfs.readString("/hmconfig_template.json")) {
    writeJsonConfig();  // persist derived runtime copy; template remains
                        // untouched
  }
}

// HMConfig::HMConfig(void) { log_e("Bad, very bad!!!"); }

bool HMConfig::validateAndFix() {
  bool changed = false;
  // Servo hard limits
  const uint8_t SERVO_ABS_MIN = 0;
  const uint8_t SERVO_ABS_MAX = 180;
  if (servodata.angle_min_hard < SERVO_ABS_MIN) {
    servodata.angle_min_hard = SERVO_ABS_MIN;
    changed = true;
  }
  if (servodata.angle_min_hard > SERVO_ABS_MAX) {
    servodata.angle_min_hard = 0;
    changed = true;
  }
  if (servodata.angle_max_hard > SERVO_ABS_MAX) {
    servodata.angle_max_hard = SERVO_ABS_MAX;
    changed = true;
  }
  if (servodata.angle_max_hard <
      servodata.angle_min_hard + 5) {  // ensure sensible span
    servodata.angle_max_hard = servodata.angle_min_hard + 5;
    changed = true;
  }

  // User servo range clamped into hard range
  if (servodata.angle_min < servodata.angle_min_hard) {
    servodata.angle_min = servodata.angle_min_hard;
    changed = true;
  }
  if (servodata.angle_min > servodata.angle_max_hard) {
    servodata.angle_min = servodata.angle_min_hard;
    changed = true;
  }
  if (servodata.angle_max > servodata.angle_max_hard) {
    servodata.angle_max = servodata.angle_max_hard;
    changed = true;
  }
  if (servodata.angle_max < servodata.angle_min) {
    servodata.angle_max = servodata.angle_min;
    changed = true;
  }
  if (servodata.angle_fine < servodata.angle_min) {
    servodata.angle_fine = servodata.angle_min;
    changed = true;
  }
  if (servodata.angle_fine > servodata.angle_max) {
    servodata.angle_fine = servodata.angle_max;
    changed = true;
  }

  // Weights & tolerance
  const uint16_t WEIGHT_MIN = 50;    // arbitrary safety low bound
  const uint16_t WEIGHT_MAX = 5000;  // upper bound to avoid nonsense
  if (weight_filling < WEIGHT_MIN) {
    weight_filling = WEIGHT_MIN;
    changed = true;
  }
  if (weight_filling > WEIGHT_MAX) {
    weight_filling = WEIGHT_MAX;
    changed = true;
  }
  // weight_fine must be below weight_filling
  if (weight_fine >= weight_filling) {
    uint16_t newFine =
        (weight_filling > 50) ? (weight_filling - 50) : (weight_filling / 2);
    if (newFine != weight_fine) {
      weight_fine = newFine;
      changed = true;
    }
  }
  // glass_empty should not exceed target filling weight (soft rule)
  if (glass_empty > weight_filling && weight_filling > 0) {
    glass_empty =
        (weight_filling > 100) ? (weight_filling / 2) : (weight_filling / 2);
    changed = true;
  }
  // tolerance reasonable range 1..100
  if (glass_tolerance == 0) {
    glass_tolerance = 1;
    changed = true;
  }
  if (glass_tolerance > 100) {
    glass_tolerance = 100;
    changed = true;
  }

  // SCALE sanity: avoid zero which would break division logic elsewhere
  if (SCALE == 0) {
    SCALE = 1;
    changed = true;
  }

  return changed;
}

bool HMConfig::getBatchNumber(char* buffer, size_t bufferSize) const {
  if (!buffer || bufferSize < 11) return false;  // Need at least "DD.MM.YYYY\0"

  // Check if date_filling is set and valid (format: YYYY-MM-DD)
  if (!date_filling || strlen(date_filling) < 10) {
    strlcpy(buffer, "__.__.____", bufferSize);
    return false;
  }

  // Parse date_filling (YYYY-MM-DD)
  struct tm timeinfo = {};
  int year, month, day;
  if (sscanf(date_filling, "%d-%d-%d", &year, &month, &day) != 3) {
    strlcpy(buffer, "__.__.____", bufferSize);
    return false;
  }

  timeinfo.tm_year = year - 1900 + 2;  // Add 2 years
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;
  mktime(&timeinfo);  // Normalize the date

  snprintf(buffer, bufferSize, "%02d.%02d.%04d", timeinfo.tm_mday,
           timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  return true;
}
