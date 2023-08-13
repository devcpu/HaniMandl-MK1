/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /webserverx.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/WebServerX
 * Description:
 * -----
 * Created Date: 2023-08-12 16:28
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-13 04:19
 * Modified By: Johannes G.  Arlt
 */

#include <WebServerX.h>

struct HTML_Error {
  String ErrorMsg;
  boolean isSended;

 public:
  void setErrorMsg(String msg) {
    if (ErrorMsg == "") {
      ErrorMsg = msg;
    } else {
      ErrorMsg += msg;
    }
    isSended = false;
  }
  String getErrorMsg(void) {
    isSended = true;
    return ErrorMsg;
  }
};

HTML_Error html_error;

extern HMConfig cfg;

// extern QueueHandle_t LoRaTXQueue;

// #include "CallBackList.h"

// @TODO remove together with restart()

extern HMConfig cfg;

AsyncWebServer *WebServer;
AsyncWebSocket *ws;
AsyncWebSocketClient *globalClient = NULL;

void WebserverStart(void) {
  log_e("starting Webserver");
  WebServer = new AsyncWebServer(80);
  ws = new AsyncWebSocket("/ws");

  ws->onEvent(onWsEvent);
  WebServer->addHandler(ws);

  WebServer->onNotFound([](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(404);
  });

  WebServer->serveStatic("/hanimandlmk1.css", SPIFFS, "/hanimandlmk1.css");
  WebServer->serveStatic("/rebootinfo", SPIFFS, "/reboot.html");

  WebServer->on("/hanimandlmk1.js", HTTP_GET,
                [](AsyncWebServerRequest *request) {
                  log_e("/hanimandlmk1.js");
                  request->send(SPIFFS, "/hanimandlmk1.js",
                                "application/javascript", false, ProcessorJS);
                });

  /*
  .########...#######..##.....##.########.########.##....##
  .##.....##.##.....##.##.....##....##....##.......###...##
  .##.....##.##.....##.##.....##....##....##.......####..##
  .########..##.....##.##.....##....##....######...##.##.##
  .##...##...##.....##.##.....##....##....##.......##..####
  .##....##..##.....##.##.....##....##....##.......##...###
  .##.....##..#######...#######.....##....########.##....##
  */

  WebServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_d("/");
    log_d("wie weiter?");
    // first run wizard
    if (cfg.beekeeping == "") {  // first run wizard
      log_d("redirect setup");
      request->redirect("/setup");
    }
    log_d("master.html");
    request->send(SPIFFS, "/master.html", "text/html", false, ProcessorDefault);
  });

  // System Info
  WebServer->on("/systeminfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/systeminfo");
    request->send(SPIFFS, "/master.html", "text/html", false,
                  systemInfoProcessor);
  });

  // // Configure Call
  // WebServer->on("/cc", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/cc");
  //   showRequest(request);
  //   if (request->params() > 0) {
  //     log_e("/cc");
  //     handleRequestConfigCall(request);
  //     if (cfg.APCredentials.auth_tocken == "letmein42") {  // first run
  //     wizard
  //       request->redirect("/ca");
  //     }
  //     request->redirect("/");
  //   } else {
  //     request->send(SPIFFS, "/master.html", "text/html", false,
  //                   ProcessorConfigCall);
  //   }
  // });

  // // Send Message
  // WebServer->on("/sm", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/sm");
  //   showRequest(request);
  //   if (request->params() > 0) {
  //     handleRequestSendMessage(request);
  //     request->redirect("/");
  //   }

  //   request->send(SPIFFS, "/master.html", "text/html", false,
  //                 ProcessorSendMessage);
  // });

  // // Change Mode
  // WebServer->on("/cm", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   bool changed = false;
  //   log_e("/cm");
  //   showRequest(request);
  //   log_e("Change Mode");
  //   if (request->params() == 2) {
  //     log_e("got 2 params");
  //     changed = handleRequestChangeMode(request);
  //     log_e("new run_mode: %d / new_wifi_mode %d",
  //           static_cast<uint8_t>(cfg.current_run_mode),
  //           static_cast<uint8_t>(cfg.current_wifi_mode));
  //     // @FIXME cast error? see debug console
  //     log_e("new run_mode: %d / new_wifi_mode %d\n",
  //           static_cast<uint8_t>(getPrefsDouble(PREFS_CURRENT_SYSTEM_MODE)),
  //           static_cast<uint8_t>(getPrefsDouble(PREFS_CURRENT_WIFI_MODE)));
  //     request->redirect("/");
  //   }
  //   if (changed) {
  //   } else {
  //     request->send(SPIFFS, "/master.html", "text/html", false,
  //                   ProcessorChangeMode);
  //   }
  // });

  // // GPS Info
  // WebServer->on("/gi", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/gi");
  //   showRequest(request);
  //   request->send(SPIFFS, "/master.html", "text/html", false,
  //   ProcessorGPSInfo);
  // });

  // // WX Info
  // WebServer->on("/wx", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/wx");
  //   showRequest(request);
  //   request->send(SPIFFS, "/master.html", "text/html", false,
  //   ProcessorWXInfo);
  // });

  // // Wifi AP
  // WebServer->on("/ca", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/ca");
  //   showRequest(request);
  //   if (request->params()) {
  //     if (request->params() == 2) {
  //       handleRequestConfigAP(request);
  //       request->redirect("/");
  //     } else {
  //       log_e("ERR: wrong request");
  //     }
  //   }

  //   request->send(SPIFFS, "/master.html", "text/html", false,
  //                 ProcessorConfigWifiAP);
  // });

  // // Config Gateway
  // WebServer->on("/cg", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/cg");
  //   showRequest(request);
  //   if (request->params()) {
  //     handleRequestConfigGateway(request);
  //     request->redirect("/");
  //   }
  //   request->send(SPIFFS, "/master.html", "text/html", false,
  //                 ProcessorConfigGateway);
  // });

  // reboot
  WebServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/reboot");
    reboot(request);
  });

  // // ConfigWLAN
  // WebServer->on("/cl", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/cl");
  //   showRequest(request);
  //   if (request->params()) {
  //     if (request->params() == 2) {
  //       handleRequestConfigWLAN(request);
  //       request->redirect("/");
  //     } else {
  //       log_e("ERR: wrong request");
  //     }
  //   }
  //   request->send(SPIFFS, "/master.html", "text/html", false,
  //                 ProcessorConfigWLAN);
  // });

  // Config Web Admin
  // WebServer->on("/cw", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/cw");
  //   showRequest(request);
  //   // handle Request in /ca
  // });

  // Config Web Admin
  // WebServer->on("/APRSSymbol", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   log_e("/APRSSymbol");
  //   showRequest(request);
  //   request->send(SPIFFS, "/APRS_Symbol_Chart.pdf", "application/pdf",
  //   false);
  // });

  WebServer->on("/bb", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/bb");
    showRequest(request);
    // no Processor !
  });

  WebServer->begin();
  log_e("HTTP WebServer started");
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    log_e("Websocket client connection received");
    globalClient = client;
  } else if (type == WS_EVT_DISCONNECT) {
    log_e("Websocket client connection finished");
    globalClient = NULL;
  }
}

