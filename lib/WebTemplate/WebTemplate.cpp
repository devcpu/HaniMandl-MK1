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
 * Last Modified: 2023-08-18 01:58
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

String DefaultTemplating(const String &var) {
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
  if (var == "waagen_gewicht") {
    return String("not implemented yet");
  }
  if (var == "run_modus") {
    if (cfg.run_modus == RUN_MODUS_AUTO) {
      return String("Auto");
    } else {
      return String("Hand");
    }
    return String("not implemented yet");
  }

  return "wrong placeholder " + var;
}

String SetupFillingTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setupfilling.html");
  }
  return DefaultTemplating(var);
}

String SetupTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Grundeinrichtung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setup.html");
  }
  return DefaultTemplating(var);
}

String CalibrateTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Waage kalibrieren";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/kalibrieren.html");
  }
  return DefaultTemplating(var);
}

String SetupWlanTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Einrichtung Wlan";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/setupwlan.html");
  }
  return DefaultTemplating(var);
}

String UpdateFirmwareTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Update Firmware";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/updatefirmware.html");
  }
  return DefaultTemplating(var);
}

String FillingTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/filling.html");
  }
  return DefaultTemplating(var);
}

String SystemInfoTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "System Info";
  }
  if (var == "BODY") {
    return table2DGenerator(ESPHelper::getSystemInfoTable(), true) +
           mainmenue;  // FIXME
  }
  return DefaultTemplating(var);
}

String DefaultProcessor(const String &var) {
  if (var == "H2TITLE") {
    return "Main Menue";
  }
  if (var == "BODY") {
    return readSPIFFS2String("/mainbutton.html");
  }
  return DefaultTemplating(var);
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
  char buf[val.length() + 1];
  val.toCharArray(buf, val.length() + 1);
  log_e("isNumber::length=%d", val.length());
  for (uint8_t i = 0; i < val.length(); i++) {
    Serial.print(buf[i]);
    if (!isDigit(buf[i])) {
      Serial.println(" is wrong");
      return false;
    }
    Serial.println(" is ok");
  }
  return true;
}

/**
 * table2DGenerator.
 *
 * @author	JA
 * @param	SystemData systemdata[]
 * @param	boolean	bold
 * @return	mixed
 */
String table2DGenerator(Table2RData *systemdata, boolean bold) {
  uint8_t sz = sizeof(systemdata);  // FIXME(janusz)
  log_d("Size of systemdata: %d", sz);
  log_d("bold: %d", bold);

  String retvar("<table>");
  String tdend("</td></tr>");
  String tdstart("<tr><td>");
  String tdmittle("</td><td>");

  if (bold) {
    log_e("bold active");
    tdstart = "<tr><td><b>";
    tdmittle = "</b></td><td>";
  }

  for (uint8_t i = 0; i <= 25; i++) {
    retvar +=
        tdstart + systemdata[i].label + tdmittle + systemdata[i].value + tdend;
  }
  retvar += "</table>";
  return retvar;
}
