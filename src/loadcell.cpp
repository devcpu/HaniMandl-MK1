/*
 * File: /loadcell.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-22 16:27
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-22 18:27
 * Modified By: Johannes G.  Arlt (janusz)
 * -----
 * Copyright (c) 2023 STRATO AG Berlin, Germany
 */

#include <loadcell.h>

HX711 scale;

float get_set_Weight() {
  float units = scale.get_units(LOADCELL_READ_TIMES);
  HMConfig::instance().weight_current = units;
  return units;
}

void calibrate() {
  log_i("Calibrate:\n");
  scale.set_scale(0);
  scale.set_offset(0);
  log_d("SCALE: %f", scale.get_scale());
  log_d("OFFSET: %d", scale.get_offset());

  log_i("Remove all things from scale and press t in console");
  boolean _resume = false;
  float cal_units = 0;
  float known_mass = 0;

  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') {
        scale.tare();
        log_d("SCALE: %f", scale.get_scale());
        log_d("OFFSET: %d", scale.get_offset());
        _resume = true;
      }
    }
  }

  _resume = false;
  log_i("Now place a know weight and press c in console");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'c') {
        cal_units = scale.get_value(20);
        log_e("Tara Value: %f", cal_units);
        _resume = true;
      }
    }
  }

  _resume = false;
  log_i("Please give the known weight in console in form of 509.0");
  while (_resume == false) {
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        log_i("Known mass is %f", known_mass);
        _resume = true;
      }
    }
  }

  float cal = cal_units / known_mass;
  log_i("cal_unit with know weight: %f", cal_units);
  log_i("Set scale factor: %f", cal);
  scale.set_scale(cal);
  log_d("SCALE: %f", scale.get_scale());
  log_d("OFFSET: %d\n\n", scale.get_offset());
  delay(500);
  show_scale_data();
}

void setupLoadcell() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  log_d("begin");
  show_scale_data();
  scale.set_gain(128);
  scale.power_up();
  calibrate();
  log_d("end");
}

void show_scale_data() {
  log_d("------------   show_scale_data()   ------------");
  log_d("read(): %f", scale.read());
  uint32_t t_s = millis();
  int32_t ra = scale.read_average(LOADCELL_READ_TIMES);
  uint32_t t_e = millis();
  log_d("SCALE: %f", scale.get_scale());
  log_d("OFFSET: %d", scale.get_offset());
  log_d("read_average(5): %d in %d millisec", ra, t_e - t_s);
  log_d("get_value(5): %f", scale.get_value(LOADCELL_READ_TIMES));
  log_d("get_units(5): %f\n", scale.get_units(LOADCELL_READ_TIMES));
  log_d("SCALE: %f", scale.get_scale());
  log_d("OFFSET: %d\n", scale.get_offset());
  log_d("scale.get_value(5) = %f", scale.get_value(LOADCELL_READ_TIMES));
  log_d("Result %.0fg\n\n", round(scale.get_units(LOADCELL_READ_TIMES)));
  String result = String(round(scale.get_units(LOADCELL_READ_TIMES)), 0);
  log_i("%6.2f g", round(scale.get_units(LOADCELL_READ_TIMES)));
}

void weight2seriell() { weight2seriell(scale.get_units(LOADCELL_READ_TIMES)); }

void weight2seriell(float weight_current) {
  for (uint8_t i = 0; i <= 30; i++) {
    Serial.print("\b");
  }
  Serial.printf("%6.2f g", round(weight_current));
}
