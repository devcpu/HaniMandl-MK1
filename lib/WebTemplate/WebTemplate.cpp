/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /WebTemplate.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-16 23:33
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-17 03:10
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <WebTemplate.h>

struct HTML_Error {
  String ErrorMsg;
  boolean isSended;

 public:
  void setErrorMsg(String msg) {
    ErrorMsg.isEmpty() ? ErrorMsg = msg : ErrorMsg += msg;
    isSended = false;
  }
  String getErrorMsg(void) {
    isSended = true;
    return ErrorMsg;
  }
};

HTML_Error html_error;

extern HMConfig cfg;

String DefaultPlaceholder(const String &var) {
  if (var == "SERVER_IP") {
    log_e("%s", cfg.localIP.c_str());
    return cfg.localIP;
  }
  if (var == "HTMLTILE") {
    return htmltitle;
  }
  if (var == "H3TITLE") {
    return h3title;
  }
  if (var == "ERRORMSG") {
    return html_error.getErrorMsg();
  }
  if (var == "mainmenue") {
    return mainmenue;
  }
  if (var == "los_number") {
    return cfg.los_number;
  }
  if (var == "date_filling") {
    return cfg.date_filling;
  }
  if (var == "weight_empty") {
    return String(cfg.weight_empty);
  }
  if (var == "weight_filling") {
    return String(cfg.weight_filling);
  }
  if (var == "beekeeping") {
    return cfg.beekeeping;
  }
  if (var == "angle_max_hard") {
    return String(cfg.angle_max_hard);
  }
  if (var == "angle_min_hard") {
    return String(cfg.angle_min_hard);
  }
  if (var == "angle_max") {
    return String(cfg.angle_max);
  }
  if (var == "angle_min") {
    return String(cfg.angle_min);
  }
  if (var == "angle_fine") {
    return String(cfg.angle_fine);
  }
  if (var == "glass_tolerance") {
    return String(cfg.glass_tolerance);
  }
  if (var == "weight_calibrate") {
    return String(cfg.weight_calibrate);
  }
  if (var == "glass_count") {
    return String(cfg.glass_count);
  }
  if (var == "weight_fine") {
    return String(cfg.weight_fine);
  }
  return "wrong placeholder " + var;
}

String ProcessorSetupFilling(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setupfilling.html");
  }
  return DefaultPlaceholder(var);
}

String ProcessorSetup(const String &var) {
  if (var == "H2TITLE") {
    return "Grundeinrichtung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setup.html");
  }
  return DefaultPlaceholder(var);
}

String ProcessorCalibrate(const String &var) {
  if (var == "H2TITLE") {
    return "Waage kalibrieren";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/kalibrieren.html");
  }
  return DefaultPlaceholder(var);
}

String ProcessorSetupWlan(const String &var) {
  if (var == "H2TITLE") {
    return "Einrichtung Wlan";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setupwlan.html");
  }
  return DefaultPlaceholder(var);
}

String ProcessorUpdateFirmware(const String &var) {
  if (var == "H2TITLE") {
    return "Update Firmware";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/updatefirmware.html");
  }
  return DefaultPlaceholder(var);
}

String ProcessorFilling(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/filling.html");
  }
  return DefaultPlaceholder(var);
}

String SystemInfoProcessor(const String &var) {
  if (var == "H2TITLE") {
    return "System Info";
  }
  if (var == "BODY") {
    return String("");  // FIXME
  }
  return DefaultPlaceholder(var);
}

String DefaultProcessor(const String &var) {
  if (var == "H2TITLE") {
    return "Main Menue";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/mainbutton.html");
  }
  return DefaultPlaceholder(var);
}

/**
 * The function `readSPIFFS2String` reads the content of a file from the SPIFFS
 * file system and returns it as a string.
 *
 * @param path The `path` parameter is a `String` that represents the file path
 * of the file to be read from SPIFFS (SPI Flash File System).
 *
 * @return The function `readSPIFFS2String` returns a `String` object.
 */
String readSPIFFS2String(const String &path) {
  if (!SPIFFS.exists(path)) {
    String error = "ERROR, " + path + " does not exist.";
    log_e("%s", error.c_str());
    return error;
  }

  File file = SPIFFS.open(path, "r");
  if (!file) {
    String error = "Failed to open file for reading";
    log_e("%s", error.c_str());
    return error;
  }

  String fileContent;
  while (file.available()) {
    fileContent += static_cast<char>(file.read());
  }

  file.close();
  return fileContent;
}

/**
 * The function generates an HTML select field with options based on the
 * provided data and selected value.
 *
 * @param selected The "selected" parameter is a string that represents the
 * currently selected option in the dropdown menu.
 * @param name The `name` parameter is a pointer to a character array that
 * represents the name of the select field in HTML.
 * @param data The "data" parameter is a 2-dimensional array of strings. Each
 * row in the array represents an option in the select field. The first column
 * of each row contains the display text for the option, and the second column
 * contains the corresponding value for the option.
 * @param size The parameter "size" represents the number of options in the
 * dropdown menu.
 *
 * @return a string that represents an HTML select field with options.
 */
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size) {
  ESP_LOGD("WebServerX", "%s", name);
  ESP_LOGD("WebServerX", "%s", selected);

  String buf = "\n\n<select name='" + String(name) + "'>\n";

  for (uint8_t i = 0; i < size; i++) {
    String selectxt = (selected == data[i][1]) ? " selected " : "";
    buf += "<option value=\"" + data[i][1] + "\"" + selectxt + ">" +
           data[i][0] + "</option>\n";
  }

  buf += "</select>\n\n";
  ESP_LOGD("WebServerX", "%s", name);

  return buf;
}

bool isNumber(String val) {
  uint8_t length = val.length();
  char buf[length];
  val.toCharArray(buf, length);

  for (uint8_t i = 0; i < length; i++) {
    if (!isDigit(buf[i])) {
      return false;
    }
  }
  return true;
}
