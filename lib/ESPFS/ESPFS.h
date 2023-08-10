#include <Arduino.h>
#include "esp_log.h"
#ifdef ESP32
#include <SPIFFS.h>
#else
#error This works on ESP32 only
#endif


#ifndef LIB_SLORAAPRS_ESPFS_H_
#define LIB_SLORAAPRS_ESPFS_H_

boolean ESPFSInit(void);

#endif // LIB_SLORAAPRS_ESPFS_H_
