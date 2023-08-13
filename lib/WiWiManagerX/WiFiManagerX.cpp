/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /WiFiManagerX.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/WiWiManagerX
 * Description:
 * -----
 * Created Date: 2023-08-12 15:55
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-13 02:37
 * Modified By: Johannes G.  Arlt
 */

#include <WiFiManagerX.h>

AsyncWebServer server(80);
DNSServer dns;
Ticker ticker;
extern HMConfig cfg;

// default custom static IP
// char static_ip[16] = "10.0.1.59";
// char static_gw[16] = "10.0.1.1";
// char static_sn[16] = "255.255.255.0";

const int WIFI_LED = CONFIG_WIFI_LED;  // TODO(janusz) move to central place

void tick() {
  // toggle state
  int state = digitalRead(WIFI_LED);  // get the current state of WIFI_LED pin
  digitalWrite(WIFI_LED, !state);     // set pin to the opposite state
}

// gets called when WiFiManager enters configuration mode
void configModeCallback(AsyncWiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  // entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

bool setupWifiManager() {
  pinMode(WIFI_LED, OUTPUT);
  ticker.attach(0.6, tick);
  AsyncWiFiManager wifiManager(&server, &dns);

  // wifiManager.resetSettings();

  // TODO(janusz) move to central config.h
  IPAddress _ip = IPAddress(172, 19, 19, 15);
  IPAddress _gw = IPAddress(192, 168, 42, 101);
  IPAddress _sn = IPAddress(255, 255, 255, 0);
  IPAddress _dns1 = IPAddress(192, 168, 42, 101);
  IPAddress _dns2 = IPAddress(8, 8, 8, 8);
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn, _dns1, _dns2);
  if (!wifiManager.autoConnect(
          "HaniMandlMKI",
          "Honigkuchen")) {  // TODO(janusz) move to config.h
    log_e("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  // if you get here you have connected to the WiFi
  log_i("connected...yeey :)");
  log_i("local ip");
  // log_i(WiFi.localIP().toString.c_str());
  cfg.myIP = WiFi.localIP();

  ticker.detach();
  // keep LED on
  digitalWrite(WIFI_LED, LOW);
  return true;
}
