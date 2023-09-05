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
 * Last Modified: 2023-09-05 13:31
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <main.h>

ESPFS espfs;
HX711 scale;
Servo servo;
Glass glass;
DNSServer dns;
Ticker ticker;

void setup() {
  Serial.begin(115200);
  delay(1000);
  HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
  log_i("Start Setup");
  espfs.setup();
  setupWifiManager();
  WebserverStart();
  setupLoadcell();
  setupServo();
  // FIXME remove before release
  servo.write(HMConfig::instance().servodata.angle_max);
  delay(2000);
  servo.write(HMConfig::instance().servodata.angle_min);

  log_i("%s", HMConfig::instance().beekeeping.c_str());
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
  //   // donothing();
  // show_scale_data();
  weight_current = scale.get_units(LOADCELL_READ_TIMES);
  HMConfig::instance().weight_current = weight_current;
  sendSocketData();
  // weight2seriell(weight_current);
  handleWeightAndServo(weight_current);
}
