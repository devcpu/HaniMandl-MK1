/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPFS.h
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/ESPFS
 * Description:
 * -----
 * Created Date: 2023-08-12 17:43
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-12 18:21
 * Modified By: Johannes G.  Arlt
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

boolean ESPFSInit(void);

#endif  // LIB_ESPFS_ESPFS_H_
