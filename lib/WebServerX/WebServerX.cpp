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
 * Last Modified: 2023-08-15 23:56
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <WebServerX.h>

struct HTML_Error {
  String ErrorMsg;
  boolean isSended;

 public:
  void setErrorMsg(String msg) {
    ErrorMsg.isEmpty() ? ErrorMsg = msg : ErrorMsg += msg;
    isSended = false;
  }
  String getErrorMsg(void) {
    isSended = true;
    return ErrorMsg;
  }
};

HTML_Error html_error;

// extern QueueHandle_t LoRaTXQueue;

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
                                "application/javascript", false, DefaultPlaceholderProcessor);
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

  /*
  .##.....##.########.########.########...........######...########.########
  .##.....##....##.......##....##.....##.........##....##..##..........##...
  .##.....##....##.......##....##.....##.........##........##..........##...
  .#########....##.......##....########..........##...####.######......##...
  .##.....##....##.......##....##................##....##..##..........##...
  .##.....##....##.......##....##................##....##..##..........##...
  .##.....##....##.......##....##........#######..######...########....##...
  */

  /* ---------------------------------- main ----------------------------------
   */
  WebServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_d("/");
    // first run wizard
    if (cfg.beekeeping == "") {  // first run wizard
      log_d("redirect to setup");
      request->redirect("/setup");
    }
    log_d("/");
    request->send(SPIFFS, "/master.html", "text/html", false, DefaultProcessor);
  });

  WebServer->on("/filling", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/master.html", "text/html", false, ProcessorFilling);
  });

  WebServer->on("/setupfilling", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (showRequest(request)) {  // we got data

      String weight_filling_S = (getWebParam(request, "weight_filling"));
      if (isNumber(weight_filling_S)) {
        cfg.weight_filling = weight_filling_S.toInt();
      }

      String weight_empty_S = (getWebParam(request, "weight_empty"));
      if (isNumber(weight_empty_S)) {
        cfg.weight_empty = weight_empty_S.toInt();
      }

      cfg.date_filling = getWebParam(request, "date_filling");
      cfg.los_number = getWebParam(request, "los_number");
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  ProcessorSetupFilling);
  });

  WebServer->on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(SPIFFS, "/master.html", "text/html", false, ProcessorSetup);
  });

  WebServer->on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(SPIFFS, "/master.html", "text/html", false,
                  ProcessorCalibrate);
  });
  WebServer->on("/setupwlan", HTTP_GET, [](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(SPIFFS, "/master.html", "text/html", false,
                  ProcessorSetupWlan);
  });

  WebServer->on("/updatefirmware", HTTP_GET,
                [](AsyncWebServerRequest *request) {
                  showRequest(request);
                  request->send(SPIFFS, "/master.html", "text/html", false,
                                ProcessorUpdateFirmware);
                });

  /* ------------------------------- System Info ------------------------------
   */
  WebServer->on("/systeminfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/systeminfo");
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SystemInfoProcessor);
  });

  /* --------------------------------- Reboot ---------------------------------
   */
  WebServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/reboot");
    reboot(request);
  });

  WebServer->begin();
  log_e("HTTP WebServer started");
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

String DefaultPlaceholderProcessor(const String &var) {
  if (var == "SERVER_IP") {
    log_e("%s", cfg.localIP.c_str());
    return cfg.localIP;
  }
  if (var == "HTMLTILE") {
    return htmltitle;
  }
  if (var == "H3TITLE") {
    return h3title;
  }
  if (var == "ERRORMSG") {
    return html_error.getErrorMsg();
  }
  if (var == "mainmenue") {
    return mainmenue;
  }
  if (var == "los_number") {
    return cfg.los_number;
  }
  if (var == "date_filling") {
    return cfg.date_filling;
  }
  if (var == "weight_empty") {
    return String(cfg.weight_empty);
  }
  if (var == "weight_filling") {
    return String(cfg.weight_filling);
  }
  if (var == "beekeeping") {
    return cfg.beekeeping;
  }
  if (var == "angle_max_hard") {
    return String(cfg.angle_max_hard);
  }
  if (var == "angle_min_hard") {
    return String(cfg.angle_min_hard);
  }
  if (var == "angle_max") {
    return String(cfg.angle_max);
  }
  if (var == "angle_min") {
    return String(cfg.angle_min);
  }
  if (var == "angle_fine") {
    return String(cfg.angle_fine);
  }
  if (var == "glass_tolerance") {
    return String(cfg.glass_tolerance);
  }
  if (var == "weight_calibrate") {
    return String(cfg.weight_calibrate);
  }
  if (var == "glass_count") {
    return String(cfg.glass_count);
  }
  if (var == "weight_fine") {
    return String(cfg.weight_fine);
  }
  return "wrong placeholder " + var;
}

