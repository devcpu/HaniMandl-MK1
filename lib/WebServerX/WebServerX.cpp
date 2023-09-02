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
 * Last Modified: 2023-09-02 01:38
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <WebServerX.h>

// extern QueueHandle_t LoRaTXQueue;

AsyncWebServer *WebServer;
AsyncWebSocket *ws;
AsyncWebSocketClient *globalClient = NULL;

void WebserverStart(void) {
  log_i("starting Webserver");
  WebServer = new AsyncWebServer(80);
  ws = new AsyncWebSocket("/ws");

  ws->onEvent(onWsEvent);
  WebServer->addHandler(ws);

  WebServer->onNotFound([](AsyncWebServerRequest *request) {
    showRequest(request);
    log_e("%s not found! Send 404",
          request->url().c_str());  // TODO - // better send info to client
    request->send(404);
  });

  WebServer->serveStatic("/hanimandlmk1.css", SPIFFS, "/hanimandlmk1.css");
  WebServer->serveStatic("/w3.css", SPIFFS, "/w3.css");
  WebServer->serveStatic("/w3pro.css", SPIFFS, "/w3pro.css");
  WebServer->serveStatic("/w3-theme-xcolor.css", SPIFFS,
                         "/w3-theme-xcolor.css");
  WebServer->serveStatic("/rebootinfo", SPIFFS, "/reboot.html");
  WebServer->serveStatic("/wabe.jpg", SPIFFS, "/wabe.jpg");
  //   WebServer->serveStatic("/hanimandlmk1.js", SPIFFS, "/hanimandlmk1.js");

  // is't needs templating!
  WebServer->on(
      "/hanimandlmk1.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        log_e("/hanimandlmk1.js");
        request->send(SPIFFS, "/hanimandlmk1.js", "application/javascript",
                      false, DefaultTemplating);
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
    if (HMConfig::instance().beekeeping == "") {  // first run wizard
      log_d("redirect to setup");
      request->redirect("/setup");
    }
    log_d("/");
    request->send(SPIFFS, "/master.html", "text/html", false, DefaultProcessor);
  });

  WebServer->on("/filling", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/master.html", "text/html", false,
                  FillingTemplating);
  });

  WebServer->on("/setupfilling", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (showRequest(request)) {  // we got data
      String weight_filling_S = (getWebParam(request, "weight_filling"));
      if (isNumber(weight_filling_S)) {
        HMConfig::instance().weight_filling = weight_filling_S.toInt();
      }
      HMConfig::instance().date_filling = getWebParam(request, "date_filling");
      HMConfig::instance().los_number = getWebParam(request, "los_number");
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupFillingTemplating);
  });

  WebServer->on("/emptyglass", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (showRequest(request)) {
      String glass_empty_S = (getWebParam(request, "glass_empty"));
      if (isNumber(glass_empty_S)) {
        HMConfig::instance().glass_empty = glass_empty_S.toInt();
      }
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupFillingTemplating);
  });

  WebServer->on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(SPIFFS, "/master.html", "text/html", false, SetupTemplating);
  });

  WebServer->on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(SPIFFS, "/master.html", "text/html", false,
                  CalibrateTemplating);
  });

  WebServer->on("/setupwlan", HTTP_GET, [](AsyncWebServerRequest *request) {
    showRequest(request);
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupWlanTemplating);
  });

  WebServer->on("/updatefirmware", HTTP_GET,
                [](AsyncWebServerRequest *request) {
                  showRequest(request);
                  request->send(SPIFFS, "/master.html", "text/html", false,
                                UpdateFirmwareTemplating);
                });

  /* ------------------------------- System Info ------------------------------
   */
  WebServer->on("/systeminfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/systeminfo");
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SystemInfoTemplating);
  });

  /* --------------------------------- Reboot ---------------------------------
   */
  WebServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    log_e("/reboot");
    ESPHelper::reboot(request);
  });

  WebServer->begin();
  log_e("HTTP WebServer started");
}

