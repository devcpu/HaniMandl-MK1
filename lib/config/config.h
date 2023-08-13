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
 * Last Modified: 2023-08-13 04:19
 * Modified By: Johannes G.  Arlt
 */

#ifndef LIB_CONFIG_CONFIG_H_
#define LIB_CONFIG_CONFIG_H_

#define WIFI_PASSWORD ""
#define WIFI_SSID ""
#define WIFI_IP ""
#define WIFI_GATEWAY ""
#define WIFI_TIMESERVER "de.pool.ntp.org"
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define WIFI_TIME_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"
#define WIFI_AP_NAME "HanimandlMKI"
#define WIFI_AP_PASSWORD "Honigkuchen"
#define HONEY_FARM_NAME "BIO-Imkerei Lindenstarsse Berlin"
#define PROGRAMM_NAME "HaniMandl MK I"
#define SOFTWARE_VERSION "v 0.1"
#define MAXSIZE 16

//// Pin definition

#define CONFIG_WIFI_LED 21

#define WEB_INPUT_MAX_LENGTH 31

#endif  // LIB_CONFIG_CONFIG_H_
