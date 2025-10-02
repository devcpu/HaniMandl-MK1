/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: WebTemplateOptimized.cpp
 * Project: HaniMandl-MK1
 * Description: Optimized version of WebTemplate without String usage
 * -----
 * Created Date: 2025-10-02
 * Author: GitHub Copilot
 */

#include "WebTemplateOptimized.h"

// Static buffer for template responses
char template_response_buffer[512];

const char *DefaultTemplatingOptimized(const char *var) {
  if (var == nullptr) return "";

  // Use string comparison for const char*
  if (strcmp(var, "SERVER_IP") == 0) {
    return HMConfig::instance().localIP;
  }
  if (strcmp(var, "HTMLTILE") == 0) {
    return HONEY_FARM_NAME;
  }
  if (strcmp(var, "H3TITLE") == 0) {
    return PROGRAMM_NAME;
  }
  if (strcmp(var, "los_number") == 0) {
    return HMConfig::instance().los_number;
  }
  if (strcmp(var, "date_filling") == 0) {
    return HMConfig::instance().date_filling;
  }
  if (strcmp(var, "beekeeping") == 0) {
    return HMConfig::instance().beekeeping;
  }

  // Numeric values need formatting
  if (strcmp(var, "glass_empty") == 0) {
    formatIntToBuffer(HMConfig::instance().glass_empty);
    return template_response_buffer;
  }
  if (strcmp(var, "weight_filling") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_filling);
    return template_response_buffer;
  }
  if (strcmp(var, "weight_fine") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_fine);
    return template_response_buffer;
  }
  if (strcmp(var, "weight_current") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_current);
    return template_response_buffer;
  }
  if (strcmp(var, "waagen_gewicht") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_current);
    return template_response_buffer;
  }
  if (strcmp(var, "weight_honey") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_honey);
    return template_response_buffer;
  }
  if (strcmp(var, "honey_gewicht") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_honey);
    return template_response_buffer;
  }
  if (strcmp(var, "glass_count") == 0) {
    formatIntToBuffer(HMConfig::instance().glass_count);
    return template_response_buffer;
  }
  if (strcmp(var, "glass_tolerance") == 0) {
    formatIntToBuffer(HMConfig::instance().glass_tolerance);
    return template_response_buffer;
  }
  if (strcmp(var, "weight_calibrate") == 0) {
    formatIntToBuffer(HMConfig::instance().weight_calibrate);
    return template_response_buffer;
  }

  // Servo angles
  if (strcmp(var, "angle_max_hard") == 0) {
    formatIntToBuffer(HMConfig::instance().servodata.angle_max_hard);
    return template_response_buffer;
  }
  if (strcmp(var, "angle_min_hard") == 0) {
    formatIntToBuffer(HMConfig::instance().servodata.angle_min_hard);
    return template_response_buffer;
  }
  if (strcmp(var, "angle_max") == 0) {
    formatIntToBuffer(HMConfig::instance().servodata.angle_max);
    return template_response_buffer;
  }
  if (strcmp(var, "angle_min") == 0) {
    formatIntToBuffer(HMConfig::instance().servodata.angle_min);
    return template_response_buffer;
  }
  if (strcmp(var, "angle_fine") == 0) {
    formatIntToBuffer(HMConfig::instance().servodata.angle_fine);
    return template_response_buffer;
  }

  // Status strings
  if (strcmp(var, "run_modus") == 0) {
    return HMConfig::runmod2string(HMConfig::instance().run_modus);
  }

  // Default return empty string
  return "";
}

void formatIntToBuffer(int value) {
  snprintf(template_response_buffer, sizeof(template_response_buffer), "%d",
           value);
}

void formatFloatToBuffer(float value, int precision) {
  snprintf(template_response_buffer, sizeof(template_response_buffer), "%.*f",
           precision, value);
}

// AsyncWebServer Wrapper Functions
// These convert String parameters to const char* and call optimized functions

extern ESPFS espfs;

String DefaultTemplatingWrapper(const String &var) {
  const char *result = DefaultTemplatingOptimized(var.c_str());
  return String(result);
}

String FillingTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("Abfüllung");
  }
  if (var == "BODY") {
    return espfs.readString("/filling.html");
  }
  return DefaultTemplatingWrapper(var);
}

String SetupFillingTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("Abfüllung");
  }
  if (var == "BODY") {
    return espfs.readString("/setupfilling.html");
  }
  return DefaultTemplatingWrapper(var);
}

String SetupTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("Grundeinrichtung");
  }
  if (var == "BODY") {
    return espfs.readString("/setup.html");
  }
  return DefaultTemplatingWrapper(var);
}

String CalibrateTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("Waage kalibrieren");
  }
  if (var == "BODY") {
    return espfs.readString("/kalibrieren.html");
  }
  return DefaultTemplatingWrapper(var);
}

String SetupWlanTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("Einrichtung Wlan");
  }
  if (var == "BODY") {
    return espfs.readString("/setupwlan.html");
  }
  return DefaultTemplatingWrapper(var);
}

String UpdateFirmwareTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("Update Firmware");
  }
  if (var == "BODY") {
    return espfs.readString("/updatefirmware.html");
  }
  return DefaultTemplatingWrapper(var);
}

String SystemInfoTemplatingWrapper(const String &var) {
  if (var == "H2TITLE") {
    return String("System Information");
  }
  if (var == "BODY") {
    // System info would need to be implemented
    return String("System Info Page");
  }
  return DefaultTemplatingWrapper(var);
}

String JSTemplatingWrapper(const String &var) {
  // JavaScript template - just pass through for now
  return DefaultTemplatingWrapper(var);
}

// Utility function (copied from WebTemplate.cpp)
bool isNumber(String val) {
  if (val.length() == 0) {
    return false;
  }
  for (uint8_t i = 0; i < val.length(); i++) {
    if (!isDigit(val[i])) {
      return false;
    }
  }
  return true;
}
