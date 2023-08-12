/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /main.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-12 15:55
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-12 17:52
 * Modified By: Johannes G.  Arlt
 */

#include <Arduino.h>
#include <ESPFS.h>
#include <WiFiManagerX.h>

#include "esp_log.h"

extern AsyncWebServer server;
// extern DNSServer dns;
// extern Ticker ticker;
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  log_i("Start Setup");
  if (!ESPFSInit()) {
    delay(10000);
    ESP.restart();
  }
  setupWifiManager();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello, world");
  });
  server.onNotFound(notFound);
  server.begin();

  log_i("Setup done! Starting loop ... ");
}

static void donothing() {
  static int counter = 0;
  static char looper[12] = "|/-\\|/-\\";  // Flawfinder: ignore
  if (counter == 8) {
    counter = 0;
  }
  Serial.print("\b");
  Serial.print(looper[counter]);
  counter += 1;
  delay(100);
}

void loop() { donothing(); }