/*
.########..########...#######...######..########..######...######..########.########...######.
.##.....##.##.....##.##.....##.##....##.##.......##....##.##....##.##.......##.....##.##....##
.##.....##.##.....##.##.....##.##.......##.......##.......##.......##.......##.....##.##......
.########..########..##.....##.##.......######....######...######..######...########...######.
.##........##...##...##.....##.##.......##.............##.......##.##.......##...##.........##
.##........##....##..##.....##.##....##.##.......##....##.##....##.##.......##....##..##....##
.##........##.....##..#######...######..########..######...######..########.##.....##..######.
*/

String systemInfoProcessor(const String &var) {
  if (var == "HTMLTILE") {
    return htmltitle;
  }

  if (var == "H3TITLE") {
    return h3title;
  }

  if (var == "H2TITLE") {
    return String("System Info");
  }

  if (var == "ERRORMSG") {
    return html_error.getErrorMsg();
  }

  if (var == "BODY") {
    return getSystemInfoTable();
  }

  return String("wrong placeholder " + var);
}

String ProcessorDefault(const String &var) {
  if (var == "HTMLTILE") {
    return htmltitle;
  }

  if (var == "H3TITLE") {
    return h3title;
  }

  if (var == "H2TITLE") {
    return String("Main Menue");
  }

  if (var == "ERRORMSG") {
    return html_error.getErrorMsg();
  }

  if (var == "BODY") {
    return readSPIFFS2String("/mainbutton.html");
  }
  return String("wrong placeholder " + var);
}

