/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /webserverx.cpp
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-12 16:28
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2025-11-03 17:39
 * Modified By: Johannes G.  Arlt (janusz)
 */

/**
 * Multi-client WebSocket support (since 2025-10-04):
 *  - Up to MAX_WS_CLIENTS concurrent UI sessions (lightweight broadcast model)
 *  - Each client receives the same delta/event stream (no per-client state)
 *  - Rejects additional clients above the cap with a short status frame
 * (WS:LIMIT)
 */

#include <WebServerX.h>

#include "freertos_setup.h"  // system task & event interfaces
extern volatile uint32_t
    g_eventQueueOverflows;  // overflow counter from freertos_setup.cpp
// Include HX711 directly so we can call its methods (user request to use
// "HX711.h")
#include <loadcell.h>  // safeGetValue/safeGetUnits

#include "HX711.h"
// Fallback forward declaration (some build variants may not propagate header)
#ifndef SEND_SNAPSHOT_INTERNAL_DECLARED
void sendSnapshotInternal();
#endif
extern HX711 scale;  // defined in loadcell.cpp

extern Glass glass;

AsyncWebServer* WebServer;
AsyncWebSocket* ws;
/** @brief Support up to 3 concurrent WebSocket clients. */
static const uint8_t MAX_WS_CLIENTS = 3;
AsyncWebSocketClient* wsClients[MAX_WS_CLIENTS] = {nullptr, nullptr, nullptr};

/**
 * @brief Count currently connected (WS_CONNECTED) clients.
 * @return number of active slots
 */
uint8_t wsClientCount() {
  uint8_t c = 0;
  for (auto* cl : wsClients)
    if (cl && cl->status() == WS_CONNECTED) c++;
  return c;
}

/**
 * @brief Insert or reuse an empty slot for a new client.
 * @param client pointer provided by AsyncWebSocket
 * @return true if stored, false if no free slot (should not be called when
 * full)
 */
static bool addWsClient(AsyncWebSocketClient* client) {
  for (uint8_t i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (!wsClients[i] || wsClients[i]->status() != WS_CONNECTED) {
      wsClients[i] = client;
      return true;
    }
  }
  return false;
}

/**
 * @brief Remove a client from the slot table (on disconnect).
 */
static void removeWsClient(AsyncWebSocketClient* client) {
  for (uint8_t i = 0; i < MAX_WS_CLIENTS; ++i)
    if (wsClients[i] == client) wsClients[i] = nullptr;
}

/**
 * @brief Broadcast a textual frame to all connected clients.
 */
void broadcastText(const String& msg) {
  for (auto* cl : wsClients) {
    if (cl && cl->status() == WS_CONNECTED) cl->text(msg);
  }
}

