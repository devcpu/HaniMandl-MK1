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
 * Last Modified: 2025-10-03 12:19
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef UNIT_TEST

#include <main.h>

#include "freertos_setup.h"

ESPFS espfs;
HX711 scale;
Servo servo;
Glass glass;
// DNSServer dns;
Ticker ticker;

void setup() {
  Serial.begin(115200);
  delay(1000);
  log_i("Start Setup");
  HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
  espfs.setup();
  setupWifi();  // initial WiFi setup (non-blocking task will manage
                // connection/NTP)

  WebserverStart();
  setupLoadcell();
  setupServo();
  pinMode(PIN_BUZZER, OUTPUT);

  // // FIXME remove before release
  // servo.write(HMConfig::instance().servodata.angle_max);
  // delay(2000);
  // servo.write(HMConfig::instance().servodata.angle_min);

  log_i("%s", HMConfig::instance().beekeeping);
  startSystemTasks();
  log_i("Setup done! FreeRTOS tasks started ... ");
}

float weight_current = 0;

void loop() {
  static unsigned long lastSocketSend = 0;
  // Gewicht wird jetzt im sensorTask gelesen und via Queue aktualisiert
  // WebSocket nur alle 1000ms senden (weniger frequent für AP)
  if (millis() - lastSocketSend > 1000) {
    HMConfig::instance().weight_current =
        weight_current;  // ensure latest cached value
    sendSocketData();    // still executed in loop context
    lastSocketSend = millis();
  }
  // Servo Logik läuft jetzt im servoTask
  // KRITISCH für AP-Modus: Mehr Zeit für WiFi/WebServer!
  yield();
  delay(5);  // kleine Pause
}

#endif