String ProcessorSetupFilling(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setupfilling.html");
  }
  return DefaultPlaceholderProcessor(var);
}

String ProcessorSetup(const String &var) {
  if (var == "H2TITLE") {
    return "Grundeinrichtung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setup.html");
  }
  return DefaultPlaceholderProcessor(var);
}

String ProcessorCalibrate(const String &var) {
  if (var == "H2TITLE") {
    return "Waage kalibrieren";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/kalibrieren.html");
  }
  return DefaultPlaceholderProcessor(var);
}

String ProcessorSetupWlan(const String &var) {
  if (var == "H2TITLE") {
    return "Einrichtung Wlan";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setupwlan.html");
  }
  return DefaultPlaceholderProcessor(var);
}

String ProcessorUpdateFirmware(const String &var) {
  if (var == "H2TITLE") {
    return "Update Firmware";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/updatefirmware.html");
  }
  return DefaultPlaceholderProcessor(var);
}

String ProcessorFilling(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/filling.html");
  }
  return DefaultPlaceholderProcessor(var);
}

String SystemInfoProcessor(const String &var) {
  if (var == "H2TITLE") {
    return "System Info";
  }
  if (var == "BODY") {
    return getSystemInfoTable();
  }
  return DefaultPlaceholderProcessor(var);
}

String DefaultProcessor(const String &var) {
  if (var == "H2TITLE") {
    return "Main Menue";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/mainbutton.html");
  }
  return DefaultPlaceholderProcessor(var);
}

/*
.##.....##....###....##....##.########..##.......########.########.
.##.....##...##.##...###...##.##.....##.##.......##.......##.....##
.##.....##..##...##..####..##.##.....##.##.......##.......##.....##
.#########.##.....##.##.##.##.##.....##.##.......######...########.
.##.....##.#########.##..####.##.....##.##.......##.......##...##..
.##.....##.##.....##.##...###.##.....##.##.......##.......##....##.
.##.....##.##.....##.##....##.########..########.########.##.....##
*/

/*
.##.....##.########.##.......########..########.########...######.
.##.....##.##.......##.......##.....##.##.......##.....##.##....##
.##.....##.##.......##.......##.....##.##.......##.....##.##......
.#########.######...##.......########..######...########...######.
.##.....##.##.......##.......##........##.......##...##.........##
.##.....##.##.......##.......##........##.......##....##..##....##
.##.....##.########.########.##........########.##.....##..######.
*/

bool isNumber(String val) {
  uint8_t length = val.length();
  char buf[length];
  val.toCharArray(buf, length);

  for (uint8_t i = 0; i < length; i++) {
    if (!isDigit(buf[i])) {
      return false;
    }
  }
  return true;
}

/**
 * The function `getChipId` returns a string that combines the hexadecimal
 * representation of the upper 16 bits and lower 32 bits of the ESP chip ID.
 *
 * @return String ESP32 chip id
 */
String getChipId() {
  char ssid1[MAXSIZE + 1];
  char ssid2[MAXSIZE + 1];
  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid1, sizeof(ssid1), "%04X", chip);
  snprintf(ssid2, sizeof(ssid1), "%08X", (uint32_t)chipid);
  return String(ssid1) + String(ssid2);
}

/**
 * The function `getSystemInfoTable` returns a table containing various system
 * information such as software version, build date and time, SDK version,
 * uptime, chip revision, flash chip size, sketch size, and free heap size.
 *
 * @return a string that contains a table of system information.
 */
