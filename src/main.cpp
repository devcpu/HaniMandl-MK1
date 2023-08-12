#include "esp_log.h"
#include <Arduino.h>
#include <ESPFS.h>
#include <WiFiManagerX.h>

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
  static char looper[9] = "|/-\\|/-\\";
  if (counter == 8) {
    counter = 0;
  }
  Serial.print("\b");
  Serial.print(looper[counter]);
  counter += 1;
  delay(100);
}

void loop() { donothing(); }
