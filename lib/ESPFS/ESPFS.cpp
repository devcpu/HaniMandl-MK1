/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPFS.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/ESPFS
 * Description:
 * -----
 * Created Date: 2023-08-12 17:47
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-12 17:51
 * Modified By: Johannes G.  Arlt
 */

#include <Arduino.h>
#include <ESPFS.h>

boolean ESPFSInit() {
  bool initok = false;
  if (SPIFFS.begin()) {
    initok = true;
    log_i("SPIFFS is mounted");
  } else {
    log_e("Failed to mount SPIFFS! Did you upload filesystem image?");
    log_w("Try to format SPIFFS.");
    SPIFFS.format();
    if (SPIFFS.begin()) {
      initok = true;
      log_i("SPIFFS is mounted");
    } else {
      log_e(
          "Failed to mount SPIFFS! Did you upload filesystem image? Give up!");
      initok = false;
    }
  }

  //   bool initok = false;
  //   initok = SPIFFS.begin();
  //   if (!(initok)) { // Format SPIFS, of not formatted. - Try 1
  //     log_i("Format SPIFFS (try 1). Please wait ... ");
  //     SPIFFS.format();
  //     initok = SPIFFS.begin();
  //   }
  //   if (!(initok)) { // Format SPIFS, of not formatted. - Try 2
  //     log_w("Format SPIFFS (try 2). Please wait ... ");
  //     SPIFFS.format();
  //     initok = SPIFFS.begin();
  //   }
  //   if (!initok) {
  //     log_e("SPIFFS not OK");
  //   } else {
  //     log_i("ESP FS ready");
  //   }
  //   // #endif

  return initok;
}
