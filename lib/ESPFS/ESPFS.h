/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPFS.h
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-12 17:43
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2025-11-03 17:39
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_ESPFS_ESPFS_H_
#define LIB_ESPFS_ESPFS_H_

#include <Arduino.h>

#include "esp_log.h"
#ifdef ESP32
#include <SPIFFS.h>
#else
#error This works on ESP32 only
#endif

class ESPFS {
 public:
  bool mounted = false;
  ESPFS();
  String readString(const String& path);
  void writeString(const String& path, const String& data);
  bool setup();

 private:
  bool isMounted();
};

#endif  // LIB_ESPFS_ESPFS_H_
