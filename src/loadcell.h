/*
 * File: /loadcell.h
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-22 16:52
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 - 2025 Johannes Arlt (devcpu) Berlin, Germany
 */

#ifndef SRC_LOADCELL_H_
#define SRC_LOADCELL_H_

#include <Arduino.h>
#include <HMConfig.h>
#include <HX711.h>
#include <appconfig.h>

#include "esp_log.h"

void weight2seriell();
void weight2seriell(float weight_current);
void show_scale_data();
void setupLoadcell();

// Wait until the HX711 signals readiness or timeout (ms). Returns true if
// ready.
bool waitForScaleReady(uint32_t timeoutMs = 250);
// Safe wrappers with readiness guard. Return NAN on timeout.
float safeGetUnits(uint8_t times = 1, uint32_t timeoutMs = 250);
long safeGetValue(uint8_t times = 1, uint32_t timeoutMs = 250);

#endif  // SRC_LOADCELL_H_
