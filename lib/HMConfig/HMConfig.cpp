/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /Config.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/Config
 * Description:
 * -----
 * Created Date: 2023-08-12 20:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-08-30 02:11
 * Modified By: Johannes G.  Arlt
 */

#include <HMConfig.h>

String HMConfig::runmod2string(RunModus modus) {
  switch (modus) {
    case RUN_MODUS_AUTO:
      return String("RUN_MODUS_AUTO");
    case RUN_MODUS_HAND:
      return String("RUN_MODUS_HAND");
    case RUN_MODUS_STOPPED:
      return String("RUN_MODUS_STOPPED");
    default:
      return String("Unknown modus");
  }
}

String HMConfig::fillingstatus2string(FillingStatus status) {
  switch (status) {
    case FILLING_STATUS_FINE:
      return String("FINE");
    case FILLING_STATUS_OPEN:
      return String("OPEN");
    case FILLING_STATUS_CLOSED:
      return String("CLOSED");
    case FILLING_STATUS_FOLLOW_UP:
      return String("FOLLOW_UP");
    case FILLING_STATUS_STOPPED:
      return String("STOPPED");
    case FILLING_STATUS_STANDBY:
      return String("STANDBY");
    default:
      String retvar = String("unknown filling status: ");
      retvar += String(status);
      return retvar;
  }
}

// HMConfig::HMConfig(void) { log_e("Bad, very bad!!!"); }
