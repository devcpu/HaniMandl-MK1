/*
 * File: /handleServo.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-23 02:13
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <handleServo.h>

Servo servo;
Glass glass;
extern HX711 scale;

void setupServo() {
  log_d("Init Servo ...");
  HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
  servo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  servo.setPeriodHertz(SERVO_FREQUENCY);
  log_d("Winkel=%d", 0);
  servo.write(0);
}

int handleWeightAndServo(float weight_scale_brutto) {
  static FillingStatus fs = FILLING_STATUS_CLOSED;
  HMConfig& hmcfg = HMConfig::instance();
  glass.setScaleUnit(weight_scale_brutto);

  if (glass.isGlassRemoved() && fs != FILLING_STATUS_CLOSED) {
    servo.write(hmcfg.servodata.angle_min);
    fs = FILLING_STATUS_CLOSED;
    log_e("Glass remove detected! STOP");
    hmcfg.run_modus = RUN_MODUS_STOPPED;
    glass.reset();
  }

  if (hmcfg.run_modus == RUN_MODUS_STOPPED) {
    servo.write(hmcfg.servodata.angle_min);
    fs = FILLING_STATUS_CLOSED;
    log_e("fs = FILLING_STATUS_CLOSED");
    // log_event(weight_scale_brutto, weight_scale_brutto, fs, hmcfg.run_modus);
    return 0;
  }

  // starts if automodus and new empty glass
  if (hmcfg.run_modus == RUN_MODUS_AUTO && fs == FILLING_STATUS_CLOSED) {
    if (glass.isAutoStart()) {
      delay(500);
      glass.setTaraWeight(scale.get_units(5));
      servo.write(hmcfg.servodata.angle_max);
      fs = FILLING_STATUS_OPEN;
      log_d("switch to automodus");
      return 0;
    }
  }

  if (glass.isFineFull() && fs == FILLING_STATUS_OPEN) {
    servo.write(hmcfg.servodata.angle_fine);
    log_d("Reach fine filling. Weight fine is %d", hmcfg.weight_fine);
    fs = FILLING_STATUS_FINE;
    return 0;
  }

  if (glass.isFull() &&  // TODO(janusz) Gewichtskorektur
      fs == FILLING_STATUS_FINE) {
    servo.write(hmcfg.servodata.angle_min);
    log_i("Glass full! :-)))");
    fs = FILLING_STATUS_CLOSED;
    return 0;
  }

  if (glass.isFull() && fs == FILLING_STATUS_OPEN) {
    servo.write(hmcfg.servodata.angle_min);
    log_e("Panic close! This should never happens!");
    hmcfg.run_modus = RUN_MODUS_STOPPED;
    fs = FILLING_STATUS_CLOSED;
    return 0;
  }

  // log_e("No catch found! RunModus=%s, FillingStatus=%s, Weight=%f,
  // weight_glass_netto=%d", runmod2string(hmcfg.run_modus).c_str(),
  // fillingstatus2string(fs).c_str(), weight_scale_brutto, weight_glass_netto);
  // log_d("\n");
  return 1;
}

String runmod2string(RunModus mod) {
  switch (mod) {
    case RUN_MODUS_AUTO:
      return String("RUN_MODUS_AUTO");
    case RUN_MODUS_HAND:
      return String("RUN_MODUS_HAND");
    case RUN_MODUS_STOPPED:
      return String("RUN_MODUS_STOPPED");
    default:
      return String("Unknown modus");
  }
}

String fillingstatus2string(FillingStatus status) {
  switch (status) {
    case FILLING_STATUS_FINE:
      return String("FILLING_STATUS_FINE");
    case FILLING_STATUS_OPEN:
      return String("FILLING_STATUS_OPEN");
    case FILLING_STATUS_CLOSED:
      return String("FILLING_STATUS_CLOSED");
    default:
      return String("unknown filling status");
  }
}

// int log_event(float weight_scale_brutto, int16_t weight_glass_netto,
// FillingStatus status, uint16_t weight_last){
//   HMConfig& hmcfg = HMConfig::instance();
//   log_d("weight_scale_brutto: %f", weight_scale_brutto);
//   log_d("weight_glass_netto %d", weight_glass_netto);
//   log_d("FillingStatus %s", fillingstatus2string(status).c_str());
//   log_d("RunModus %s", runmod2string(hmcfg.run_modus).c_str());
//   log_d("hmcfg.weight_filling %d", hmcfg.weight_filling);
//   log_d("hmcfg.weight_fine %d", hmcfg.weight_fine);
//   log_d("hmcfg.weight_empty %d", hmcfg.weight_empty);
//   log_d("hmcfg.glass_tolerance %d", hmcfg.glass_tolerance);
//   log_d("glass_empty_current %d", hmcfg.glass_tara_weight);
//   log_d("weight_last %d", weight_last);

//   return 0;
// }

void glass_full() {}
