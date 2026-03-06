/*
 * File: handleServo.h
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 - 2025 Johannes Arlt (devcpu) Berlin, Germany
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

#endif  // SRC_HANDLESERVO_H_
