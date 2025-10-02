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
 * Last Modified: 2024-04-29 17:27
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
  // pinMode(PIN_WIFI_LED, OUTPUT);
  // ticker.attach(0.6, tick);
  Serial.println("Starting WiFiManager");
  // // log_i();
  // // log_i();
  // AsyncWiFiManager wifiManager(&server, &dns);

  // //   wifiManager.resetSettings();

  // TODO(janusz) move to central config.h
  IPAddress _ip = IPAddress(WIFI_IP);
  IPAddress _gw = IPAddress(WIFI_GATEWAY);
  IPAddress _sn = IPAddress(WIFI_SUBNET);
  IPAddress _dns1 = IPAddress(WIFI_DNS1);
  IPAddress _dns2 = IPAddress(WIFI_DNS2);

  WiFi.config(_ip, _gw, _sn, _dns1);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("[+] ESP32 IP : ");
  Serial.println(WiFi.localIP());
  String temp_ip = WiFi.localIP().toString();
  strlcpy(HMConfig::instance().localIP, temp_ip.c_str(),
          sizeof(HMConfig::instance().localIP));
  // // set callback that gets called when connecting to previous WiFi fails,
  // and we need to start configuration mode
  // wifiManager.setAPCallback(configModeCallback);

  // // fetches ssid and pass from eeprom and tries to connect
  // // if it does not connect it starts an access point with the specified name
  // // here  "AutoConnectAP"
  // // and goes into a blocking loop awaiting configuration
  // // if you are connected to it at

  return true;

  // wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn, _dns1, _dns2);
  // //   wifiManager.autoConnect("AutoConnectAP");
  // if (!wifiManager.autoConnect(
  //         WIFI_AP_NAME,
  //         WIFI_AP_PASSWORD)) {
  //   log_e("failed to connect, we should reset as see if it connects");
  //   delay(3000);
  //   ESP.restart();
  //   delay(5000);
  // }

  // // if you get here you have connected to the WiFi
  // log_i("connected...yeey :)");
  // log_i("local ip");
  // // log_i(WiFi.localIP().toString.c_str());
  // HMConfig::instance().localIP = WiFi.localIP().toString();

  // ticker.detach();
  // // keep LED on
  // digitalWrite(PIN_WIFI_LED, LOW);
  // return true;
}

String getNTPDate(int16_t gmt_offset_sec, int16_t daylight_offset_sec,
                  const char *ntpserver) {
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

  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  String timeString = String(buffer);
  log_e("Date: %s", timeString.c_str());
  return timeString;
}