String getChipId() {
  char ssid1[MAXSIZE + 1];
  char ssid2[MAXSIZE + 1];

  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid1, sizeof(ssid1), "%04X", chip);
  snprintf(ssid2, sizeof(ssid1), "%08X", (uint32_t)chipid);
  return String(ssid1) + String(ssid2);
}

/// @brief Generate data for Systeminfo
/// @return HTML table with system info
String getSystemInfoTable(void) {
  FlashMode_t ideMode = ESP.getFlashChipMode();

  String systemdata[][2] = {
      {"SoftwareVersion", cfg.version},
      {"Build DateTime: ", GetBuildDateAndTime()},
      {"SDKVersion: ", String(ESP.getSdkVersion())},
      //      {"BootCount: ", String(cfg.boot_count)},
      {"Uptime: ", String(millis() / 1000 / 60, DEC) + "min"},
#ifdef ESP32
      {"Chip Revision:", String(ESP.getChipRevision())},
      {"ESP32 Chip ID:",
       // cppcheck-suppress shiftTooManyBits
       // String((uint16_t)chipid >> 32, HEX) + String((uint32_t)chipid, HEX)},
       getChipId()},
      {"Reset Reason CPU0: ", getResetReason(rtc_get_reset_reason(0))},
      {"Reset Reason CPU1: ", getResetReason(rtc_get_reset_reason(1))},
      {"CpuFreqMHz: ", String(ESP.getCpuFreqMHz()) + "MHz"},
      {"CycleCount: ", String(ESP.getCycleCount())},
      {"FlashChipMode: ", String(ESP.getFlashChipMode())},
      {"FlashChipSize: ", String(ESP.getFlashChipSize() / 1024 / 1024) + "MB"},
      {"FlashChipSpeed: ",
       String(ESP.getFlashChipSpeed() / 1024 / 1024) + "MHz"},
      {"SketchSize: ", String(ESP.getSketchSize() / 1024) + "kB"},
      {"FreeSketchSpace: ", String(ESP.getFreeSketchSpace() / 1024) + "kB"},
      {"SketchMD5: ", String(ESP.getSketchMD5())},
      {"HeapSize: ", String(ESP.getHeapSize() / 1024) + "kB"},
      {"FreeHeap: ", String(ESP.getFreeHeap() / 1024) + "kB"},
      {"MaxAllocHeap: ", String(ESP.getMaxAllocHeap() / 1024) + "kB"},
      {"MinFreeHeap: ", String(ESP.getMinFreeHeap() / 1024) + "kB"},

  // {"PsramSize: ", String(ESP.getPsramSize() / 1024) + "kB"},
  // {"FreePsram", String(ESP.getFreePsram() / 1024) + "kB"},
  // {"MaxAllocPsram: ", String(ESP.getMaxAllocPsram() / 1024) + "kB"},
  // {"MinFreePsram", String(ESP.getMinFreePsram() / 1024) + "kB"},

#elif defined(ESP8266)
      {"Flash real id:", String(ESP.getFlashChipId(), HEX)},
      {"Flash real size:", String(ESP.getFlashChipRealSize() / 1024) + "kB"},
#endif
      // size = 5
      {"Flash ide  size:", String(ESP.getFlashChipSize() / 1024) + "kB"},
      {"Flash ide speed:",
       String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"},
      {"Flash ide mode:", String((ideMode == FM_QIO    ? "QIO"
                                  : ideMode == FM_QOUT ? "QOUT"
                                  : ideMode == FM_DIO  ? "DIO"
                                  : ideMode == FM_DOUT ? "DOUT"
                                                       : "UNKNOWN"))},
      {"Sketch size:", String(ESP.getSketchSize() / 1024) + "kB"},
      // size = 10
      {"Free sketch size:", String(ESP.getFreeSketchSpace() / 1024) + "kB"},
      {"Free heap:", String(ESP.getFreeHeap() / 1024) + "kB"},
#ifdef ESP32
#elif defined(ESP8266)
      {"ResetReason", String(ESP.getResetReason())},
//      {"Chip Config Status:", String()},
#endif
      //
      // {"", String()},
  };
  // second param hast to fit line numbers
  return table2DGenerator(systemdata, 26, true) + mainmenue;
}

/// @brief Populate myIP to client for websocket
/// @param var
/// @return myIP
String ProcessorJS(const String &var) {
  if (var == "SERVER_IP") {
    log_e("%s", cfg.myIP);
    return cfg.myIP.toString();
  }

  return String("wrong placeholder " + var);
}

/// @brief Get the Build Date And Time for device info
/// @return String "2017-03-07T11:08:02"
String GetBuildDateAndTime(void) {
  // "2017-03-07T11:08:02" - ISO8601:2004
  char bdt[60];  // Flawfinder: ignore TODO fix 60
  char *p;
  char mdate[] = __DATE__;  // "Mar  7 2017"
  char *smonth = mdate;
  int day = 0;
  int year = 0;

  uint8_t i = 0;
  for (char *str = strtok_r(mdate, " ", &p); str && i < 3;
       str = strtok_r(nullptr, " ", &p)) {
    switch (i++) {
      case 0:  // Month
        smonth = str;
        break;
      case 1:             // Day
        day = atoi(str);  // Flawfinder: ignore
        break;
      case 2:              // Year
        year = atoi(str);  // Flawfinder: ignore
    }
  }

  int month = (strstr(kMonthNamesEnglish, smonth) - kMonthNamesEnglish) / 3 + 1;
  snprintf_P(bdt, sizeof(bdt), "%04d-%02d-%02d %s", year, month, day, __TIME__);
  return String(bdt);  // 2017-03-07T11:08:02
}

#ifdef ESP32
/// @brief Convert int RESET_REASON in more human readable string
/// @param reason
/// @return more human readable reset reason
String getResetReason(RESET_REASON reason) {
  String retvar;
  switch (reason) {
    case 1:
      retvar = "POWERON_RESET";
      break; /**<1, Vbat power on reset*/
    case 3:
      retvar = "SW_RESET";
      break; /**<3, Software reset digital core*/
    case 4:
      retvar = "OWDT_RESET";
      break; /**<4, Legacy watch dog reset digital core*/
    case 5:
      retvar = "DEEPSLEEP_RESET";
      break; /**<5, Deep Sleep reset digital core*/
    case 6:
      retvar = "SDIO_RESET";
      break; /**<6, Reset by SLC module, reset digital core*/
    case 7:
      retvar = "TG0WDT_SYS_RESET";
      break; /**<7, Timer Group0 Watch dog reset digital core*/
    case 8:
      retvar = "TG1WDT_SYS_RESET";
      break; /**<8, Timer Group1 Watch dog reset digital core*/
    case 9:
      retvar = "RTCWDT_SYS_RESET";
      break; /**<9, RTC Watch dog Reset digital core*/
    case 10:
      retvar = "INTRUSION_RESET";
      break; /**<10, Instrusion tested to reset CPU*/
    case 11:
      retvar = "TGWDT_CPU_RESET";
      break; /**<11, Time Group reset CPU*/
    case 12:
      retvar = "SW_CPU_RESET";
      break; /**<12, Software reset CPU*/
    case 13:
      retvar = "RTCWDT_CPU_RESET";
      break; /**<13, RTC Watch dog Reset CPU*/
    case 14:
      retvar = "EXT_CPU_RESET";
      break; /**<14, for APP CPU, reseted by PRO CPU*/
    case 15:
      retvar = "RTCWDT_BROWN_OUT_RESET";
      break; /**<15, Reset when the vdd voltage is not stable*/
    case 16:
      retvar = "RTCWDT_RTC_RESET";
      break; /**<16, RTC Watch dog reset digital core and rtc module*/
    default:
      retvar = "NO_MEAN";
  }
  return retvar;
}
#endif

/**
 * table2DGenerator.
 *
 * @author	JA
 * @param	String 	data[][2]
 * @param	uint8_t	size
 * @param	boolean	bold
 * @return	mixed
 */
String table2DGenerator(String data[][2], uint8_t size, boolean bold) {
  String tdstart("<tr><td>");
  String tdmittle("</td><td>");
  if (bold) {
    tdstart = "<tr><td><b>";
    tdmittle = "</b></td><td>";
  }
  String tdend("</td></tr>");
  String retvar("<table>");

  for (uint8_t z = 0; z < size; z++) {
    retvar += tdstart + data[z][0] + tdmittle + data[z][1] + tdend;
  }
  retvar += "</table>";
  return retvar;
}

String readSPIFFS2String(const char *path) {
  char buf[64] = {0};  // Flawfinder: ignore
  if (!SPIFFS.exists(path)) {
    snprintf(buf, sizeof(buf), "ERROR, %s do not exists.", path);
    log_e("%s", buf);
    return String(buf);
  }
  File f = SPIFFS.open(path, "r");  // Flawfinder: ignore
  String retvar;
  while (f.available()) {
    retvar += static_cast<char>(f.read());  // Flawfinder: ignore
  }
  return retvar;
}

/**
 * @brief generates HTML options field from given parameter
 *
 * @param selected pre selected field
 * @param name field name (HTML)
 * @param data array of HTML display Strings (keys) -> values
 * @param size count elements
 * @return String
 */
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size) {
  log_d("%s", name);
  log_d("%s", selected);
  char buf[1200] = {0};     // Flawfinder: ignore
  char zbuf[1200] = {0};    // Flawfinder: ignore
  char selectxt[32] = {0};  // Flawfinder: ignore
  snprintf(zbuf, sizeof(zbuf), "\n\n<select name='%s'>\n", name);
  strncat(buf, zbuf, sizeof(buf) - 1);  // Flawfinder: ignore
  for (uint8_t i = 0; i < size; i++) {
    if (selected.compareTo(data[i][1]) == 0) {
      strncpy(selectxt, " selected ",               // Flawfinder: ignore
              sizeof(selectxt) - 1);                // Flawfinder: ignore
    } else {                                        // Flawfinder: ignore
      strncpy(selectxt, "", sizeof(selectxt) - 1);  // Flawfinder: ignore
    }
    snprintf(zbuf, sizeof(zbuf), "<option value=\"%s\"%s>%s</option>\n",
             data[i][1].c_str(), selectxt, data[i][0].c_str());
    strncat(buf, zbuf, sizeof(buf) - 1);  // Flawfinder: ignore
  }                                       // END for

  strncat(buf, "</select>\n\n", sizeof(buf) - 1);  // Flawfinder: ignore
  log_e("%s", name);

  return String(buf);
}

