/*
 * File: /Glass.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/Glass
 * Description:
 * -----
 * Created Date: 2023-08-22 23:24
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-09-02 01:36
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <Glass.h>

Glass::Glass() { this->reset(); }
void Glass::reset() {
  HMConfig& hmcfg = HMConfig::instance();
  _config_weight_filling = hmcfg.weight_filling;
  _config_weight_servo_fine = hmcfg.weight_fine;
  _config_glass_tolerance = hmcfg.glass_tolerance;
  _config_glass_empty = hmcfg.glass_empty;
  _glass_weight = 0;  //
  _honey_weight = 0;
  _weight_last = 0;
  _is_full = false;
  _is_fine_full = false;
  _is_auto_start = false;
  _is_glass_removed = false;
  _glass_in_work = false;
  _no_glass = true;
}

void Glass::setTaraWeight(uint16_t tara_weight) { _glass_weight = tara_weight; }

void Glass::setScaleUnit(float sunits) {
  //   log_e("sunits=%6.2f", sunits);
  // if (_glass_weight > 10) {
  if (sunits < 10) {
    _honey_weight = 0;
  }
  if (sunits > _glass_weight) {
    _honey_weight = sunits - _glass_weight;
    log_d("_honey_weight=%d", _honey_weight);
  }

  if (sunits > _config_glass_empty - _config_glass_tolerance) {
    _no_glass = false;
  } else {
    _no_glass = true;
  }

  if (_honey_weight >= _config_weight_filling - corr) {
    log_d("_is_full Honig: %d HonigMax: %d Korr: %d Sum: %d", _honey_weight,
          _config_weight_filling, corr, _config_weight_filling - corr);
    // Serial.printf("_is_full    %6.2f     sunits", sunits);
    _is_full = true;
  } else {
    _is_full = false;
  }

  if (_honey_weight >= _config_weight_servo_fine &&
      (_honey_weight < _config_weight_filling)) {
    log_d("_is_fine_full");
    // Serial.printf("\r _is_fine_full      %6.2f sunits", sunits);
    _is_fine_full = true;
  } else {
    _is_fine_full = false;
  }

  if (sunits > _config_glass_empty - _config_glass_tolerance &&
      sunits < _config_glass_empty + _config_glass_tolerance) {
    // log_d("sunits=%f", sunits);
    // log_d("_config_glass_empty=%d", _config_glass_empty);
    // log_d("_config_glass_tolerance=%d", _config_glass_tolerance);
    log_d("_is_auto_start");
    // Serial.printf("\r _is_auto_start     %6.2f sunits", sunits);
    _is_auto_start = true;
  } else {
    _is_auto_start = false;
  }

  if (sunits >
      _config_glass_empty + _config_glass_tolerance + 5) {  // TODO - replace 5
    // Serial.printf("_glass_in_work %6.2f  sunits", sunits);
    log_d("_glass_in_work");
    _glass_in_work = true;
  } else {
    _glass_in_work = false;
  }

  // if (_weight_last > 50 && sunits < _weight_last - _weight_last / 10) {
  //   log_d("_weight_last=%d", _weight_last);
  //   log_d("_honey_weight=%d", _honey_weight);
  //   log_d("_is_glass_removed");
  //   _is_glass_removed = true;
  // }
  // _weight_last = sunits;

  if (sunits < _config_glass_empty * 0.8 &&
      (HMConfig::instance().fs == FILLING_STATUS_OPEN ||
       HMConfig::instance().fs == FILLING_STATUS_FINE)) {
    // log_d("sunits=%f", sunits);
    // log_d("_config_glass_empty=%d", _config_glass_empty);
    log_d("_is_glass_removed");
    // Serial.printf("\r _is_glass_removed  %6.2f sunits", sunits);
    _is_glass_removed = true;
  } else {
    _is_glass_removed = false;
  }
}
