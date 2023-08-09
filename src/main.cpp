#include <Arduino.h>
#include "esp_log.h"
#include <ESPFS.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  log_i("Start Setup");
  if (!ESPFSInit()) {
    delay(10000);
    ESP.restart();
  }
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

void loop() {
  donothing();
}

