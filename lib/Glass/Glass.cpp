/*
 * File: /Glass.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/Glass
 * Description:
 * -----
 * Created Date: 2023-08-22 23:24
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2024-04-27 00:21
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <Glass.h>

Glass::Glass() { this->reset(); }

/**
 * Resets all values
 */
void Glass::reset() {
  HMConfig& hmcfg = HMConfig::instance();
  _config_weight_filling = hmcfg.weight_filling;
  _config_weight_servo_fine = hmcfg.weight_fine;
  _config_glass_tolerance = hmcfg.glass_tolerance;
  _config_glass_empty = hmcfg.glass_empty;
  _glass_weight = 0;  //
  _honey_weight = 0;
  // _weight_last = 0;
  _is_full = false;
  _is_fine_full = false;
  _is_auto_start = false;
  _is_glass_removed = false;
  _glass_in_work = false;
  _no_glass = true;
}

void Glass::setTaraWeight(uint16_t tara_weight) { _glass_weight = tara_weight; }

/**
 * This function is allways called from main loop and handle most processing
 * and sets lot of imported values.
 */
void Glass::setScaleUnit(float sunits) {
  //   log_e("sunits=%6.2f", sunits);
  // if (_glass_weight > 10) {

  if (sunits < 10) {
    _honey_weight = 0;
  }

  // set hony weight
  if (sunits > _glass_weight) {
    _honey_weight = sunits - _glass_weight;
    // log_d("_honey_weight=%d", _honey_weight);
  }

  // glass on scale?
  if (sunits > _config_glass_empty - _config_glass_tolerance) {
    _no_glass = false;
  } else {
    _no_glass = true;
  }

  // glass full?
  if (_honey_weight >= _config_weight_filling - follow_up_adjustment) {
    log_d("_is_full Honig!: %d HonigMax: %d Korr: %d Sum: %d", _honey_weight,
          _config_weight_filling, follow_up_adjustment,
          _config_weight_filling - follow_up_adjustment);
    // Serial.printf("_is_full    %6.2f     sunits", sunits);
    _is_full = true;
  } else {
    _is_full = false;
  }

  // filled until fine fill?
  if (_honey_weight >= _config_weight_servo_fine &&
      (_honey_weight < _config_weight_filling)) {
    log_d("_is_fine_full");
    // Serial.printf("\r _is_fine_full      %6.2f sunits", sunits);
    _is_fine_full = true;
  } else {
    _is_fine_full = false;
  }

  // auto start
  // TODO config autostart
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

  // FIXME? diff _is_auto_start and _glass_in_work?
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

  // auto emergency stop, if glass is removed
  if (sunits < _config_glass_empty * 0.8 &&
      (HMConfig::instance().fs == FILLING_STATUS_OPEN ||  // FIXME hmcfg?
       HMConfig::instance().fs == FILLING_STATUS_FINE)) {
    // log_d("sunits=%f", sunits);
    // log_d("_config_glass_empty=%d", _config_glass_empty);
    log_d("_is_glass_removed");
    // Serial.printf("\r _is_glass_removed  %6.2f sunits", sunits);
    _is_glass_removed = true;
  } else {
    _is_glass_removed = false;
  }

  HMConfig::instance().weight_honey = _honey_weight;
  // log_d("weight_honey=%d", HMConfig::instance().weight_honey);
}
