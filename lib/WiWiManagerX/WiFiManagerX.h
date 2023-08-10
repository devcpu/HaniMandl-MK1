#ifndef LIB_WIWIMANAGERX_H_
#define LIB_WIWIMANAGERX_H_

#include <Arduino.h>
#include "esp_log.h"
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Ticker.h>

void tick(void);
void configModeCallback (AsyncWiFiManager *myWiFiManager);
bool setupWifiManager(void);

#endif // LIB_WIWIMANAGERX_H_
