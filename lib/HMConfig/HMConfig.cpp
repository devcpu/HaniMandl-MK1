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
 * Last Modified: 2023-09-02 02:16
 * Modified By: Johannes G.  Arlt (janusz)
 */

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

void writeJsonConfig() {
  String output;
  StaticJsonDocument<1024> doc;

  JsonObject servodata = doc.createNestedObject("servodata");
  servodata["angle_max_hard"] = 180;
  servodata["angle_min_hard"] = 0;
  servodata["angle_max"] = 180;
  servodata["angle_min"] = 10;
  servodata["angle_fine"] = 45;
  doc["los_number"] = "das_ist_die_losnummer";
  doc["date_filling"] = "2023-09-01";
  doc["weight_filling"] = 500;
  doc["weight_fine"] = 400;
  doc["glass_empty"] = 222;
  doc["glass_tolerance"] = 22;
  doc["glass_count"] = 2000;
  doc["boot_count"] = 12000;
  doc["OFFSET"] = 456789;
  doc["SCALE"] = 346.02359;

  JsonObject mqtt_server = doc.createNestedObject("mqtt_server");
  mqtt_server["server_user"] = "e7555e77-0d62-4ab9-a934-9352d9bdf0ce";
  mqtt_server["server_passwd"] = "e7555e77-0d62-4ab9-a934-9352d9bdf0ce";
  mqtt_server["server_ip"] = "100.100.100.100";
  mqtt_server["server_port"] = "65000";
  mqtt_server["server_token"] = "e7555e77-0d62-4ab9-a934-9352d9bdf0ce";

  JsonObject api_server = doc.createNestedObject("api_server");
  api_server["server_user"] = "e7555e77-0d62-4ab9-a934-9352d9bdf0ce";
  api_server["server_passwd"] = "e7555e77-0d62-4ab9-a934-9352d9bdf0ce";
  api_server["server_ip"] = "100.100.100.100";
  api_server["server_port"] = "65000";
  api_server["server_token"] = "e7555e77-0d62-4ab9-a934-9352d9bdf0ce";

  serializeJson(doc, output);
}

void readJsonConfig() {
  // String input;

  StaticJsonDocument<1536> doc;

  String input;

  DeserializationError error = deserializeJson(doc, input);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject servodata = doc["servodata"];
  int servodata_angle_max_hard = servodata["angle_max_hard"];  // 180
  int servodata_angle_min_hard = servodata["angle_min_hard"];  // 0
  int servodata_angle_max = servodata["angle_max"];            // 180
  int servodata_angle_min = servodata["angle_min"];            // 10
  int servodata_angle_fine = servodata["angle_fine"];          // 45

  const char* los_number = doc["los_number"];      // "das_ist_die_losnummer"
  const char* date_filling = doc["date_filling"];  // "2023-09-01"
  int weight_filling = doc["weight_filling"];      // 500
  int weight_fine = doc["weight_fine"];            // 400
  int glass_empty = doc["glass_empty"];            // 222
  int glass_tolerance = doc["glass_tolerance"];    // 22
  int glass_count = doc["glass_count"];            // 2000
  int boot_count = doc["boot_count"];              // 12000
  long OFFSET = doc["OFFSET"];                     // 456789
  double SCALE = doc["SCALE"];                     // 346.02359

  JsonObject mqtt_server = doc["mqtt_server"];
  const char* mqtt_server_server_user = mqtt_server["server_user"];
  const char* mqtt_server_server_passwd = mqtt_server["server_passwd"];
  const char* mqtt_server_server_ip =
      mqtt_server["server_ip"];  // "100.100.100.100"
  const char* mqtt_server_server_port = mqtt_server["server_port"];  // "65000"
  const char* mqtt_server_server_token = mqtt_server["server_token"];

  JsonObject api_server = doc["api_server"];
  const char* api_server_server_user = api_server["server_user"];
  const char* api_server_server_passwd = api_server["server_passwd"];
  const char* api_server_server_ip =
      api_server["server_ip"];  // "100.100.100.100"
  const char* api_server_server_port = api_server["server_port"];  // "65000"
  const char* api_server_server_token = api_server["server_token"];
}

// HMConfig::HMConfig(void) { log_e("Bad, very bad!!!"); }