/// @brief reboots the ESP32
/// @param request
void reboot(AsyncWebServerRequest *request) {
  request->redirect("/rebootinfo");
  delay(3000);
  // TODO(JA) disconnect
  ESP.restart();
}

// void APRSWebServerTick(void) {
//   if (globalClient != NULL && globalClient->status() == WS_CONNECTED) {
//     //      String randomNumber = String(random(0,20));
//     //      globalClient->text(randomNumber);
//     static uint32_t last = 0;
//     uint32_t i = millis();
//     if (i > last + 1000) {
//       last = i;
//       // globalClient->text("Uptime: " + String(last / 1000) + " Sekunden");
//       sendGPSDataJson();
//     }
//   }
// }

// void sendGPSDataJson(void) {
//   // StaticJsonDocument<10000> doc;

//   // AsyncJsonResponse * response = new AsyncJsonResponse();
//   // JsonVariant& root = response->getRoot();
//   globalClient->server()->cleanupClients();
//   char tmpbuf[32] = {0};  // Flawfinder: ignore
//   StaticJsonDocument<1024> root;
//   root["isValidTime"] = gps.time.isValid();
//   root["isValidGPS"] = gps.date.isValid();

//   snprintf(tmpbuf, sizeof(tmpbuf), "%02d:%02d:%02d", cfg.gps_time.hour,
//            cfg.gps_time.minute, cfg.gps_time.second);
//   root["time"] = tmpbuf;
//   // log_e("%s", tmpbuf);

