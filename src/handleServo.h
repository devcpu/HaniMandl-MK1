/*
 * File: /handleServo.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 17:22
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-23 18:41
 * Modified By: Johannes G.  Arlt (janusz)
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

typedef enum {
  FILLING_STATUS_START,
  FILLING_STATUS_OPEN,
  FILLING_STATUS_FINE,
  FILLING_STATUS_CLOSED,
  FILLING_STATUS_FOLLOW_UP,
  FILLING_STATUS_STOPPED,
} FillingStatus;

void setupServo();
int handleWeightAndServo(float weight_current);
String runmod2string(RunModus mod);
String fillingstatus2string(FillingStatus status);
int log_event(float weight_current, int16_t weight_fill_current,
              FillingStatus status, uint16_t weight_last);

#endif  // SRC_HANDLESERVO_H_
