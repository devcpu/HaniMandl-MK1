/*
 * File: /Glass.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-22 23:24
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 - 2025 Johannes Arlt (devcpu) Berlin, Germany
 */

#include <Glass.h>

Glass::Glass() { this->reset(); }

/**
 * Resets all values
 */
void Glass::reset() {
  _glass_weight = 0;  //
  _honey_in_glass_weight_filled = 0;
  _is_full = false;
  _is_fine_full = false;
  _is_auto_start = false;
  _is_glass_removed = false;
  _glass_in_work = false;
  _no_glass = true;
  // Always update cutoff_weight from current config on reset
  cutoff_weight = HMConfig::instance().weight_filling;
}

void Glass::setTaraWeight(uint16_t tara_weight) { _glass_weight = tara_weight; }

/**
 * This function is allways called from main loop and handle most processing
 * and sets lot of imported values.
 */
void Glass::setScaleUnit(float sunits) {
  //   log_e("sunits=%6.2f", sunits);
  // if (_glass_weight > 10) {

  // _logFillingData();

  if (sunits < 10) {
    _honey_in_glass_weight_filled = 0;
  }

  // set hony weight
  if (sunits > _glass_weight) {
    _honey_in_glass_weight_filled = sunits - _glass_weight;
    // log_d("_honey_weight=%d", _honey_weight);
  }

  // glass on scale?
  if (sunits >
      HMConfig::instance().glass_empty - HMConfig::instance().glass_tolerance) {
    _no_glass = false;
  } else {
    _no_glass = true;
  }

  // glass full?
  if (_honey_in_glass_weight_filled >= cutoff_weight) {
    log_e("Glass FULL check: honey=%d cutoff=%d -> TRUE",
          _honey_in_glass_weight_filled, cutoff_weight);
    _is_full = true;
  } else {
    // Log periodically when NOT full during filling
    static uint32_t lastNotFullLog = 0;
    if (HMConfig::instance().fs == FILLING_STATUS_FINE &&
        millis() - lastNotFullLog >= 1000) {
      log_e("Glass NOT full: honey=%d cutoff=%d glassTara=%d sunits=%.1f",
            _honey_in_glass_weight_filled, cutoff_weight, _glass_weight,
            sunits);
      lastNotFullLog = millis();
    }
    _is_full = false;
  }

  // filled until fine fill?
  if (_honey_in_glass_weight_filled >= HMConfig::instance().weight_fine &&
      (_honey_in_glass_weight_filled < HMConfig::instance().weight_filling)) {
    // log_d("_is_fine_full");
    // Serial.printf("\r _is_fine_full      %6.2f sunits", sunits);
    _is_fine_full = true;
  } else {
    _is_fine_full = false;
  }

  // auto start
  // TODO config autostart
  if (sunits > HMConfig::instance().glass_empty -
                   HMConfig::instance().glass_tolerance &&
      sunits < HMConfig::instance().glass_empty +
                   HMConfig::instance().glass_tolerance) {
    // log_d("sunits=%f", sunits);
    // log_d("HMConfig::instance().glass_empty=%d",
    // HMConfig::instance().glass_empty);
    // log_d("HMConfig::instance().glass_tolerance=%d",
    // HMConfig::instance().glass_tolerance); log_d("_is_auto_start");
    // Serial.printf("\r _is_auto_start     %6.2f sunits", sunits);
    _is_auto_start = true;
  } else {
    _is_auto_start = false;
  }

  // FIXME? diff _is_auto_start and _glass_in_work?
  if (sunits > HMConfig::instance().glass_empty +
                   HMConfig::instance().glass_tolerance +
                   5) {  // TODO - replace 5
    // Serial.printf("_glass_in_work %6.2f  sunits", sunits);
    // log_d("_glass_in_work");
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
  if (sunits < HMConfig::instance().glass_empty * 0.8 &&
      (HMConfig::instance().fs == FILLING_STATUS_OPEN ||  // FIXME hmcfg?
       HMConfig::instance().fs == FILLING_STATUS_FINE)) {
    // log_d("sunits=%f", sunits);
    // log_d("HMConfig::instance().glass_empty=%d",
    // HMConfig::instance().glass_empty);
    log_d("_is_glass_removed");
    // Serial.printf("\r _is_glass_removed  %6.2f sunits", sunits);
    _is_glass_removed = true;
  } else {
    _is_glass_removed = false;
  }

  HMConfig::instance().weight_honey = _honey_in_glass_weight_filled;
  // log_d("weight_honey=%d", HMConfig::instance().weight_honey);
}

void Glass::setFollowUpAdjustment() {
  _follow_up_adjustment =
      _honey_in_glass_weight_filled - HMConfig::instance().weight_filling;
  cutoff_weight = cutoff_weight - _follow_up_adjustment;  // FIXME - + or - ?
  log_e("FollowUp adjustment: honey=%d target=%d adj=%d NEW_cutoff=%d",
        _honey_in_glass_weight_filled, HMConfig::instance().weight_filling,
        _follow_up_adjustment, cutoff_weight);
  _logFillingData();
  _is_full = true;
}

void Glass::_logFillingData() {
  log_d(
      "\n\t[_honey_weight (filled)=%d]"
      "\n\t[HMConfig.weight_filling (target)=%d]"
      "\n\t[weight_fine=%d]"
      "\n\t[_cutoff_weight=%d]"
      "\n\t[follow_up_adjustment=%d]",
      _honey_in_glass_weight_filled, HMConfig::instance().weight_filling,
      HMConfig::instance().weight_fine, cutoff_weight, _follow_up_adjustment);
}
