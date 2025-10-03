/*
 * File: /handleServo.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2024-04-30 12:31
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <handleServo.h>

extern Servo servo;
extern Glass glass;
extern HX711 scale;

bool first_run = true;

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

  // FIXME - Stop button decoupled from HAND_MODE_CLOSED
  //  if (hmcfg.hm == HAND_MODE_CLOSED && hmcfg.run_modus == RUN_MODUS_AUTO) {
  //    servo.write(hmcfg.servodata.angle_min);
  //    log_e("STOP Button in auto mode pressed! [STOP]");
  //    hmcfg.run_modus = RUN_MODUS_STOPPED;
  //    hmcfg.fs = FILLING_STATUS_STOPPED;
  //    return 0;
  //  }

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

    // Nicht-blockierender AutoStart (ersetzt delay(500))
    {
      static bool autostart_pending = false;     // wartet auf Stabilisierung
      static uint32_t autostart_t0 = 0;          // Startzeit des Fensters
      const uint32_t AUTOSTART_SETTLE_MS = 500;  // ehemals delay(500)

      if (hmcfg.fs == FILLING_STATUS_STANDBY) {
        if (!autostart_pending && glass.isAutoStart()) {
          autostart_pending = true;
          autostart_t0 = millis();
        }
        if (autostart_pending) {
          // Abbruch falls Zustand wieder weg
          if (!glass.isAutoStart()) {
            autostart_pending = false;  // Zurücksetzen
          } else if (millis() - autostart_t0 >= AUTOSTART_SETTLE_MS) {
            // Stabil genug – zweite Prüfung
            glass.setScaleUnit(scale.get_units());  // "cool down" Messung
            if (glass.isAutoStart()) {
              glass.setTaraWeight(
                  scale.get_units(5));  // TODO: 5 -> konfigurierbar
              servo.write(hmcfg.servodata.angle_max);
              hmcfg.fs = FILLING_STATUS_OPEN;
              glass.setGlassInWork(true);
              autostart_pending = false;
              log_d("switch to automodus (non-blocking autostart)");
              return 0;
            }
            // Falls zweite Prüfung scheitert -> erneut warten
            autostart_t0 = millis();
          }
        }
      } else {
        // Anderer Status -> aufräumen
        autostart_pending = false;
      }
    }

    if (glass.isFineFull() && hmcfg.fs == FILLING_STATUS_OPEN) {
      servo.write(hmcfg.servodata.angle_fine);
      log_d("Reach fine filling. Weight fine is %d", hmcfg.weight_fine);
      hmcfg.fs = FILLING_STATUS_FINE;
      return 0;
    }

    if (glass.isFull() && hmcfg.fs == FILLING_STATUS_FINE) {
      // TODO - (janusz) Gewichtskorektur
      servo.write(hmcfg.servodata.angle_min);
      log_i("Glass full! :-)))");
      hmcfg.fs = FILLING_STATUS_FOLLOW_UP;
      return 0;
    }

    if (hmcfg.fs == FILLING_STATUS_FOLLOW_UP) {
      // Nicht-blockierende Follow-Up Sequenz (ersetzt delay(5000) + 2x400ms)
      static uint8_t follow_step = 0;
      static uint32_t follow_tRef = 0;
      const uint32_t FOLLOW_WAIT_MS = 5000;  // TODO: konfigurierbar
      const uint32_t BUZZER_GAP_MS = 400;    // TODO: konfigurierbar

      switch (follow_step) {
        case 0:  // Wartephase nach "voll"
          if (first_run) {
            follow_tRef = millis();
            first_run = false;
          }
          if (millis() - follow_tRef >= FOLLOW_WAIT_MS) {
            glass.setFollowUpAdjustment();
            log_e("Piiiiiiiiiip (1)");
            tone(PIN_BUZZER, 1750, 200);
            follow_tRef = millis();
            follow_step = 1;
          }
          return 0;
        case 1:  // zweiter Ton nach Pause
          if (millis() - follow_tRef >= BUZZER_GAP_MS) {
            log_e("Piiiiiiiiiip (2)");
            tone(PIN_BUZZER, 1750, 200);
            follow_tRef = millis();
            follow_step = 2;
          }
          return 0;
        case 2:  // dritter Ton nach Pause
          if (millis() - follow_tRef >= BUZZER_GAP_MS) {
            log_e("Piiiiiiiiiip (3)");
            tone(PIN_BUZZER, 1750, 200);
            follow_step = 3;
          }
          return 0;
        case 3:  // Abschluss
          hmcfg.fs = FILLING_STATUS_CLOSED;
          follow_step = 0;
          first_run = true;
          return 0;
      }
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
      // log_e("Closed filling");
      servo.write(hmcfg.servodata.angle_min);
    } else if (hmcfg.hm == HAND_MODE_OPEN) {
      // log_e("Open filling");
      servo.write(hmcfg.servodata.angle_max);
    } else if (hmcfg.hm == HAND_MODE_FINE) {
      // log_e("Fine filling");
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
