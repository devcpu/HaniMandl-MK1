/*
 * File: /Glass.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/Glass
 * Description:
 * -----
 * Created Date: 2023-08-22 23:24
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-23 19:01
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#ifndef LIB_GLASS_GLASS_H_
#define LIB_GLASS_GLASS_H_

#include <Arduino.h>
#include <HMConfig.h>

class Glass {
 public:
  Glass();
  /// @brief gets units from scale and calc all value in class
  /// @param sunits units from scale
  void setScaleUnit(float sunits);

  /// @brief sets tara weight of this glass
  /// @param tara_weight
  void setTaraWeight(uint16_t tara_weight);

  /// @return true if glass is full
  bool isFull() { return _is_full; }

  /// @return true if glass is filled until fine fill weight
  bool isFineFull() { return _is_fine_full; }

  /// @brief Sets all values back (instead of making a new one)
  void reset();

  /// @brief true if Glass removed from scale
  bool isGlassRemoved() { return _is_glass_removed; }

  bool isAutoStart() { return _is_auto_start; }

  // has to set to true if filling starts
  void setGlassInWork(bool status = true) { _glass_in_work = status; }

  uint8_t corr_conter = 0;
  uint16_t corr = 0;

 private:
  bool _is_auto_start = false;
  bool _is_glass_removed = false;
  bool _is_fine_full = false;
  bool _is_full = false;
  bool _glass_in_work = false;
  uint16_t _weight_last = 0;
  uint16_t _config_glass_empty = 0;

  /// @brief weight of this glass (empty)
  uint16_t _glass_weight = 0;

  /// @brief toleranc for automatic detection
  uint8_t _config_glass_tolerance = 0;

  /// @brief real honey weight
  uint16_t _honey_weight = 0;

  /// @brief target weight (from HMConfig)
  uint16_t _config_weight_filling = 0;

  /// @brief starts weight fine filling (from HMConfig)
  uint16_t _config_weight_servo_fine = 0;
};

#endif  // LIB_GLASS_GLASS_H_
