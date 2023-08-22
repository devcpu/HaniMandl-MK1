/*
 * File: /handleServo.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-22 17:55
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#ifndef SRC_HANDLESERVO_H_
#define SRC_HANDLESERVO_H_

#include <Arduino.h>
#include <ESP32Servo.h>
#include <HMConfig.h>
#include <appconfig.h>

void setupServo();
int handleWeightAndServo(float weight_current);

#endif  // SRC_HANDLESERVO_H_