String getSystemInfoTable(void) {
  FlashMode_t ideMode = ESP.getFlashChipMode();

  SystemData systemdata[] = {
      {"SoftwareVersion", cfg.version},
      {"Build DateTime: ", getBuildDateAndTime()},
      {"SDKVersion: ", String(ESP.getSdkVersion())},
      {"Uptime: ", String(millis() / 1000 / 60, DEC) + "min"},
#ifdef ESP32
      {"Chip Revision:", String(ESP.getChipRevision())},
      {"ESP32 Chip ID:", getChipId()},
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
#elif defined(ESP8266)
      {"Flash real id:", String(ESP.getFlashChipId(), HEX)},
      {"Flash real size:", String(ESP.getFlashChipRealSize() / 1024) + "kB"},
#endif
      {"Flash ide  size:", String(ESP.getFlashChipSize() / 1024) + "kB"},
      {"Flash ide speed:",
       String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"},
      {"Flash ide mode:", String((ideMode == FM_QIO    ? "QIO"
                                  : ideMode == FM_QOUT ? "QOUT"
                                  : ideMode == FM_DIO  ? "DIO"
                                  : ideMode == FM_DOUT ? "DOUT"
                                                       : "UNKNOWN"))},
      {"Sketch size:", String(ESP.getSketchSize() / 1024) + "kB"},
      {"Free sketch size:", String(ESP.getFreeSketchSpace() / 1024) + "kB"},
      {"Free heap:", String(ESP.getFreeHeap() / 1024) + "kB"},
#ifdef ESP32
#elif defined(ESP8266)
      {"ResetReason", String(ESP.getResetReason())},
#endif
  };

  String systemInfoTable = table2DGenerator(
      systemdata, sizeof(systemdata) / sizeof(systemdata[0]), true);
  return systemInfoTable + mainmenue;
}

/**
 * The function GetBuildDateAndTime returns a string representing the build date
 * and time in the format "YYYY-MM-DD HH:MM:SS".
 *
 * @return a string that represents the build date and time in the format
 * "YYYY-MM-DD HH:MM:SS".
 */
String getBuildDateAndTime() {
  char bdt[45];             // "2017-03-07 11:08:02"
  char mdate[] = __DATE__;  // "Mar  7 2017"
  int month = 0;
  int day = 0;
  int year = 0;

  sscanf(mdate, "%s %d %d", bdt, &day, &year);

  const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  for (int i = 0; i < 12; i++) {
    if (strcmp(bdt, monthNames[i]) == 0) {
      month = i + 1;
      break;
    }
  }
  snprintf(bdt, sizeof(bdt), "%04d-%02d-%02d %s", year, month, day, __TIME__);

  return String(bdt);
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
 * @param	SystemData systemdata[]
 * @param	boolean	bold
 * @return	mixed
 */
String table2DGenerator(SystemData systemdata[], uint8_t size, boolean bold) {
  String tdstart("<tr><td>");
  String tdmittle("</td><td>");
  if (bold) {
    tdstart = "<tr><td><b>";
    tdmittle = "</b></td><td>";
  }
  String tdend("</td></tr>");
  String retvar("<table>");

  for (uint8_t z = 0; z < size; z++) {
    retvar +=
        tdstart + systemdata[z].label + tdmittle + systemdata[z].value + tdend;
  }
  retvar += "</table>";
  return retvar;
}

/**
 * The function `readSPIFFS2String` reads the content of a file from the SPIFFS
 * file system and returns it as a string.
 *
 * @param path The `path` parameter is a `String` that represents the file path
 * of the file to be read from SPIFFS (SPI Flash File System).
 *
 * @return The function `readSPIFFS2String` returns a `String` object.
 */
String readSPIFFS2String(const String &path) {
  if (!SPIFFS.exists(path)) {
    String error = "ERROR, " + path + " does not exist.";
    log_e("%s", error.c_str());
    return error;
  }

  File file = SPIFFS.open(path, "r");
  if (!file) {
    String error = "Failed to open file for reading";
    log_e("%s", error.c_str());
    return error;
  }

  String fileContent;
  while (file.available()) {
    fileContent += static_cast<char>(file.read());
  }

  file.close();
  return fileContent;
}

/**
 * The function generates an HTML select field with options based on the
 * provided data and selected value.
 *
 * @param selected The "selected" parameter is a string that represents the
 * currently selected option in the dropdown menu.
 * @param name The `name` parameter is a pointer to a character array that
 * represents the name of the select field in HTML.
 * @param data The "data" parameter is a 2-dimensional array of strings. Each
 * row in the array represents an option in the select field. The first column
 * of each row contains the display text for the option, and the second column
 * contains the corresponding value for the option.
 * @param size The parameter "size" represents the number of options in the
 * dropdown menu.
 *
 * @return a string that represents an HTML select field with options.
 */
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size) {
  ESP_LOGD("WebServerX", "%s", name);
  ESP_LOGD("WebServerX", "%s", selected);

  String buf = "\n\n<select name='" + String(name) + "'>\n";

  for (uint8_t i = 0; i < size; i++) {
    String selectxt = (selected == data[i][1]) ? " selected " : "";
    buf += "<option value=\"" + data[i][1] + "\"" + selectxt + ">" +
           data[i][0] + "</option>\n";
  }

  buf += "</select>\n\n";
  ESP_LOGD("WebServerX", "%s", name);

  return buf;
}

void reboot(AsyncWebServerRequest *request) {
  request->redirect("/rebootinfo");
  delay(3000);
  disconnect();
  restartESP();
}

void disconnect() {
  // Implement disconnect functionality here
}

void restartESP() { ESP.restart(); }

/* ------------------------------- getWebParam ------------------------------ */
/// @brief gets the value under given key from given http-request
/// @param request
/// @param key
/// @param cfgvar
/// @return
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

/* ------------------------------- getWebParam ------------------------------ */
/// @brief gets the value under given key from given http-request
/// @param request
/// @param key
/// @param cfgvar
/// @return
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

/* ------------------------------- getWebParam ------------------------------ */
/// @brief gets the value under given key from given http-request
/// @param request
/// @param key
/// @return
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

/* ------------------------------- showRequest ------------------------------ */
#if CORE_DEBUG_LEVEL > 4
/**
 * @brief return count of param and
 * useful debugging infos about HTTP-Requests if DEBUG_LEVEL > 4
 *
 * @param request
 * @return int count of params in HTTP-request
 */
int showRequest(AsyncWebServerRequest *request) {
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
  log_e("http://%s%s\n", request->host().c_str(), request->url().c_str());

  if (request->contentLength()) {
    log_e("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    log_e("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int i;

  //   int headers = request->headers();
  //   log_d("count headers %d", headers);
  //   for (i = 0; i < headers; i++) {
  //     AsyncWebHeader *h = request->getHeader(i);
  //     log_e("HEADER %s: %s\n", h->name().c_str(), h->value().c_str());
  //    }

  int params_count = request->params();
  log_d("count params_count %d", params_count);
  for (i = 0; i < params_count; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile()) {
      log_e("FILE %s: %s, size: %u\n", p->name().c_str(), p->value().c_str(),
            p->size());
    } else if (p->isPost()) {
      log_e("POST %s: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      log_e("GET/PUT/... %s: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
  return params_count;
}
#else
int showRequest(AsyncWebServerRequest *request) { return request->params(); }
#endif

/*
.##......##.########.########...######...#######...######..##....##.########.########
.##..##..##.##.......##.....##.##....##.##.....##.##....##.##...##..##..........##...
.##..##..##.##.......##.....##.##.......##.....##.##.......##..##...##..........##...
.##..##..##.######...########...######..##.....##.##.......#####....######......##...
.##..##..##.##.......##.....##.......##.##.....##.##.......##..##...##..........##...
.##..##..##.##.......##.....##.##....##.##.....##.##....##.##...##..##..........##...
..###..###..########.########...######...#######...######..##....##.########....##...
*/

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  log_d("got event");
  if (type == WS_EVT_CONNECT) {
    log_e("Websocket client connection received");
    globalClient = client;
  } else if (type == WS_EVT_DISCONNECT) {
    log_e("Websocket client connection finished");
    globalClient = NULL;
  } else if (type == WS_EVT_ERROR) {
    log_e("Got WS_EVT_ERROR");
  } else if (type == WS_EVT_PONG) {
    log_d("Got WS_EVT_PONG");
  } else if (type == WS_EVT_DATA) {
    log_e("Websocket client sended data");
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode == WS_TEXT) {
      log_d("%s\n", (char *)data);
      String wsdata = String((char *)data);
      log_d("%s", wsdata.c_str());
      KeyValue rdata = split(wsdata);
      log_d("%s", rdata.key.c_str());
      if (rdata.key == "angle_max") {
        log_d("Before: %d", cfg.angle_max);
        cfg.angle_max = rdata.value.toInt();
        log_d("After: %d", cfg.angle_max);
      }
      if (rdata.key == "angle_fine") {
        log_d("Before: %d", cfg.angle_fine);
        cfg.angle_fine = rdata.value.toInt();
        log_d("After: %d", cfg.angle_fine);
      }
      if (rdata.key == "weight_fine") {
        log_d("Before: %d", cfg.weight_fine);
        cfg.weight_fine = rdata.value.toInt();
        log_d("After: %d", cfg.weight_fine);
      }
    }
  }
}

KeyValue split(String wsdata) {
  log_d("%s", wsdata.c_str());
  KeyValue rval;
  uint8_t idx = wsdata.indexOf("=");
  rval.key = wsdata.substring(0, idx);
  rval.value = wsdata.substring(idx + 1);
  log_d("Key: %s Value: %s", rval.key.c_str(), rval.value.c_str());
  return rval;
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
