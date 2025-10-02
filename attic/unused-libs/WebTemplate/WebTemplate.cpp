/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: WebTemplate.cpp
 * Project: uHuFiMa
 * Description: Template-Engine for Webserver
 * This file contains the implementation of the Template-Engine for the
 * webserver. It provides functions for generating HTML templates and processing
 * template variables.
 * -----
 * Created Date: 2023-08-16 23:33
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2024-10-04 23:57
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <WebTemplate.h>

extern ESPFS espfs;

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

/**
 * @brief Default templating function.
 *
 * This function takes a variable as input and returns a string.
 *
 * @param var The variable to be processed.
 * @return The processed string.
 */
String DefaultTemplating(const String &var) {
  log_e("DefaultTemplating %s", var.c_str());
  if (var == "SERVER_IP") {
    log_e("%s", HMConfig::instance().localIP);
    return String(HMConfig::instance().localIP);
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
  //   if (var == "mainmenue") {
  //     return mainmenue;
  //   }
  if (var == "los_number") {
    return HMConfig::instance().los_number;
  }
  if (var == "date_filling") {
    return HMConfig::instance().date_filling;
  }
  if (var == "glass_empty") {
    return String(HMConfig::instance().glass_empty);
  }
  if (var == "weight_filling") {
    return String(HMConfig::instance().weight_filling);
  }
  if (var == "beekeeping") {
    String s = HMConfig::instance().beekeeping;
    log_e("beekeeping: %s", s.c_str());
    return s;
  }
  if (var == "angle_max_hard") {
    String s = String(HMConfig::instance().servodata.angle_max_hard);
    log_e("angle_max_hard: %s", s.c_str());
    return s;
  }
  if (var == "angle_min_hard") {
    String s = String(HMConfig::instance().servodata.angle_min_hard);
    log_e("angle_min_hard: %s", s.c_str());
    return s;
  }
  if (var == "angle_max") {
    String s = String(HMConfig::instance().servodata.angle_max);
    log_e("angle_max: %s", s.c_str());
    return s;
  }
  if (var == "angle_min") {
    String s = String(HMConfig::instance().servodata.angle_min);
    log_e("angle_min: %s", s.c_str());
    return s;
  }
  if (var == "angle_fine") {
    String s = String(HMConfig::instance().servodata.angle_fine);
    log_e("angle_fine: %s", s.c_str());
    return s;
  }
  if (var == "glass_tolerance") {
    String s = String(HMConfig::instance().glass_tolerance);
    log_e("glass_tolerance: %s", s.c_str());
    return s;
  }
  if (var == "weight_calibrate") {
    String s = String(HMConfig::instance().weight_calibrate);
    log_e("weight_calibrate: %s", s.c_str());
    return s;
  }
  if (var == "glass_count") {
    return String(HMConfig::instance().glass_count);
  }
  if (var == "weight_fine") {
    return String(HMConfig::instance().weight_fine);
  }
  if (var == "waagen_gewicht") {
    return String(HMConfig::instance().weight_current);
  }
  if (var == "honey_gewicht") {
    return String(HMConfig::instance().weight_honey);
  }
  if (var == "run_modus") {
    return HMConfig::instance().runmod2string(HMConfig::instance().run_modus);
    // if (HMConfig::instance().run_modus == RUN_MODUS_AUTO) {
    //   return String("Auto");
    // } else {
    //   return String("Hand");
    // }
    // return String("not implemented yet");
  }

  return "wrong placeholder " + var;
}

/**
 * @brief Sets up the filling templating for the given variable.
 *
 * @param var The variable to set up the filling templating for.
 * @return The modified string after setting up the filling templating.
 */
String SetupFillingTemplating(const String &var) {
  log_e("SetupFillingTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return espfs.readString("/setupfilling.html");
  }
  return DefaultTemplating(var);
}

String SetupTemplating(const String &var) {
  log_e("SetupTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "Grundeinrichtung";
  }
  if (var == "BODY") {
    return espfs.readString("/setup.html");
  }
  return DefaultTemplating(var);
}

String CalibrateTemplating(const String &var) {
  log_e("CalibrateTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "Waage kalibrieren";
  }
  if (var == "BODY") {
    return espfs.readString("/kalibrieren.html");
  }
  return DefaultTemplating(var);
}

String SetupWlanTemplating(const String &var) {
  log_e("SetupWlanTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "Einrichtung Wlan";
  }
  if (var == "BODY") {
    return espfs.readString("/setupwlan.html");
  }
  return DefaultTemplating(var);
}

String UpdateFirmwareTemplating(const String &var) {
  log_e("UpdateFirmwareTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "Update Firmware";
  }
  if (var == "BODY") {
    return espfs.readString("/updatefirmware.html");
  }
  return DefaultTemplating(var);
}

String FillingTemplating(const String &var) {
  log_e("FillingTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return espfs.readString("/filling.html");
  }
  return DefaultTemplating(var);
}

String SystemInfoTemplating(const String &var) {
  log_e("SystemInfoTemplating %s", var.c_str());
  if (var == "H2TITLE") {
    return "System Info";
  }
  if (var == "BODY") {
    return table2DGenerator(ESPHelper::getSystemInfoTable(), true);
    // + mainmenue;
  }
  return DefaultTemplating(var);
}

String JSTemplating(const String &var) {
  log_e("JSTemplating %s", var.c_str());
  if (var == "SERVER_IP") {
    String localIP = HMConfig::instance().localIP;
    log_i("Local IP: %s", localIP.c_str());
    return localIP;
  }
  return DefaultTemplating(var);
}

String DefaultProcessor(const String &var) {
  log_e("DefaultProcessor %s", var.c_str());
  if (var == "H2TITLE") {
    return "Main Menue";
  }
  if (var == "BODY") {
    return espfs.readString("/into.html");
  }
  return DefaultTemplating(var);
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
  log_e("optionsFieldGenerator %s", name);
  ESP_LOGD("WebServerX", "%s", name);
  ESP_LOGD("WebServerX", "%s", selected.c_str());

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
  log_e("isNumber %s", val.c_str());
  if (val.length() == 0) {
    return false;
  }
  char buf[val.length() + 1];
  val.toCharArray(buf, val.length() + 1);
  for (uint8_t i = 0; i < val.length(); i++) {
    if (!isDigit(buf[i])) {
      log_e("%d", buf[i]);
      return false;
    }
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
  log_e("table2DGenerator");
  uint8_t sz = sizeof(systemdata);  // FIXME(janusz)
  log_d("Size of systemdata: %d", sz);
  log_d("bold: %d", bold);

  String retvar("<table>");
  String tdend("</td></tr>");
  String tdstart("<tr><td class=\"w3-left-align\">");
  String tdmittle("</td><td class=\"w3-right-align\">");

  if (bold) {
    log_e("bold active");
    tdstart = "<tr><td class=\"w3-left-align b\">";
    tdmittle = "</td><td class=\"w3-right-align\">";
  }

  for (uint8_t i = 0; i <= 25; i++) {
    retvar +=
        tdstart + systemdata[i].label + tdmittle + systemdata[i].value + tdend;
  }
  retvar += "</table>";
  return retvar;
}
