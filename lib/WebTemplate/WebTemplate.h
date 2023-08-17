/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: WebTemplate.h
 * Project: HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2023-08-16 23:33
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-17 18:42
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_WEBTEMPLATE_WEBTEMPLATE_H_
#define LIB_WEBTEMPLATE_WEBTEMPLATE_H_

#include <Arduino.h>
#include <ESPHelper.h>
#include <HMConfig.h>
#include <SPIFFS.h>

static const String mainmenue(
    "<form action='.' method='get'><button>Main Menue</button></form><br />");
static const String htmltitle = HONEY_FARM_NAME;
static const String h3title = PROGRAMM_NAME;

String DefaultTemplating(const String &var);

String DefaultProcessor(const String &var);
String FillingTemplating(const String &var);
String SetupFillingTemplating(const String &var);
String SetupTemplating(const String &var);
String CalibrateTemplating(const String &var);
String SetupWlanTemplating(const String &var);
String UpdateFirmwareTemplating(const String &var);
String SystemInfoTemplating(const String &var);

String readSPIFFS2String(const String &path);
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size);
bool isNumber(String val);
String table2DGenerator(Table2RData *systemdata, boolean bold);

#endif  // LIB_WEBTEMPLATE_WEBTEMPLATE_H_
