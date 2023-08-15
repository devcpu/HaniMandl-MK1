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
 * Last Modified: 2023-08-15 23:33
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
    void APRSWebServerTick(void);

    String DefaultPlaceholderProcessor(const String &var);
    String DefaultProcessor(const String &var);
    String SystemInfoProcessor(const String &var);
    String ProcessorFilling(const String &var);
    String ProcessorSetupFilling(const String &var);
    String ProcessorSetup(const String &var);
    String ProcessorSetupWlan(const String &var);
    String ProcessorUpdateFirmware(const String &var);
    String ProcessorCalibrate(const String &var);

    String readSPIFFS2String(const String &path);
    String getSystemInfoTable();
    String getBuildDateAndTime();
    String table2DGenerator(SystemData systemdata[], uint8_t size, boolean bold);
    String optionsFieldGenerator(String selected, const char *name, String data[][2], uint8_t size);

    void resetHTMLError(void);
    void HTMLSendError(String msg, AsyncWebServerRequest *request);

                                    
    void disconnect();
    void restartESP();
    void reboot(AsyncWebServerRequest *request);

    KeyValue split(String wsdata);
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



#ifdef ESP32
        String getResetReason(RESET_REASON);
#endif
#if CORE_DEBUG_LEVEL > 4
        int showRequest(AsyncWebServerRequest *request);
#endif

#endif  // LIB_WEBSERVERX_WEBSERVERX_H_
