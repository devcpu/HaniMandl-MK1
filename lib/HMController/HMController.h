/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /HMController.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/HMController
 * Description:
 * -----
 * Created Date: 2023-08-15 03:08
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-17 17:36
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_HMCONTROLLER_HMCONTROLLER_H_
#define LIB_HMCONTROLLER_HMCONTROLLER_H_

#include <Arduino.h>

typedef enum RunMode { hand, auto };

class HMController {
 public:
  RunMode run_mode = RunMode::hand;
  String getWeight();
  void calibrate();
  void start_filling();
  void stop_filling();

 private:
  int16_t weight = 0;
  void weigh();
  void report();
  void sendMQTT();
  void sendAPIRequest();
};

#endif  // LIB_HMCONTROLLER_HMCONTROLLER_H_
