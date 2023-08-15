/*
 * File: /HMControler.h
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/HMControler
 * Description:
 * -----
 * Created Date: 2023-08-15 03:08
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-15 03:30
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */
#ifndef LIB_HMCONTROLER_HMCONTROLER_H_
#define LIB_HMCONTROLER_HMCONTROLER_H_

typedef enum RunMode { hand, auto };

class HMControler {
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
}

#endif /* LIB_HMCONTROLER_H_ */
