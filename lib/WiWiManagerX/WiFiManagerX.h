/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /WiFiManagerX.h
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/WiWiManagerX
 * Description:
 * -----
 * Created Date: 2023-08-12 15:55
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-12 18:19
 * Modified By: Johannes G.  Arlt
 */

#ifndef LIB_WIWIMANAGERX_WIFIMANAGERX_H_
#define LIB_WIWIMANAGERX_WIFIMANAGERX_H_

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
// https://github.com/tzapu/WiFiManager
#include <ESPAsyncWiFiManager.h>
#include <Ticker.h>

#include "esp_log.h"

void tick(void);
void configModeCallback(AsyncWiFiManager *myWiFiManager);
bool setupWifiManager(void);

#endif  // LIB_WIWIMANAGERX_WIFIMANAGERX_H_