void WebserverStart(void) {
  log_i("starting Webserver");
  WebServer = new AsyncWebServer(80);
  ws = new AsyncWebSocket("/ws");

  ws->onEvent(onWsEvent);
  WebServer->addHandler(ws);

  WebServer->onNotFound([](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
    showRequest(request);
#endif
    log_e("%s not found! Send 404",
          request->url().c_str());  // TODO - // better send info to client
    request->send(404);
  });

  WebServer->serveStatic("/honeyFillingMachine.css", SPIFFS,
                         "/honeyFillingMachine.css");
  WebServer->serveStatic("/w3.css", SPIFFS, "/w3.css");
  WebServer->serveStatic("/w3pro.css", SPIFFS, "/w3pro.css");
  WebServer->serveStatic("/w3-theme-xcolor.css", SPIFFS,
                         "/w3-theme-xcolor.css");
  WebServer->serveStatic("/rebootinfo", SPIFFS, "/reboot.html");
  WebServer->serveStatic("/wabe.jpg", SPIFFS, "/wabe.jpg");
  //   WebServer->serveStatic("/honeyFillingMachine.js", SPIFFS,
  //   "/honeyFillingMachine.js");

  // nonstatic! because, it needs templating!
  WebServer->on(
      "/honeyFillingMachine.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        log_e("/honeyFillingMachine.js");
        request->send(SPIFFS, "/honeyFillingMachine.js",
                      "application/javascript", false, JSTemplatingWrapper);
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
  WebServer->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    log_d("/");
    // first run wizard
    if (strlen(HMConfig::instance().beekeeping) == 0) {  // first run wizard
      log_d("redirect to setup");
      request->redirect("/setup");
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  DefaultTemplatingWrapper);
  });

  WebServer->on("/filling", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/master.html", "text/html", false,
                  FillingTemplatingWrapper);
  });

  WebServer->on("/setupfilling", HTTP_GET, [](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
    int paramCount = showRequest(request);
#else
    int paramCount = request->params();
#endif
    if (paramCount) {  // we got data
      bool changed = false;
      String weight_filling_S = (getWebParam(request, "weight_filling"));
      if (isNumber(weight_filling_S)) {
        HMConfig::instance().weight_filling = weight_filling_S.toInt();
        // NOTE - Importend! Also change weight_fine
        HMConfig::instance().weight_fine =
            HMConfig::instance().weight_filling -
            50;  // FIXME - one place only see HMCOnfig.weight_fine
        glass.cutoff_weight = HMConfig::instance().weight_filling;
        changed = true;
      }
      String glass_empty_S = (getWebParam(request, "glass_empty"));
      if (isNumber(glass_empty_S)) {
        HMConfig::instance().glass_empty = glass_empty_S.toInt();
        changed = true;
      }
      String temp_date = getWebParam(request, "date_filling");
      strlcpy(HMConfig::instance().date_filling, temp_date.c_str(),
              sizeof(HMConfig::instance().date_filling));
      if (temp_date.length() > 0) changed = true;

      String temp_los = getWebParam(request, "los_number");
      strlcpy(HMConfig::instance().los_number, temp_los.c_str(),
              sizeof(HMConfig::instance().los_number));
      if (temp_los.length() > 0) changed = true;
      if (changed) {
        HMConfig::instance().writeJsonConfig();
      }
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupFillingTemplatingWrapper);
  });

  WebServer->on("/emptyglass", HTTP_GET, [](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
    int paramCount = showRequest(request);
#else
    int paramCount = request->params();
#endif
    if (paramCount) {
      String glass_empty_S = (getWebParam(request, "glass_empty"));
      if (isNumber(glass_empty_S)) {
        HMConfig::instance().glass_empty = glass_empty_S.toInt();
      }
      HMConfig::instance().writeJsonConfig();
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupFillingTemplatingWrapper);
  });

  WebServer->on("/setup", HTTP_GET, [](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
    showRequest(request);
#endif
    log_e("->on /setup");
    String beekeeping = getWebParam(request, "beekeeping");
    bool changed = false;  // track any config field modifications (incl. servo
                           // & tolerance)
    if (beekeeping != "") {
      strlcpy(HMConfig::instance().beekeeping, beekeeping.c_str(),
              sizeof(HMConfig::instance().beekeeping));
      changed = true;
      log_e("saved data beekeeping: %s", HMConfig::instance().beekeeping);
    }
    // Additional future fields could be added here with changed=true when
    // modified
    String angle_max_hard_S = getWebParam(request, "angle_max_hard");
    if (isNumber(angle_max_hard_S)) {
      HMConfig::instance().servodata.angle_max_hard = angle_max_hard_S.toInt();
      log_e("saved data angle_max_hard=%d",
            HMConfig::instance().servodata.angle_max_hard);
      changed = true;
    }
    String angle_min_hard_S = getWebParam(request, "angle_min_hard");
    if (isNumber(angle_min_hard_S)) {
      HMConfig::instance().servodata.angle_min_hard = angle_min_hard_S.toInt();
      log_e("saved data angle_min_hard=%d",
            HMConfig::instance().servodata.angle_min_hard);
      changed = true;
    }
    String angle_max_S = getWebParam(request, "angle_max");
    if (isNumber(angle_max_S)) {
      HMConfig::instance().servodata.angle_max = angle_max_S.toInt();
      log_e("saved data angle_max=%d",
            HMConfig::instance().servodata.angle_max);
      changed = true;
    }
    String angle_min_S = getWebParam(request, "angle_min");
    if (isNumber(angle_min_S)) {
      HMConfig::instance().servodata.angle_min = angle_min_S.toInt();
      log_e("saved data angle_min=%d",
            HMConfig::instance().servodata.angle_min);
      changed = true;
    }
    String angle_fine_S = getWebParam(request, "angle_fine");
    if (isNumber(angle_fine_S)) {
      HMConfig::instance().servodata.angle_fine = angle_fine_S.toInt();
      log_e("saved data angle_fine=%d",
            HMConfig::instance().servodata.angle_fine);
      changed = true;
    }
    String glass_tolerance_S = getWebParam(request, "glass_tolerance");
    if (isNumber(glass_tolerance_S)) {
      HMConfig::instance().glass_tolerance = glass_tolerance_S.toInt();
      log_e("saved data glass_tolerance=%d",
            HMConfig::instance().glass_tolerance);
      changed = true;
    }
    if (changed) {
      // Persist all aggregated changes once to minimize flash wear
      HMConfig::instance().writeJsonConfig();
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupTemplatingWrapper);
  });

  WebServer->on("/calibrate", HTTP_GET, [](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
    showRequest(request);
#endif
    bool changed = false;
    // Step 1: Tare empty scale
    if (request->hasParam("empty_scale")) {
      log_i("[Calib] Step1: Tare empty scale");
      scale.tare();
      // Allow brief mechanical/electrical settle (50-100 ms)
      delay(75);
      HMConfig::instance().OFFSET = scale.get_offset();
      changed = true;
    }
    // Step 2: Known weight calibration (robust parsing + positive factor)
    if (request->hasParam("know_weight_scale")) {
      String w = getWebParam(request, "weight_calibrate");
      w.trim();
      w.replace(',', '.');
      if (w.length() == 0) {
        log_w("[Calib] empty weight_calibrate");
      } else {
        // allow decimal input
        double knownD = w.toDouble();
        if (knownD > 0.0 && knownD < 50000.0) {  // sanity upper bound
          uint32_t known = static_cast<uint32_t>(llround(knownD));
          HMConfig::instance().weight_calibrate = known;
          // optional discard to settle after user placing weight
          (void)safeGetUnits(1);
          long raw =
              safeGetValue(20, 500);  // guarded average with explicit timeout
          if (raw == 0) {
            log_w("[Calib] raw=0 (timeout?) skip factor");
          } else {
            long absRaw = labs(raw);
            if (absRaw < (long)(known * 0.1)) {
              log_w(
                  "[Calib] raw(%ld) unexpectedly small vs known(%u) -> check "
                  "placement",
                  absRaw, known);
            }
            double factor =
                static_cast<double>(absRaw) / static_cast<double>(known);
            scale.set_scale(factor);
            HMConfig::instance().SCALE = factor;
            // OFFSET remains from tare step (if performed)
            log_i("[Calib] raw=%ld known=%u factor=%f (orig_raw=%ld)", absRaw,
                  known, factor, raw);
            changed = true;
          }
        } else {
          log_w("[Calib] invalid numeric weight_calibrate: %s", w.c_str());
        }
      }
    }
    // Step 3: Measure empty glass weight (tare glass)
    if (request->hasParam("glass_weight_scale")) {
      log_i("[Calib] Step3: measure empty glass");
      float u = safeGetUnits(20);
      if (!isnan(u)) {
        int glassW = static_cast<int>(round(u));
        if (glassW > 0 && glassW < 2000) {
          HMConfig::instance().glass_empty = static_cast<uint16_t>(glassW);
          changed = true;
        } else {
          log_w("[Calib] glass weight out of bounds: %d", glassW);
        }
      } else {
        log_w("[Calib] timeout reading glass weight");
      }
    }
    if (changed) {
      // Persist OFFSET, SCALE, weight_calibrate, glass_empty
      HMConfig::instance().writeJsonConfig();
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  CalibrateTemplatingWrapper);
  });

  WebServer->on("/setupwlan", HTTP_GET, [](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
    showRequest(request);
#endif
    bool changed = false;
    // Accept optional static network params
    String ip = getWebParam(request, "ip_address");
    if (ip.length() > 0) {
      strlcpy(HMConfig::instance().wlan.ip_address, ip.c_str(),
              sizeof(HMConfig::instance().wlan.ip_address));
      changed = true;
    }
    String mask = getWebParam(request, "net_mask");
    if (mask.length() > 0) {
      strlcpy(HMConfig::instance().wlan.net_mask, mask.c_str(),
              sizeof(HMConfig::instance().wlan.net_mask));
      changed = true;
    }
    String gw = getWebParam(request, "gw");
    if (gw.length() > 0) {
      strlcpy(HMConfig::instance().wlan.gw, gw.c_str(),
              sizeof(HMConfig::instance().wlan.gw));
      changed = true;
    }
    String dns1 = getWebParam(request, "dns1");
    if (dns1.length() > 0) {
      strlcpy(HMConfig::instance().wlan.dns1, dns1.c_str(),
              sizeof(HMConfig::instance().wlan.dns1));
      changed = true;
    }
    String dns2 = getWebParam(request, "dns2");
    if (dns2.length() > 0) {
      strlcpy(HMConfig::instance().wlan.dns2, dns2.c_str(),
              sizeof(HMConfig::instance().wlan.dns2));
      changed = true;
    }
    if (changed) {
      HMConfig::instance().writeJsonConfig();
    }
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SetupWlanTemplatingWrapper);
  });

  WebServer->on("/updatefirmware", HTTP_GET,
                [](AsyncWebServerRequest* request) {
#if CORE_DEBUG_LEVEL > 4
                  showRequest(request);
#endif
                  request->send(SPIFFS, "/master.html", "text/html", false,
                                UpdateFirmwareTemplatingWrapper);
                });

  /* ------------------------------- System Info ------------------------------
   */
  WebServer->on("/systeminfo", HTTP_GET, [](AsyncWebServerRequest* request) {
    log_e("/systeminfo");
    request->send(SPIFFS, "/master.html", "text/html", false,
                  SystemInfoTemplatingWrapper);
  });

  WebServer->on("/hmconfig", HTTP_GET, [](AsyncWebServerRequest* request) {
    log_e("/hmconfig");
    request->send(SPIFFS, "/master.html", "text/html", false,
                  HMConfigTemplatingWrapper);
  });

  /* --------------------------------- Reboot ---------------------------------
   */
  WebServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) {
    log_e("/reboot");
    request->send(SPIFFS, "/reboot.html", "text/html");
    // Delayed restart to allow page to be sent
    xTaskCreate(
        [](void*) {
          vTaskDelay(pdMS_TO_TICKS(1500));
          ESP.restart();
        },
        "rebootTask", 2048, nullptr, 1, nullptr);
  });

  // Diagnostic endpoint: raw scale data (no templating)
  WebServer->on("/scale_raw", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Gather diagnostics
    bool ready = scale.is_ready();
    long raw = 0;
    float units = NAN;
    if (ready) {
      raw = safeGetValue(5, 150);
      units = safeGetUnits(5, 150);
    }
    double factor = scale.get_scale();
    long offset = scale.get_offset();
    bool calibrated =
        (factor != 0.0 && factor != 1.0) ||
        (offset != 0);  // heuristic: default (1,0) means not yet calibrated
    // Build JSON manually (small payload)
    String json = F("{");
    json += F("\"ready\":");
    json += (ready ? "true" : "false");
    json += F(",\"raw\":");
    json += raw;
    json += F(",\"units\":");
    if (!isnan(units))
      json += (int)units;
    else
      json += F("null");
    json += F(",\"scale\":");
    json += String(factor, 6);
    json += F(",\"offset\":");
    json += offset;
    json += F(",\"calibrated\":");
    json += (calibrated ? "true" : "false");
    json += F(",\"abs_raw\":");
    json += labs(raw);
    json += F(",\"timestamp\":");
    json += millis();
    json += F("}");
    request->send(200, "application/json", json);
  });

  ElegantOTA.begin(WebServer);
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
String getWebParam(AsyncWebServerRequest* request, const char* key,
                   String* cfgvar) {
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

/**
 * Retrieves the value of a web parameter from the given request.
 *
 * @param request The AsyncWebServerRequest object representing the incoming
 * request.
 * @param key The key of the web parameter to retrieve.
 * @param cfgvar A pointer to the variable where the retrieved value will be
 * stored.
 *
 * @return The value of the web parameter, or an empty string if the parameter
 * is not found.
 */
String getWebParam(AsyncWebServerRequest* request, const char* key,
                   uint8_t* cfgvar) {
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

/**
 * Retrieves the value of a specified parameter from the HTTP request.
 *
 * @param request The pointer to the AsyncWebServerRequest object representing
 * the HTTP request.
 * @param key The key of the parameter to retrieve.
 * @return The value of the specified parameter, or an empty string if the
 * parameter is not found.
 */
String getWebParam(AsyncWebServerRequest* request, const char* key) {
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
/**
 * @brief return count of param and
 * useful debugging infos about HTTP-Requests if DEBUG_LEVEL > 4
 *
 * @param request
 * @return int count of params in HTTP-request
 */
int showRequest(AsyncWebServerRequest* request) {
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
    const AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) {
      log_d("FILE %s: %s, size: %u\n", p->name().c_str(), p->value().c_str(),
            p->size());
    } else if (p->isPost()) {
      log_d("POST %s: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      log_d("GET/PUT/... %s: %s", p->name().c_str(), p->value().c_str());
    }
  }
  return params_count;
}
#else
int showRequest(AsyncWebServerRequest* request) { return request->params(); }
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

/**
 * The function "split" takes a string as input, splits it into a key-value pair
 * using the "=" delimiter, and returns the key and value as a KeyValue struct.
 *
 * @param wsdata The parameter "wsdata" is a string that contains key-value data
 * in the format "key=value".
 *
 * @return a KeyValue object.
 */
KeyValueArray split(KeyValueArray rval, String wsdata) {
  log_d("%s", wsdata.c_str());

  uint8_t count = 0;
  uint16_t str_position = 0;
  uint8_t key_value_idx = 0;
  bool found_amp = false;

  for (uint8_t i = 0; i < wsdata.length(); i++) {
    if (wsdata[i] == '=') {
      count++;
      // log_e("count = %d", count);
    }
  }
  if (count < 1) {
    log_e("No '=' found in %s", wsdata.c_str());
    return rval;
  } else {
    for (uint16_t i = 0; i < wsdata.length(); i++) {
      // log_e("str_position=%d; key_value_idx=%d; i=%d; char: %c",
      // str_position,
      //       key_value_idx, i, wsdata[i]);
      if (wsdata[i] == '=') {
        // log_e("str_position=%d; key_value_idx=%d; i=%d; key: %s",
        // str_position,
        //       key_value_idx, i, wsdata.substring(str_position, i).c_str());
        rval.keyValue[key_value_idx].key =
            wsdata.substring(str_position, i).c_str();
        str_position = i;

        for (uint8_t j = i; j < wsdata.length(); j++) {
          // log_e("str_position=%d; key_value_idx=%d; i=%d; j=%d; char: %c",
          //       str_position, key_value_idx, i, j, wsdata[j]);
          if (wsdata[j] == '&') {
            // log_e(
            //     "catch '&' str_position=%d; key_value_idx=%d; i)%d; j=%d; "
            //     "value: %s",
            //     str_position, key_value_idx, i, j,
            //     wsdata.substring(i + 1, j).c_str());
            rval.keyValue[key_value_idx].value =
                wsdata.substring(i + 1, j).c_str();
            str_position = j + 1;
            i = j;
            found_amp = true;
            key_value_idx++;
            // log_e("END if wsdata[j] == &");
            break;
          }
          // log_e("END for wsdata[j] search for &");
        }
        if (found_amp) {
          found_amp = false;
        } else {
          // log_e("str_position=%d; key_value_idx=%d; i=%d; value: %s",
          //       str_position, key_value_idx, i,
          //       wsdata.substring(i + 1, wsdata.length()).c_str());
          rval.keyValue[key_value_idx].value =
              wsdata.substring(i + 1, wsdata.length()).c_str();
          str_position = wsdata.length();
          rval.count = key_value_idx;
          // log_e("END if wsdata[i] == '=' ");
        }
      }
      // log_e("END for wsdata.length()");
    }
    // log_e("END else");
  }
  // log_e("MULTIVALUE: str_position=%d; key_value_idx=%d", str_position,
  //       key_value_idx);
  rval.count = key_value_idx;
  return rval;
}

/**
 * Handles WebSocket events.
 *
 * @param server The WebSocket server instance.
 * @param client The WebSocket client instance.
 * @param type The type of WebSocket event.
 * @param arg Additional argument associated with the event.
 * @param data The data received from the client.
 * @param len The length of the data received.
 */
void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  log_d("got event");
  if (type == WS_EVT_CONNECT) {
    IPAddress rip = client->remoteIP();
    uint16_t rport = client->remotePort();
    uint8_t current = wsClientCount();
    if (current >= MAX_WS_CLIENTS) {
      log_w("WS client limit reached (%u) -> rejecting %u.%u.%u.%u:%u", current,
            rip[0], rip[1], rip[2], rip[3], rport);
      client->text("WS:LIMIT");
      client->close();
      return;
    }
    addWsClient(client);
    log_d("Websocket client accepted (%u/%u) from %u.%u.%u.%u:%u", current + 1,
          MAX_WS_CLIENTS, rip[0], rip[1], rip[2], rip[3], rport);
    // Initial Snapshot an nur diesen Client senden
    // (sendSnapshotInternal() refactored later to broadcast; here we build
    // once)
    extern void buildSnapshot(
        String &
        out);  // forward (already static in other translation unit?) fallback
    String snap;  // local snapshot because buildSnapshot is static in
                  // freertos_setup.cpp; provide inline copy if linker complains
    // We cannot call static buildSnapshot here directly (diff TU); reuse
    // existing sendSnapshotInternal via broadcast wrapper later. For now call
    // sendSnapshotInternal which will broadcast to all (acceptable) or
    // reimplement minimal snapshot. Placeholder call - will be adapted after
    // refactor.
    sendSnapshotInternal();
  } else if (type == WS_EVT_DISCONNECT) {
    uint32_t cid = client ? client->id() : 0;
    IPAddress rip = client->remoteIP();
    removeWsClient(client);
    log_w(
        "Websocket client disconnect id=%lu from %u.%u.%u.%u "
        "queueOverflows=%lu remaining=%u",
        (unsigned long)cid, rip[0], rip[1], rip[2], rip[3],
        (unsigned long)g_eventQueueOverflows, wsClientCount());
  } else if (type == WS_EVT_ERROR) {
    log_d("Got WS_EVT_ERROR");
  } else if (type == WS_EVT_PONG) {
    log_d("Got WS_EVT_PONG");
  } else if (type == WS_EVT_DATA) {
    log_d("Websocket client sended data");
    AwsFrameInfo* info = reinterpret_cast<AwsFrameInfo*>(arg);
    if (info->opcode == WS_TEXT) {
      data[len] = 0;
      log_d("%s\n", reinterpret_cast<char*>(data));
      String wsdata = String(reinterpret_cast<char*>(data));
      log_d("%s", wsdata.c_str());
      KeyValueArray rdata;
      rdata.count = 0;
      rdata.keyValue[0].key = "";
      rdata.keyValue[0].value = "";
      rdata = split(rdata, wsdata);
      log_d("key: %s", rdata.keyValue[0].key.c_str());
      log_d("value: %s", rdata.keyValue[0].value.c_str());

      /*
      ##      ##  ######      ######   #######  ##    ## ######## ####  ######
      ##  ##  ## ##    ##    ##    ## ##     ## ###   ## ##        ##  ##    ##
      ##  ##  ## ##          ##       ##     ## ####  ## ##        ##  ##
      ##  ##  ##  ######     ##       ##     ## ## ## ## ######    ##  ##   ####
      ##  ##  ##       ##    ##       ##     ## ##  #### ##        ##  ##    ##
      ##  ##  ## ##    ##    ##    ## ##     ## ##   ### ##        ##  ##    ##
       ###  ###   ######      ######   #######  ##    ## ##       ####  ######
      */

      if (rdata.keyValue[0].key == "angle_max") {
        log_d("got angle_max");
        if (isNumber(rdata.keyValue[0].value)) {
          log_d("Before: %d", HMConfig::instance().servodata.angle_max);
          HMConfig::instance().servodata.angle_max =
              rdata.keyValue[0].value.toInt();
          log_d("After: %d", HMConfig::instance().servodata.angle_max);
          HMConfig::instance().writeJsonConfig();  // persist change
        } else {
          log_d("value of angle_max is not a valid Int %s",
                rdata.keyValue[0].value);
        }
      }

      if (rdata.keyValue[0].key == "angle_fine") {
        log_d("got angle_fine");
        if (isNumber(rdata.keyValue[0].value)) {
          log_d("Before: %d", HMConfig::instance().servodata.angle_fine);
          HMConfig::instance().servodata.angle_fine =
              rdata.keyValue[0].value.toInt();
          log_d("After: %d", HMConfig::instance().servodata.angle_fine);
          HMConfig::instance().writeJsonConfig();  // persist change
        } else {
          log_d("value of angle_fine is not a valid Int %s",
                rdata.keyValue[0].value);
        }
      }

      if (rdata.keyValue[0].key == "weight_fine") {
        log_d("got weight_fine");
        if (isNumber(rdata.keyValue[0].value)) {
          log_d("Before: %d", HMConfig::instance().weight_fine);
          HMConfig::instance().weight_fine = rdata.keyValue[0].value.toInt();
          log_d("After: %d", HMConfig::instance().weight_fine);
          HMConfig::instance().writeJsonConfig();  // persist change
        } else {
          log_e("value of weight_fine is not a valid Int %s",
                rdata.keyValue[0].value);
        }
      }

      if (rdata.keyValue[0].key == "glass_tolerance") {
        log_d("got glass_tolerance");
        if (isNumber(rdata.keyValue[0].value)) {
          log_d("Before: %d", HMConfig::instance().glass_tolerance);
          HMConfig::instance().glass_tolerance =
              rdata.keyValue[0].value.toInt();
          log_d("After: %d", HMConfig::instance().glass_tolerance);
        } else {
          log_e("value of glass_tolerance is not a valid Int %s",
                rdata.keyValue[0].value);
        }
      }

      /*
      ##      ##  ######     ########  ##     ## ######## ########  #######  ##
      ##
      ##  ##  ## ##    ##    ##     ## ##     ##    ##       ##    ##     ## ###
      ##
      ##  ##  ## ##          ##     ## ##     ##    ##       ##    ##     ##
      ####  ##
      ##  ##  ##  ######     ########  ##     ##    ##       ##    ##     ## ##
      ## ##
      ##  ##  ##       ##    ##     ## ##     ##    ##       ##    ##     ## ##
      ####
      ##  ##  ## ##    ##    ##     ## ##     ##    ##       ##    ##     ## ##
      ###
       ###  ###   ######     ########   #######     ##       ##     #######  ##
      ##
      */

      if (rdata.keyValue[0].key == "button") {
        log_d("got button with %s", rdata.keyValue[0].value);
        if (rdata.keyValue[0].value == "auto") {
          HMConfig::instance().run_modus = RUN_MODUS_AUTO;
          HMConfig::instance().fs = FILLING_STATUS_CLOSED;
          log_e("auto set");

        } else if (rdata.keyValue[0].value == "hand") {
          HMConfig::instance().run_modus = RUN_MODUS_HAND;
          HMConfig::instance().fs = FILLING_STATUS_CLOSED;
          log_e("hand set");

        } else if (rdata.keyValue[0].value == "start") {
          log_d("[Start]");
          HMConfig::instance().hm = HAND_MODE_OPEN;

        } else if (rdata.keyValue[0].value == "stop") {
          log_d("[Stop]");
          HMConfig::instance().hm = HAND_MODE_CLOSED;
          HMConfig::instance().emergency_stop =
              true;  // trigger immediate servo closure

        } else if (rdata.keyValue[0].value == "close") {
          log_d("[Close]");
          HMConfig::instance().hm = HAND_MODE_CLOSED;
          // Normal close - no emergency stop, stays in hand mode

        } else if (rdata.keyValue[0].value == "fine") {
          log_d("[Fine]");
          HMConfig::instance().hm = HAND_MODE_FINE;

          /*
          ##      ##  ######     ######## ########  ######  ########
          ##  ##  ## ##    ##       ##    ##       ##    ##    ##
          ##  ##  ## ##             ##    ##       ##          ##
          ##  ##  ##  ######        ##    ######    ######     ##
          ##  ##  ##       ##       ##    ##             ##    ##
          ##  ##  ## ##    ##       ##    ##       ##    ##    ##
           ###  ###   ######        ##    ########  ######     ##
          */

        } else if (rdata.keyValue[0].value == "servo_test") {
          log_e("button servo_test");
          log_e("rdata.keyValue[1].value: %d", rdata.keyValue[1].value.toInt());
          if (rdata.keyValue[1].key == "angle_max_hard") {
            log_e("button servo_test angle_max_hard");
            if (isNumber(rdata.keyValue[1].value)) {
              log_e("button servo_test angle_max_hard with int=%d",
                    rdata.keyValue[1].value.toInt());
              HMConfig::instance().servodata.angle_max_hard =
                  rdata.keyValue[1].value.toInt();
              HMConfig::instance().servodata.angle_test =
                  rdata.keyValue[1].value.toInt();
              HMConfig::instance().run_modus = RUN_MODUS_TEST;
            }
          }
          if (rdata.keyValue[1].key == "angle_min_hard") {
            log_e("button servo_test angle_min_hard");
            if (isNumber(rdata.keyValue[1].value)) {
              log_e("button servo_test angle_min_hard with int=%d",
                    rdata.keyValue[1].value.toInt());
              HMConfig::instance().servodata.angle_min_hard =
                  rdata.keyValue[1].value.toInt();
              HMConfig::instance().run_modus = RUN_MODUS_TEST;
              HMConfig::instance().servodata.angle_test =
                  rdata.keyValue[1].value.toInt();
            }
          }
          if (rdata.keyValue[1].key == "angle_max") {
            if (isNumber(rdata.keyValue[1].value)) {
              HMConfig::instance().servodata.angle_max =
                  rdata.keyValue[1].value.toInt();
              HMConfig::instance().run_modus = RUN_MODUS_TEST;
              HMConfig::instance().servodata.angle_test =
                  rdata.keyValue[1].value.toInt();
            }
          }
          if (rdata.keyValue[1].key == "angle_min") {
            if (isNumber(rdata.keyValue[1].value)) {
              HMConfig::instance().servodata.angle_min =
                  rdata.keyValue[1].value.toInt();
              HMConfig::instance().run_modus = RUN_MODUS_TEST;
              HMConfig::instance().servodata.angle_test =
                  rdata.keyValue[1].value.toInt();
            }
          }
          if (rdata.keyValue[1].key == "angle_fine") {
            if (isNumber(rdata.keyValue[1].value)) {
              HMConfig::instance().servodata.angle_fine =
                  rdata.keyValue[1].value.toInt();
              HMConfig::instance().run_modus = RUN_MODUS_TEST;
              HMConfig::instance().servodata.angle_test =
                  rdata.keyValue[1].value.toInt();
            }
          }
        } else {
          log_e("Got wrong value for %s: %s", rdata.keyValue[1].key.c_str(),
                rdata.keyValue[1].key.c_str());
        }
      }
    }
  }
}

/*
##      ##  ######      ######  ######## ##    ## ########
##  ##  ## ##    ##    ##    ## ##       ###   ## ##     ##
##  ##  ## ##          ##       ##       ####  ## ##     ##
##  ##  ##  ######      ######  ######   ## ## ## ##     ##
##  ##  ##       ##          ## ##       ##  #### ##     ##
##  ##  ## ##    ##    ##    ## ##       ##   ### ##     ##
 ###  ###   ######      ######  ######## ##    ## ########
*/

// sendSocketData removed (event-push architecture)