//   snprintf(tmpbuf, sizeof(tmpbuf), "%4d-%02d-%02d", cfg.gps_time.year,
//            cfg.gps_time.month, cfg.gps_time.day);
//   root["date"] = tmpbuf;
//   // log_e("%s", tmpbuf);

//   root["lat"] = cfg.gps_location.latitude;
//   root["lng"] = cfg.gps_location.longitude;
//   root["alt"] = cfg.gps_location.altitude;
//   root["course"] = cfg.gps_move.course;
//   root["speed"] = cfg.gps_move.speed;
//   // have to show by more static
//   root["temp"] = cfg.WXdata.temp;
//   root["humidity"] = cfg.WXdata.humidity;
//   root["pressure"] = cfg.WXdata.pressure;
//   root["sensor"] = "BME280";
//   root["sat"] = cfg.gps_meta.sat;
//   root["hdop"] = cfg.gps_meta.hdop;
//   uint16_t len = measureJson(root);
//   // log_e("%d", len); // @FIXME remove
//   // serializeJson(root, Serial);

//   AsyncWebSocketMessageBuffer *buffer = globalClient->server()->makeBuffer(
//       len);  //  creates a buffer (len + 1) for you.
//   if (buffer) {
//     serializeJson(root, reinterpret_cast<char *>(buffer->get()), len + 1);
//     if (!globalClient->queueIsFull() &&
//         globalClient->status() == WS_CONNECTED) {  // paranoia?
//       globalClient->server()->textAll(buffer);
//     } else {
//       ESP_LOGE(TAG, "can't send to websocket");
//     }
//   }

