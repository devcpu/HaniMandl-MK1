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
 * Last Modified: 2023-08-15 03:01
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_WEBSERVERX_WEBSERVERX_H_
#define LIB_WEBSERVERX_WEBSERVERX_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HMConfig.h>
#include <SPIFFS.h>
#include <config.h>
#ifdef ESP32
#include <rom/rtc.h>
#endif

extern HMConfig cfg;

static const String mainmenue(
    "<form action='.' method='get'><button>Main Menue</button></form><br />");
static const String htmltitle = HONEY_FARM_NAME;
static const String h3title = PROGRAMM_NAME;

struct SystemData {
  const char *label;
  String value;
};

struct KeyValue {
  String key;
  String value;
};

void WebserverStart(void);

String ProcessorJS(const String &var);
String defaultProcessor(const String &var);
String systemInfoProcessor(const String &var);

String ProcessorFilling(const String &var);

String ProcessorSetupFilling(const String &var);
String ProcessorSetup(const String &var);
String ProcessorSetupWlan(const String &var);
String ProcessorUpdateFirmware(const String &var);

String getSystemInfoTable(void);
String getSystemInfoTable();
String GetBuildDateAndTime();
String table2DGenerator(SystemData systemdata[], uint8_t size, boolean bold);
String readSPIFFS2String(const String &path);
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size);
KeyValue split(String wsdata);
void disconnect();
void restartESP();
void reboot(AsyncWebServerRequest *request);
String ProcessorCalibrate(const String &var);

#ifdef ESP32
String getResetReason(RESET_REASON);
#endif
bool isNumber(String val);
#if CORE_DEBUG_LEVEL > 4
int showRequest(AsyncWebServerRequest *request);
#endif

// boolean validateNumber(String test);
// boolean isNumeric(String str);

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);
void APRSWebServerTick(void);

void resetHTMLError(void);
void HTMLSendError(String msg, AsyncWebServerRequest *request);

String getWebParam(AsyncWebServerRequest *request, const char *key,
                   String *prefsvar);
String getWebParam(AsyncWebServerRequest *request, const char *key,
                   double *prefsvar);
String getWebParam(AsyncWebServerRequest *request, const char *key);

#endif  // LIB_WEBSERVERX_WEBSERVERX_H_
