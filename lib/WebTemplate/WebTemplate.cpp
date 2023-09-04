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
 * Last Modified: 2023-09-04 18:41
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
    log_e("%s", HMConfig::instance().localIP.c_str());
    return HMConfig::instance().localIP;
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
    return HMConfig::instance().beekeeping;
  }
  if (var == "angle_max_hard") {
    return String(HMConfig::instance().servodata.angle_max_hard);
  }
  if (var == "angle_min_hard") {
    return String(HMConfig::instance().servodata.angle_min_hard);
  }
  if (var == "angle_max") {
    return String(HMConfig::instance().servodata.angle_max);
  }
  if (var == "angle_min") {
    return String(HMConfig::instance().servodata.angle_min);
  }
  if (var == "angle_fine") {
    return String(HMConfig::instance().servodata.angle_fine);
  }
  if (var == "glass_tolerance") {
    return String(HMConfig::instance().glass_tolerance);
  }
  if (var == "weight_calibrate") {
    return String(HMConfig::instance().weight_calibrate);
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

String SetupFillingTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/setupfilling.html");
  }
  return DefaultTemplating(var);
}

String SetupTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Grundeinrichtung";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/setup.html");
  }
  return DefaultTemplating(var);
}

String CalibrateTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Waage kalibrieren";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/kalibrieren.html");
  }
  return DefaultTemplating(var);
}

String SetupWlanTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Einrichtung Wlan";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/setupwlan.html");
  }
  return DefaultTemplating(var);
}

String UpdateFirmwareTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Update Firmware";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/updatefirmware.html");
  }
  return DefaultTemplating(var);
}

String FillingTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "Abfüllung";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/filling.html");
  }
  return DefaultTemplating(var);
}

String SystemInfoTemplating(const String &var) {
  if (var == "H2TITLE") {
    return "System Info";
  }
  if (var == "BODY") {
    return table2DGenerator(ESPHelper::getSystemInfoTable(), true);
    // + mainmenue;
  }
  return DefaultTemplating(var);
}

String DefaultProcessor(const String &var) {
  if (var == "H2TITLE") {
    return "Main Menue";
  }
  if (var == "BODY") {
    return ESPHelper::readSPIFFS2String("/into.html");
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
