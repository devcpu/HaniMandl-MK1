/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /WiFiManagerX.cpp
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-12 15:55
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <WiFiManagerX.h>

AsyncWebServer server(80);
// extern DNSServer dns;
extern Ticker ticker;

// default custom static IP
// char static_ip[16] = "10.0.1.59";
// char static_gw[16] = "10.0.1.1";
// char static_sn[16] = "255.255.255.0";

// void tick() {
//   // toggle state
//   int state = digitalRead(PIN_WIFI_LED);  // get the current state of
//   WIFI_LED pin digitalWrite(PIN_WIFI_LED, !state);     // set pin to the
//   opposite state
// }

// gets called when WiFiManager enters configuration mode
// void configModeCallback(AsyncWiFiManager *myWiFiManager) {
//   Serial.println("Entered config mode");
//   Serial.println(WiFi.softAPIP());
//   // if you used auto generated SSID, print it
//   Serial.println(myWiFiManager->getConfigPortalSSID());
//   // entered config mode, make led toggle faster
//   ticker.attach(0.2, tick);
// }

bool setupWifi() {
  Serial.println("Starting WiFi setup");

  // No SSID configured -> start directly in AP mode
  if (strlen(WIFI_SSID) == 0) {
    Serial.println("[WiFi] No SSID configured, starting AP mode");
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(WIFI_AP_NAME, WIFI_AP_PASSWORD, 1, 0, 1)) {
      Serial.print("[WiFi] AP active, SSID=");
      Serial.println(WIFI_AP_NAME);
      Serial.print("[WiFi] AP IP: ");
      Serial.println(WiFi.softAPIP());
      strlcpy(HMConfig::instance().localIP, WiFi.softAPIP().toString().c_str(),
              sizeof(HMConfig::instance().localIP));
    }
    return true;
  }

  // STA mode with configured credentials
  IPAddress _ip = IPAddress(WIFI_IP);
  IPAddress _gw = IPAddress(WIFI_GATEWAY);
  IPAddress _sn = IPAddress(WIFI_SUBNET);
  IPAddress _dns1 = IPAddress(WIFI_DNS1);

  // Only set static IP if a non-zero IP was configured
  if ((uint32_t)_ip != 0) {
    WiFi.config(_ip, _gw, _sn, _dns1);
  }
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait up to 15s for connection, then let tickWiFi handle fallback
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    Serial.print(".");
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("[+] ESP32 IP : ");
    Serial.println(WiFi.localIP());
    strlcpy(HMConfig::instance().localIP, WiFi.localIP().toString().c_str(),
            sizeof(HMConfig::instance().localIP));
  } else {
    Serial.println(
        "\n[WiFi] STA connect timeout in setup, tickWiFi will retry");
  }

  return true;
}

String getNTPDate(int16_t gmt_offset_sec, int16_t daylight_offset_sec,
                  const char* ntpserver) {
  uint8_t retry = 0;
  struct tm timeinfo;
  do {
    Serial.print(".");
    configTime(gmt_offset_sec, daylight_offset_sec, ntpserver);
    delay(2000);
  } while (!getLocalTime(&timeinfo) && ++retry < 10);
  Serial.println();
  if (retry >= 10) {
    log_e("Failed to obtain time after 10 retries");
    return String("0");
  }

  char buffer[80];  // flawfinder: ignore
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  String timeString = String(buffer);
  log_e("Date: %s", timeString.c_str());
  return timeString;
}
