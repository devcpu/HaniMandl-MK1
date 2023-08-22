/*
 * File: /loadcell.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 16:52
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-22 18:26
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#ifndef SRC_LOADCELL_H_
#define SRC_LOADCELL_H_

#include <Arduino.h>
#include <HMConfig.h>
#include <HX711.h>
#include <appconfig.h>

#include "esp_log.h"

void calibrate();
void weight2seriell();
void weight2seriell(float weight_current);
float get_set_Weight();
void show_scale_data();
void setupLoadcell();

#endif  // SRC_LOADCELL_H_
