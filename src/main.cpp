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
 * Last Modified: 2024-11-07 20:50
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef UNIT_TEST

#include <main.h>

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
  setupWifi();

  String temp_date =
      getNTPDate(3600L, 3600L, "pool.ntp.org");  // TODO - move to config
  strlcpy(HMConfig::instance().date_filling, temp_date.c_str(),
          sizeof(HMConfig::instance().date_filling));

  WebserverStart();
  setupLoadcell();
  setupServo();
  pinMode(PIN_BUZZER, OUTPUT);

  // // FIXME remove before release
  // servo.write(HMConfig::instance().servodata.angle_max);
  // delay(2000);
  // servo.write(HMConfig::instance().servodata.angle_min);

  log_i("%s", HMConfig::instance().beekeeping);
  log_i("Setup done! Starting loop ... ");
}

void donothing() {
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

float weight_current = 0;

void loop() {
  static unsigned long lastWeightRead = 0;
  static unsigned long lastSocketSend = 0;

  // Gewicht nur alle 300ms lesen (noch seltener für AP-Modus)
  if (millis() - lastWeightRead > 300) {
    weight_current = scale.get_units(1);  // Nur 1x lesen statt 3x
    HMConfig::instance().weight_current = weight_current;
    lastWeightRead = millis();
  }

  // WebSocket nur alle 1000ms senden (weniger frequent für AP)
  if (millis() - lastSocketSend > 1000) {
    sendSocketData();
    lastSocketSend = millis();
  }

  handleWeightAndServo(weight_current);

  // KRITISCH für AP-Modus: Mehr Zeit für WiFi/WebServer!
  yield();
  delay(10);  // Zusätzliche Pause für Core 0 Tasks
}

#endif
