/*
 * File: /handleServo.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-23 20:23
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 - 2025 Johannes Arlt (devcpu) Berlin, Germany
 */

#include <MQTTHelper.h>
#include <handleServo.h>
#include <loadcell.h>  // for safeGetUnits

#include "persistence.h"

extern Servo servo;
extern Glass glass;
extern HX711 scale;

// Forward declare buzzer enqueue (provided by cooperative_loop.cpp)
bool enqueueBuzzerPattern(uint16_t freq, uint16_t durationMs, uint8_t count,
                          uint16_t gapMs, uint32_t initialDelay = 0);
bool emitGlassCount(uint32_t gc);

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
  static uint32_t lastHeartbeat = 0;       // periodic heartbeat

  if (hmcfg.run_modus == RUN_MODUS_AUTO) {
    uint32_t now = millis();
    if (now - lastHeartbeat >= 1000) {
      lastHeartbeat = now;
      log_d("HB auto fs=%d emg=%d w=%.1f", hmcfg.fs, hmcfg.emergency_stop,
            weight_scale_brutto);
    }
  }

  // Highest priority: emergency stop. One-shot clear after action.
  if (hmcfg.emergency_stop) {
    servo.write(hmcfg.servodata.angle_min);
    hmcfg.run_modus = RUN_MODUS_STOPPED;
    hmcfg.fs = FILLING_STATUS_STOPPED;
    hmcfg.hm = HAND_MODE_CLOSED;
    hmcfg.emergency_stop = false;
    log_w("hmcfg.emergency_stop");
    return 0;
  }

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
    static uint32_t openStateEnterMs = 0;          // when we entered OPEN/FINE
    static bool openWatchArmed = false;            // track open watchdog
    static float openStateWeightRef = 0.0f;        // reference weight at OPEN
    const uint32_t OPEN_MAX_DURATION_MS = 180000;  // 180s max open
    const uint32_t OPEN_MIN_PROGRESS_MS = 10000;   // after 10s expect progress
    const float OPEN_PROGRESS_DELTA = 2.0f;        // grams minimal increase
    // Emergency stop check (async from ISR or WebSocket)
    if (hmcfg.emergency_stop) {
      servo.write(hmcfg.servodata.angle_min);
      hmcfg.run_modus = RUN_MODUS_STOPPED;
      hmcfg.fs = FILLING_STATUS_STOPPED;
      hmcfg.hm = HAND_MODE_CLOSED;
      hmcfg.emergency_stop = false;
      log_w("hmcfg.emergency_stop");
      return 0;  // RET_EM_FAST
    }
    if (glass.isGlassRemoved() &&
        (hmcfg.fs == FILLING_STATUS_FINE || hmcfg.fs == FILLING_STATUS_OPEN)) {
      log_w("SERVO CLOSE: Glass removed w=%.1f fs=%d", weight_scale_brutto,
            hmcfg.fs);
      servo.write(hmcfg.servodata.angle_min);
      hmcfg.fs = FILLING_STATUS_STOPPED;
      hmcfg.run_modus = RUN_MODUS_STOPPED;
      glass.reset();
    }

    if (hmcfg.run_modus == RUN_MODUS_STOPPED) {  // TODO - move up?
      log_i("SERVO CLOSE: RUN_MODUS_STOPPED w=%.1f fs=%d", weight_scale_brutto,
            hmcfg.fs);
      servo.write(hmcfg.servodata.angle_min);
      hmcfg.fs = FILLING_STATUS_STOPPED;
      // log_event(weight_scale_brutto, weight_scale_brutto, hmcfg.fs,
      // hmcfg.run_modus);
      return 0;
    }

    // Simplified non-blocking AutoStart: rely solely on sensorTask updates (no
    // extra HX711 reads here)
    {
      static bool autostart_pending = false;
      static uint32_t autostart_t0 = 0;
      const uint32_t AUTOSTART_SETTLE_MS = 500;  // stable presence window
      if (hmcfg.fs == FILLING_STATUS_STANDBY) {
        if (!autostart_pending && glass.isAutoStart()) {
          autostart_pending = true;
          autostart_t0 = millis();
          log_i("AS:start w=%.1f", weight_scale_brutto);
        }
        if (autostart_pending) {
          if (!glass.isAutoStart()) {
            autostart_pending = false;
            log_i("AS:cancel");
          } else if (millis() - autostart_t0 >= AUTOSTART_SETTLE_MS) {
            // Confirm still present and transition to OPEN using current weight
            // reference
            if (glass.isAutoStart()) {
              glass.setTaraWeight(
                  weight_scale_brutto);  // baseline (may be empty glass + drip)
              servo.write(hmcfg.servodata.angle_max);
              hmcfg.fs = FILLING_STATUS_OPEN;
              glass.setGlassInWork(true);
              autostart_pending = false;
              openStateEnterMs = millis();
              openWatchArmed = true;
              openStateWeightRef = weight_scale_brutto;
              log_i("AS:OPEN wRef=%.1f", openStateWeightRef);
              return 0;  // RET_AS_OPEN
            }
            // Presence lost exactly at expiry: restart timer if re-detected
            // later
            autostart_pending = false;
          }
        }
      } else {
        autostart_pending = false;  // any other state cancels pending
      }
    }

    if (glass.isFineFull() && hmcfg.fs == FILLING_STATUS_OPEN) {
      servo.write(hmcfg.servodata.angle_fine);
      log_i("Reach fine filling. Weight fine is %d honey=%d w=%.1f",
            hmcfg.weight_fine, hmcfg.weight_honey, weight_scale_brutto);
      hmcfg.fs = FILLING_STATUS_FINE;
      openStateEnterMs = millis();
      openStateWeightRef = weight_scale_brutto;
      return 0;
    }

    if (hmcfg.fs == FILLING_STATUS_FINE) {
      // Debug: Log status during fine filling
      static uint32_t lastFineLog = 0;
      if (millis() - lastFineLog >= 500) {
        log_d("FINE filling: honey=%d target=%d isFull=%d w=%.1f",
              hmcfg.weight_honey, hmcfg.weight_filling, glass.isFull(),
              weight_scale_brutto);
        lastFineLog = millis();
      }
    }

    if (glass.isFull() && hmcfg.fs == FILLING_STATUS_FINE) {
      // TODO - (janusz) Gewichtskorektur
      log_i("SERVO CLOSE: Glass full! w=%.1f target=%d honey=%d",
            weight_scale_brutto, hmcfg.weight_filling, hmcfg.weight_honey);
      servo.write(hmcfg.servodata.angle_min);
      log_i("Glass full! :-))");
      hmcfg.fs = FILLING_STATUS_FOLLOW_UP;
      openWatchArmed = false;
      return 0;
    }

    if (hmcfg.fs == FILLING_STATUS_FOLLOW_UP) {
      // Nicht-blockierende Follow-Up Sequenz (ersetzt delay(5000) + 2x400ms)
      static bool followUpActive = false;
      static bool patternQueued = false;
      static bool mqttSent = false;
      const uint32_t FOLLOW_WAIT_MS = 5000;              // TODO: konfigurierbar
      const uint32_t BEEP_PATTERN_MS = 3 * (200 + 400);  // 3x(beep+gap)
      static uint32_t follow_tRef = 0;
      if (!followUpActive) {
        follow_tRef = millis();
        followUpActive = true;
        patternQueued = false;
        mqttSent = false;
      }
      if (!patternQueued && millis() - follow_tRef >= FOLLOW_WAIT_MS) {
        glass.setFollowUpAdjustment();
        // 3 Beeps pattern via buzzer queue (1750Hz, 200ms, 3x, 400ms gap)
        enqueueBuzzerPattern(1750, 200, 3, 400, 0);
        patternQueued = true;
      }
      if (patternQueued) {
        // Send MQTT during beep pattern (non-blocking, uses gaps between beeps)
        if (!mqttSent && millis() - follow_tRef >= FOLLOW_WAIT_MS + 500) {
          // Start MQTT send 500ms after beeps started (during first gap)
          // weight_honey is measured AFTER 5s settle time = actual weight
          log_i("Sending MQTT filling data: actual=%d target=%d count=%u",
                hmcfg.weight_honey, hmcfg.weight_filling, hmcfg.glass_count);
          MQTTHelper::instance().sendFillingData(
              hmcfg.weight_honey,    // actual weight after 5s
              hmcfg.weight_filling,  // configured target weight
              hmcfg.glass_count);
          mqttSent = true;
        }
        // Warten bis Pattern durchgelaufen ist
        if (millis() - follow_tRef >= FOLLOW_WAIT_MS + BEEP_PATTERN_MS + 200) {
          hmcfg.fs = FILLING_STATUS_CLOSED;
          // Increment glass counter persistence hook
          hmcfg.glass_count++;
          emitGlassCount(hmcfg.glass_count);
          // Mark persistence dirty
          Persistence::markGlassCountDirty();
          followUpActive = false;
          patternQueued = false;
        }
      }
      return 0;
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
    // Watchdog für OPEN/FINE: Kein Fortschritt -> Warnung/Timeout
    if (openWatchArmed &&
        (hmcfg.fs == FILLING_STATUS_OPEN || hmcfg.fs == FILLING_STATUS_FINE)) {
      uint32_t dt = millis() - openStateEnterMs;
      float delta = weight_scale_brutto - openStateWeightRef;
      if (dt > OPEN_MIN_PROGRESS_MS && delta < OPEN_PROGRESS_DELTA) {
        log_e("OPEN WD warn dt=%lu d=%.1f", (unsigned long)dt, delta);
      }
      if (dt > OPEN_MAX_DURATION_MS) {
        log_e("OPEN WD timeout dt=%lu d=%.1f -> FORCE CLOSE", (unsigned long)dt,
              delta);
        servo.write(hmcfg.servodata.angle_min);
        hmcfg.run_modus = RUN_MODUS_STOPPED;
        hmcfg.fs = FILLING_STATUS_STOPPED;
        openWatchArmed = false;
        return 0;  // RET_WD_FORCE
      }
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
      log_i("SERVO CLOSE: HAND_MODE_CLOSED w=%.1f", weight_scale_brutto);
      servo.write(hmcfg.servodata.angle_min);
    } else if (hmcfg.hm == HAND_MODE_OPEN) {
      // log_e("Open filling");
      servo.write(hmcfg.servodata.angle_max);
    } else if (hmcfg.hm == HAND_MODE_FINE) {
      // log_e("Fine filling");
      servo.write(hmcfg.servodata.angle_fine);
    } else {
      log_w("No catch found! hmcfg.hm=%d", hmcfg.hm);
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
    log_d("hmcfg.run_modus == RUN_MODUS_TEST");
    log_d("weight_scale_brutto=%d", weight_scale_brutto);
    servo.write(hmcfg.servodata.angle_test);
  }

  return 1;
}
