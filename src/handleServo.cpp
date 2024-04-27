/*
 * File: /handleServo.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2024-04-27 04:00
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <handleServo.h>

extern Servo servo;
extern Glass glass;
extern HX711 scale;

void setupServo() {
  log_d("Init Servo ...");
  HMConfig::instance().run_modus = RUN_MODUS_STOPPED;
  servo.attach(PIN_SERVO, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  servo.setPeriodHertz(SERVO_FREQUENCY);
  log_d("Winkel=%d", 0);
  servo.write(0);
}

int handleWeightAndServo(float weight_scale_brutto) {
  HMConfig& hmcfg = HMConfig::instance();  // TODO - got pointer?
  //   log_d("automatic=%s fs=%s", hmcfg.runmod2string(hmcfg.run_modus).c_str(),
  //         hmcfg.fillingstatus2string(hmcfg.fs).c_str());

  if (hmcfg.run_modus == RUN_MODUS_AUTO &&
      (hmcfg.fs == FILLING_STATUS_CLOSED ||
       hmcfg.fs == FILLING_STATUS_STOPPED) &&
      glass.isNoGlass()) {
    hmcfg.fs = FILLING_STATUS_STANDBY;
  }
  glass.setScaleUnit(weight_scale_brutto);

  /* ----------------------------- AUTOMATIK BEGIN ----------------------------
   */

  if (hmcfg.run_modus == RUN_MODUS_AUTO) {
    if (glass.isGlassRemoved() &&
        (hmcfg.fs == FILLING_STATUS_FINE || hmcfg.fs == FILLING_STATUS_OPEN)) {
      servo.write(hmcfg.servodata.angle_min);
      hmcfg.fs = FILLING_STATUS_STOPPED;
      log_e("Glass remove detected! STOP");
      hmcfg.run_modus = RUN_MODUS_STOPPED;
      glass.reset();
    }

    if (hmcfg.run_modus == RUN_MODUS_STOPPED) {  // TODO - move up?
      servo.write(hmcfg.servodata.angle_min);
      hmcfg.fs = FILLING_STATUS_STOPPED;
      log_e("hmcfg.fs = FILLING_STATUS_CLOSED");
      // log_event(weight_scale_brutto, weight_scale_brutto, hmcfg.fs,
      // hmcfg.run_modus);
      return 0;
    }

    // starts if automodus and new empty glass
    if (glass.isAutoStart() && hmcfg.fs == FILLING_STATUS_STANDBY) {
      delay(500);
      glass.setScaleUnit(scale.get_units());      // cool down
      if (glass.isAutoStart()) {                  // 2nd check
        glass.setTaraWeight(scale.get_units(5));  // TODO - 5 makes delay
        servo.write(hmcfg.servodata.angle_max);
        hmcfg.fs = FILLING_STATUS_OPEN;
        glass.setGlassInWork(true);
        log_d("switch to automodus");
        return 0;
      }
    }

    if (glass.isFineFull() && hmcfg.fs == FILLING_STATUS_OPEN) {
      servo.write(hmcfg.servodata.angle_fine);
      log_d("Reach fine filling. Weight fine is %d", hmcfg.weight_fine);
      hmcfg.fs = FILLING_STATUS_FINE;
      return 0;
    }

    if (glass.isFull() && hmcfg.fs == FILLING_STATUS_FINE) {
      // TODO(janusz) Gewichtskorektur
      servo.write(hmcfg.servodata.angle_min);
      log_i("Glass full! :-)))");
      hmcfg.fs = FILLING_STATUS_FOLLOW_UP;
      return 0;
    }

    if (hmcfg.fs == FILLING_STATUS_FOLLOW_UP) {
      delay(5000);  // FIXME config var instand fix!
      glass.setFollowUpAdjustment();
      log_i("Piiiiiiiiiip");
      tone(PIN_BUZZER, 1750, 200);
      delay(400);
      log_i("Piiiiiiiiiip");
      tone(PIN_BUZZER, 1750, 200);
      delay(400);
      log_i("Piiiiiiiiiip");
      tone(PIN_BUZZER, 1750, 200);
      // delay(400);
      hmcfg.fs = FILLING_STATUS_CLOSED;
    }

    if (glass.isFull() &&
        (hmcfg.fs == FILLING_STATUS_OPEN ||
         hmcfg.fs == FILLING_STATUS_FINE)) {  // TODO - move up?
      servo.write(hmcfg.servodata.angle_min);
      log_e("Panic close! This should never happens!");
      hmcfg.run_modus = RUN_MODUS_STOPPED;
      hmcfg.fs = FILLING_STATUS_STOPPED;
      return 0;
    }
    // log_e("no if catching");
  }
  /* ------------------------------ AUTOMATIK END -----------------------------
   */

  /* --------------------------------------------------------------------------
   */
  /* ------------------------------- HAND BEGIN -------------------------------
   */
  /* --------------------------------------------------------------------------
   */
  else if (hmcfg.run_modus == RUN_MODUS_HAND) {
    if (hmcfg.hm == HAND_MODE_CLOSED) {
      log_e("Closed filling");
      servo.write(hmcfg.servodata.angle_min);
    } else if (hmcfg.hm == HAND_MODE_OPEN) {
      log_e("Open filling");
      servo.write(hmcfg.servodata.angle_max);
    } else if (hmcfg.hm == HAND_MODE_FINE) {
      log_e("Fine filling");
      servo.write(hmcfg.servodata.angle_fine);
    } else {
      log_e("No catch found! hmcfg.hm=%d", hmcfg.hm);
      log_d("\n");
    }
    // servo.write(hmcfg.servodata.angle_max);
  }

  /* -------------------------------- HAND END --------------------------------
   */

  // log_e("No catch found! RunModus=%s, FillingStatus=%s, Weight=%f,
  // weight_glass_netto=%d", runmod2string(hmcfg.run_modus).c_str(),
  // fillingstatus2string(hmcfg.fs).c_str(), weight_scale_brutto,
  // weight_glass_netto); log_d("\n");

  else if (hmcfg.run_modus == RUN_MODUS_TEST) {
    log_e("hmcfg.run_modus == RUN_MODUS_TEST");
    log_e("weight_scale_brutto=%d", weight_scale_brutto);
    servo.write(hmcfg.servodata.angle_test);
  }

  return 1;
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
//   log_d("hmcfg.glass_empty %d", hmcfg.glass_empty);
//   log_d("hmcfg.glass_tolerance %d", hmcfg.glass_tolerance);
//   log_d("glass_empty_current %d", hmcfg.glass_tara_weight);
//   log_d("weight_last %d", weight_last);

//   return 0;
// }

void glass_full() {}