/*
.##.....##.########.##.......########..########.########...######.
.##.....##.##.......##.......##.....##.##.......##.....##.##....##
.##.....##.##.......##.......##.....##.##.......##.....##.##......
.#########.######...##.......########..######...########...######.
.##.....##.##.......##.......##........##.......##...##.........##
.##.....##.##.......##.......##........##.......##....##..##....##
.##.....##.########.########.##........########.##.....##..######.
*/

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
    char buf[80];  // FIXME(janusz)
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
    log_d("GET");
  else if (request->method() == HTTP_POST)
    log_d("POST");
  else if (request->method() == HTTP_DELETE)
    log_d("DELETE");
  else if (request->method() == HTTP_PUT)
    log_d("PUT");
  else if (request->method() == HTTP_PATCH)
    log_d("PATCH");
  else if (request->method() == HTTP_HEAD)
    log_d("HEAD");
  else if (request->method() == HTTP_OPTIONS)
    log_d("OPTIONS");
  else
    log_d("UNKNOWN");
  log_d("http://%s%s\n", request->host().c_str(), request->url().c_str());

  if (request->contentLength()) {
    log_d("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    log_d("_CONTENT_LENGTH: %u\n", request->contentLength());
    log_d("Args: %d", request->args());
  }

  int i;

  //   int headers = request->headers();
  //   log_d("count headers %d", headers);
  //   for (i = 0; i < headers; i++) {
  //     AsyncWebHeader *h = request->getHeader(i);
  //     log_d("HEADER %s: %s\n", h->name().c_str(), h->value().c_str());
  //    }

  int params_count = request->params();
  log_d("count params_count %d", params_count);
  for (i = 0; i < params_count; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile()) {
      log_d("FILE %s: %s, size: %u\n", p->name().c_str(), p->value().c_str(),
            p->size());
    } else if (p->isPost()) {
      log_d("POST %s: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      log_d("GET/PUT/... %s: %s\n", p->name().c_str(), p->value().c_str());
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
    log_d("Websocket client connection received");
    globalClient = client;
  } else if (type == WS_EVT_DISCONNECT) {
    log_d("Websocket client connection finished");
    globalClient = NULL;
  } else if (type == WS_EVT_ERROR) {
    log_d("Got WS_EVT_ERROR");
  } else if (type == WS_EVT_PONG) {
    log_d("Got WS_EVT_PONG");
  } else if (type == WS_EVT_DATA) {
    log_d("Websocket client sended data");
    AwsFrameInfo *info = reinterpret_cast<AwsFrameInfo *>(arg);
    if (info->opcode == WS_TEXT) {
      data[len] = 0;
      log_d("%s\n", reinterpret_cast<char *>(data));
      String wsdata = String(reinterpret_cast<char *>(data));
      log_d("%s", wsdata.c_str());
      KeyValue rdata = split(wsdata);
      log_d("key: %s", rdata.key.c_str());
      log_d("value: %s", rdata.value.c_str());

      if (rdata.key == "angle_max") {
        log_d("got angle_max");
        if (isNumber(rdata.value)) {
          log_d("Before: %d", HMConfig::instance().servodata.angle_max);
          HMConfig::instance().servodata.angle_max = rdata.value.toInt();
          log_d("After: %d", HMConfig::instance().servodata.angle_max);
        } else {
          log_d("value of angle_max is not a valid Int %s", rdata.value);
        }
      }

      if (rdata.key == "angle_fine") {
        log_d("got angle_fine");
        if (isNumber(rdata.value)) {
          log_d("Before: %d", HMConfig::instance().servodata.angle_fine);
          HMConfig::instance().servodata.angle_fine = rdata.value.toInt();
          log_d("After: %d", HMConfig::instance().servodata.angle_fine);
        } else {
          log_d("value of angle_fine is not a valid Int %s", rdata.value);
        }
      }

      if (rdata.key == "weight_fine") {
        log_d("got weight_fine");
        if (isNumber(rdata.value)) {
          log_d("Before: %d", HMConfig::instance().weight_fine);
          HMConfig::instance().weight_fine = rdata.value.toInt();
          log_d("After: %d", HMConfig::instance().weight_fine);
        } else {
          log_e("value of weight_fine is not a valid Int %s", rdata.value);
        }
      }

      if (rdata.key == "button") {
        log_d("got button with %s", rdata.value);
        if (rdata.value == "auto") {
          HMConfig::instance().run_modus = RUN_MODUS_AUTO;
          HMConfig::instance().fs = FILLING_STATUS_CLOSED;
        } else if (rdata.value == "hand") {
          HMConfig::instance().run_modus = RUN_MODUS_HAND;
        } else if (rdata.value == "start") {
          log_d("[Start]");
          HMConfig::instance().start = true;
        } else if (rdata.value == "stop") {
          HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
        } else {
          log_e("Got wrong value for %s: %s", rdata.value.c_str(),
                rdata.key.c_str());
        }
      }
    }
  }
}

/**
 * The function "split" takes a string as input, splits it into a key-value pair
 * using the "=" delimiter, and returns the key and value as a KeyValue struct.
 *
 * @param wsdata The parameter "wsdata" is a string that contains key-value data
 * in the format "key=value".
 *
 * @return a KeyValue object.
 */
KeyValue split(String wsdata) {
  log_d("%s", wsdata.c_str());
  KeyValue rval;
  uint8_t idx = wsdata.indexOf("=");
  rval.key = wsdata.substring(0, idx);
  rval.value = wsdata.substring(idx + 1);
  log_d("Key: %s Value: %s", rval.key.c_str(), rval.value.c_str());
  return rval;
}

void sendSocketData() {
  if (globalClient == NULL) {
    log_d("globalClient == NULL");
    return;
  }
  if (globalClient->status() != WS_CONNECTED) {
    log_d("globalClient->status() != WS_CONNECTED");
    return;
  }
  static uint32_t last = 0;
  uint32_t i = millis();
  if (i < last + 800) {
    return;
  }
  last = i;

  // make json
  StaticJsonDocument<96> doc;
  String output;  // FIXME: should be char[] instand
  doc["glass_count"] = HMConfig::instance().glass_count;
  doc["waagen_gewicht"] = HMConfig::instance().weight_current;
  doc["run_modus"] =
      HMConfig::instance().runmod2string(HMConfig::instance().run_modus);
  //   uint8_t len = measureJsonPretty(doc);
  serializeJson(doc, output);
  ws->textAll(output.c_str());
}
