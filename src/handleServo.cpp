/*
 * File: /handleServo.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-22 18:55
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <handleServo.h>

typedef enum {
  FILLING_STATUS_OPEN,
  FILLING_STATUS_FINE,
  FILLING_STATUS_CLOSED,
} FillingStatus;

Servo servo;

void setupServo() {
  log_d("Init Servo ...");
  HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
  servo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  servo.setPeriodHertz(SERVO_FREQUENCY);
  log_d("Winkel=%d", 0);
  servo.write(0);
}

int handleWeightAndServo(float weight_current) {
  static FillingStatus fs = FILLING_STATUS_CLOSED;
  HMConfig& hmcfg = HMConfig::instance();

  if (hmcfg.run_modus == RUN_MODUS_STOPPED) {
    servo.write(hmcfg.angle_min);
    fs = FILLING_STATUS_CLOSED;
    return 0;
  }

  if (weight_current >= hmcfg.weight_filling &&
      fs != FILLING_STATUS_FINE) {  // FIXME(janusz) Gewichtskorrektur
    servo.write(hmcfg.angle_min);
    log_e("Panic close! This should never happens!");
    fs = FILLING_STATUS_CLOSED;
    return 0;
  }

  if (weight_current >= hmcfg.weight_fine) {
    servo.write(hmcfg.angle_fine);
    log_d("Reach fine filling.");
    fs = FILLING_STATUS_FINE;
    return 0;
  }

  if (hmcfg.run_modus == RUN_MODUS_AUTO) {
    if (weight_current > hmcfg.weight_empty - hmcfg.glass_tolerance &&
        weight_current < hmcfg.weight_empty + hmcfg.glass_tolerance) {
      servo.write(hmcfg.angle_max);
      return 0;
    }
  }

  // log_e("No catch found! RunModus=%d, FillingStatus=%d, Weight=%f",
  // hmcfg.run_modus, fs, weight_current);

  return 1;
}
