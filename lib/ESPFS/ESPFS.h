#include "esp_log.h"
#include <Arduino.h>
#ifdef ESP32
#include <SPIFFS.h>
#else
#error This works on ESP32 only
#endif

#ifndef LIB_ESPFS_H_
#define LIB_ESPFS_H_

boolean ESPFSInit(void);

#endif // LIB_ESPFS_H_