//   // serializeJsonPretty(root, Serial);
// }

/*
.##.....##.########.##.......########..########.########...######.
.##.....##.##.......##.......##.....##.##.......##.....##.##....##
.##.....##.##.......##.......##.....##.##.......##.....##.##......
.#########.######...##.......########..######...########...######.
.##.....##.##.......##.......##........##.......##...##.........##
.##.....##.##.......##.......##........##.......##....##..##....##
.##.....##.########.########.##........########.##.....##..######.
*/

String getWebParam(AsyncWebServerRequest *request, const char *key,
                   String *cfgvar) {
  String rtvar = "";
  if (request->hasParam(key)) {
    rtvar = request->getParam(key)->value();
    if (rtvar.length() > 0 && rtvar.length() < WEB_INPUT_MAX_LENGTH) {
      *cfgvar = rtvar;
      log_e("set new var to cfg key=%s value=%s\n", key, rtvar.c_str());
    }
    return rtvar;
  } else {
    char buf[80];
    snprintf(buf, sizeof(buf),
             "ERR> key %s not found in request,  no value written", key);
    log_e("%s", buf);
    return String("");
  }
}

String getWebParam(AsyncWebServerRequest *request, const char *key,
                   uint8_t *cfgvar) {
  String rtvar = "";
  if (request->hasParam(key)) {
    rtvar = request->getParam(key)->value();
    rtvar.replace(',', '.');
    *cfgvar = rtvar.toInt();
    return rtvar;
  } else {
    char buf[80];
    snprintf(buf, sizeof(buf), "key %s not found in request, no value written",
             key);
    log_e("%s", buf);
    return String("");
  }
}

String getWebParam(AsyncWebServerRequest *request, const char *key) {
  String rtvar = "";
  if (request->hasParam(key)) {
    rtvar = request->getParam(key)->value();
    if (rtvar.length() > 0 && rtvar.length() < WEB_INPUT_MAX_LENGTH) {
      return rtvar;
    }
  } else {
    char buf[80];
    snprintf(buf, sizeof(buf), "key %s not found in request, no value written",
             key);
    log_e("%s", buf);
    return String("");
  }
  return rtvar;
}

#if CORE_DEBUG_LEVEL > 4
/// @brief for debugging prints request info to serial console
/// @param request
void showRequest(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_GET)
    log_e("GET");
  else if (request->method() == HTTP_POST)
    log_e("POST");
  else if (request->method() == HTTP_DELETE)
    log_e("DELETE");
  else if (request->method() == HTTP_PUT)
    log_e("PUT");
  else if (request->method() == HTTP_PATCH)
    log_e("PATCH");
  else if (request->method() == HTTP_HEAD)
    log_e("HEAD");
  else if (request->method() == HTTP_OPTIONS)
    log_e("OPTIONS");
  else
    log_e("UNKNOWN");
  log_e(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if (request->contentLength()) {
    log_e("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    log_e("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int i;
  // int headers = request->headers();
  // for (i = 0; i < headers; i++) {
  //   AsyncWebHeader *h = request->getHeader(i);
  //   log_e("_HEADER[%s]: %s\n", h->name().c_str(),
  //   h->value().c_str());
  //  }
  int params = request->params();
  for (i = 0; i < params; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile()) {
      log_e("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(),
            p->size());
    } else if (p->isPost()) {
      log_e("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      log_e("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
}
#else
void showRequest(AsyncWebServerRequest *request) {}
#endif

// void onRequest(AsyncWebServerRequest *request) {
//   log_e("onRequest");
//   request->send(404);
// }

// void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
//             size_t index, size_t total) {
//   log_e("onBody");
// }

// void onUpload(AsyncWebServerRequest *request, String filename, size_t index,
//               uint8_t *data, size_t len, bool final) {
//   log_e("onUpload");
// }

// void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
//              AwsEventType type, void *arg, uint8_t *data, size_t len) {
//   log_e("onEvent");
// }
