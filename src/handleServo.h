/*
 * File: handleServo.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-30 01:50
 * Modified By: Johannes G.  Arlt
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#ifndef SRC_HANDLESERVO_H_
#define SRC_HANDLESERVO_H_

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Glass.h>
#include <HMConfig.h>
#include <HX711.h>
#include <appconfig.h>

void setupServo();
int handleWeightAndServo(float weight_current);
String runmod2string(RunModus mod);
String fillingstatus2string(FillingStatus status);
int log_event(float weight_current, int16_t weight_fill_current,
              FillingStatus status, uint16_t weight_last);

#endif  // SRC_HANDLESERVO_H_
