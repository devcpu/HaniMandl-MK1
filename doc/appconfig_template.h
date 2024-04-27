/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /config.h
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib
 * Description:
 * -----
 * Created Date: 2023-08-12 23:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2024-04-26 16:46
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_APPCONFIG_APPCONFIG_H_
#define LIB_APPCONFIG_APPCONFIG_H_

#include <Arduino.h>

#define PROGRAMM_NAME "Honey Filling Machine MKI"
#define SOFTWARE_VERSION "v 0.1"


// data if wifi connected
#define WIFI_PASSWORD ""
#define WIFI_SSID ""
#define WIFI_IP ""
#define WIFI_GATEWAY ""
#define WIFI_TIMESERVER "de.pool.ntp.org"

/// data if accesspoint
#define WIFI_AP_NAME = "Honey Filling Machine MKI";
#define WIFI_AP_PASSWORD = "Honigkuchen";
#define HONEY_FARM_NAME = "";


// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define WIFI_TIME_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// pin definition
#define PIN_LOADCELL_DOUT 5;
#define PIN_LOADCELL_SCK 17;
#define PIN_SERVO 33;
#define PIN_WIFI_LED 21;

// calc avg from how many times read
#define LOADCELL_READ_TIMES = 3;

// servo data
#define SERVO_FREQUENCY 300;
#define SERVO_MIN_PULSE 500;
#define SERVO_MAX_PULSE 2500;

#define WEB_INPUT_MAX_LENGTH 31

#endif  // LIB_APPCONFIG_APPCONFIG_H_
