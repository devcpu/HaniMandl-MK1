/*
 * File: /loadcell.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-22 16:27
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-03 17:34
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 - 2025 Johannes Arlt (devcpu) Berlin, Germany
 */

#include <loadcell.h>
#include <math.h>  // for fabs

extern HX711 scale;

// Guarded wait ensuring we don't block indefinitely on a dead HX711.
bool waitForScaleReady(uint32_t timeoutMs) {
  uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {
    if (scale.is_ready()) return true;
    delay(1);  // yield
  }
  log_w("[HX711] waitForScaleReady timeout after %lu ms",
        (unsigned long)timeoutMs);
  return false;
}

long safeGetValue(uint8_t times, uint32_t timeoutMs) {
  if (!waitForScaleReady(timeoutMs)) return 0;  // keep 0 semantics
  return scale.get_value(times);
}

float safeGetUnits(uint8_t times, uint32_t timeoutMs) {
  if (!waitForScaleReady(timeoutMs)) return NAN;
  return scale.get_units(times);
}

void setupLoadcell() {
  scale.begin(PIN_LOADCELL_DOUT, PIN_LOADCELL_SCK);
  log_d("begin");
  show_scale_data();
  scale.set_gain(128);
  scale.power_up();
  // Apply stored calibration non-blocking.
  // Previous logic required both SCALE!=0 and OFFSET!=0; but a valid tare can
  // have OFFSET==0 or a SCALE very close to 1 after calibration. Use epsilon
  // check and apply if either differs.
  double s = HMConfig::instance().SCALE;
  long o = HMConfig::instance().OFFSET;
  // CAL_EPS: Minimal Abweichungsschwelle für die Entscheidung, ob eine
  // gespeicherte SCALE vom Default (1.0) abweicht. Name bewusst NICHT "EPS",
  // weil das Xtensa low-level Header (specreg.h) ein Macro EPS definiert
  // (#define EPS 192). Ein direkter Bezeichner EPS würde deshalb zu einem
  // Präprozessor-Konflikt führen.
  static constexpr double CAL_EPS = 1e-6;
  bool haveScale = fabs(s - 1.0) > CAL_EPS;
  bool haveOffset = (o != 0);
  if (haveScale || haveOffset) {
    scale.set_scale(s == 0 ? 1.0 : s);
    scale.set_offset(o);
    log_i(
        "[Loadcell] Calibration loaded: SCALE=%f OFFSET=%ld (haveScale=%d "
        "haveOffset=%d)",
        s, (long)o, haveScale, haveOffset);
  } else {
    scale.set_scale(1.0f);
    scale.set_offset(0);
    log_w(
        "[Loadcell] No stored calibration detected (SCALE≈1 & OFFSET=0); use "
        "/calibrate");
  }
  log_d("end");
}

void show_scale_data() {
  // Diagnostic output (kept minimal to avoid unused-variable warnings under
  // -Werror)
  Serial.println("------------   show_scale_data()   ------------");
  Serial.printf("read(): %ld\n", scale.read());
  uint32_t t_start = millis();
  int32_t ra = scale.read_average(LOADCELL_READ_TIMES);
  uint32_t duration = millis() - t_start;
  Serial.printf("SCALE: %f\n", scale.get_scale());
  Serial.printf("OFFSET: %ld\n", (long)scale.get_offset());
  Serial.printf("read_average(%d): %d in %lu ms\n", LOADCELL_READ_TIMES, ra,
                (unsigned long)duration);
  Serial.printf("get_value(%d): %ld\n", LOADCELL_READ_TIMES,
                safeGetValue(LOADCELL_READ_TIMES));
  Serial.printf("get_units(%d): %f\n", LOADCELL_READ_TIMES,
                safeGetUnits(LOADCELL_READ_TIMES));
  Serial.printf("scale.get_value(%d) = %ld\n", LOADCELL_READ_TIMES,
                safeGetValue(LOADCELL_READ_TIMES));
  float uShow = safeGetUnits(LOADCELL_READ_TIMES);
  if (!isnan(uShow)) {
    Serial.printf("Result %.0fg\n\n", round(uShow));
    Serial.printf("%4d g\n", static_cast<int>(round(uShow)));
  } else {
    Serial.println("[HX711] units read timeout in show_scale_data()");
  }
}

void weight2seriell() {
  float u = safeGetUnits(LOADCELL_READ_TIMES);
  if (!isnan(u)) weight2seriell(u);
}

void weight2seriell(float weight_current) {
  for (uint8_t i = 0; i <= 30; i++) {
    Serial.print("\r            \r");  // FIXME -
  }
  Serial.printf("%4d g", static_cast<int>(round(weight_current)));
}
