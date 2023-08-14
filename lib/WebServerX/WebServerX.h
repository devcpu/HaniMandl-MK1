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
 * Last Modified: 2023-08-13 15:42
 * Modified By: Johannes G.  Arlt
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

static const char kMonthNamesEnglish[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
static const String mainmenue(
    "<form action='.' method='get'><button>Main Menue</button></form><br />");
static const String htmltitle = HONEY_FARM_NAME;
static const String h3title = PROGRAMM_NAME;

#define DEFAULT_RROCESS\
    if (var == "HTMLTILE") {\
        return htmltitle;\
    }\
    if (var == "H3TITLE") {\
        return h3title;\
    }\
    if (var == "ERRORMSG") {\
        return html_error.getErrorMsg();\
    }\
return String("wrong placeholder " + var);



void WebserverStart(void);

String ProcessorJS(const String &var);
String ProcessorDefault(const String &var);
String systemInfoProcessor(const String &var);

String ProcessorFilling(const String &var);

String getSystemInfoTable(void);
String getSystemInfoTable();
String GetBuildDateAndTime();
String table2DGenerator(String data[][2], uint8_t size, boolean bold);
String readSPIFFS2String(const char *path);
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size);

void reboot(AsyncWebServerRequest *request);

#ifdef ESP32
String getResetReason(RESET_REASON);
#endif

#if CORE_DEBUG_LEVEL > 4
void showRequest(AsyncWebServerRequest *request);
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
