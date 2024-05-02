/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /webserverx.h
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/WebServerX
 * Description:
 * -----
 * Created Date: 2023-08-12 17:43
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2024-04-30 13:02
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_WEBSERVERX_WEBSERVERX_H_
#define LIB_WEBSERVERX_WEBSERVERX_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <Glass.h>
#include <HMConfig.h>
#include <SPIFFS.h>
#include <WebTemplate.h>
#include <appconfig.h>

#ifdef ESP32
#include <rom/rtc.h>
#endif

struct KeyValueArray {
  struct KeyValue {
    String key;
    String value;
  };
  KeyValue keyValue[3];
  int count;
};

void WebserverStart(void);

String getSystemInfoTable();
String getBuildDateAndTime();

KeyValueArray split(KeyValueArray rval, String wsdata);
bool isNumber(String val);
// boolean validateNumber(String test);
// boolean isNumeric(String str);

String getWebParam(AsyncWebServerRequest *request, const char *key,
                   String *prefsvar);
String getWebParam(AsyncWebServerRequest *request, const char *key,
                   double *prefsvar);
String getWebParam(AsyncWebServerRequest *request, const char *key);

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);

void sendSocketData();

#ifdef ESP32
String getResetReason(RESET_REASON);
#endif
#if CORE_DEBUG_LEVEL > 4
int showRequest(AsyncWebServerRequest *request);
#endif

#endif  // LIB_WEBSERVERX_WEBSERVERX_H_
