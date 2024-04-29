/*
 * File: /main.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 00:19
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2024-04-29 16:09
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <Arduino.h>
#include <ESPFS.h>
#include <HMConfig.h>
#include <HMController.h>
#include <HX711.h>
#include <WebServerX.h>
#include <WiFiManagerX.h>
#include <freertos/FreeRTOS.h>
// #include <HX711_ADC.h>
#include <ESP32Servo.h>
#include <Ticker.h>
#include <appconfig.h>
#include <handleServo.h>
#include <loadcell.h>

#include "esp_log.h"

void donothing();

#endif  // SRC_MAIN_H_
