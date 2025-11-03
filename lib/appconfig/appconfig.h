/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /config.h
Project: simple automatic honey filling machine
 * Description:
 * -----
 * Created Date: 2023-08-12 23:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2025-11-03 17:39
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_APPCONFIG_APPCONFIG_H_
#define LIB_APPCONFIG_APPCONFIG_H_

#include <Arduino.h>

#define PROGRAMM_NAME "Honey Filling Machine MKI"
#define SOFTWARE_VERSION "v 0.3"

// data if wifi connected
#define WIFI_PASSWORD "HerbertMueller23"
#define WIFI_SSID "Echolon23"
#define WIFI_IP 192, 168, 42, 15
#define WIFI_GATEWAY 192, 168, 42, 101
#define WIFI_SUBNET 255, 255, 255, 0
#define WIFI_DNS1 192, 168, 42, 101
#define WIFI_DNS2 8, 8, 8, 8
#define WIFI_TIMESERVER "de.pool.ntp.org"

/// data if accesspoint
#define WIFI_AP_NAME "Honey Filling Machine MKI"
#define WIFI_AP_PASSWORD "Honigkuchen"
#define HONEY_FARM_NAME "BIO-Imkerei St. Johannis"

// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define WIFI_TIME_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// pin definition
#define PIN_LOADCELL_DOUT 5
#define PIN_LOADCELL_SCK 17
#define PIN_SERVO 33
#define PIN_WIFI_LED 21
#define PIN_BUZZER 16

// Emergency stop button / touch pad
#define PIN_STOP_BTN 27  // GPIO27 also touch channel T7
#define PIN_STOP_TOUCH_CHANNEL T7
#define STOP_DEBOUNCE_MS 15  // mechanical: 30–50ms; touch: 5–15ms

// calc avg from how many times read
#define LOADCELL_READ_TIMES 3

// servo data
#define SERVO_FREQUENCY 300
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500

#define WEB_INPUT_MAX_LENGTH 32

#endif  // LIB_APPCONFIG_APPCONFIG_H_
