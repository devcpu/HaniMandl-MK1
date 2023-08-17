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
 * Last Modified: 2023-08-17 03:09
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

String DefaultPlaceholder(const String &var);
String ProcessorSetupFilling(const String &var);
String readSPIFFS2String(const String &path);
String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size);
bool isNumber(String val);
String DefaultProcessor(const String &var);
String ProcessorFilling(const String &var);
String ProcessorUpdateFirmware(const String &var);
String ProcessorSetupWlan(const String &var);
String ProcessorCalibrate(const String &var);
String ProcessorSetup(const String &var);
String SystemInfoProcessor(const String &var);

#endif  // LIB_WEBTEMPLATE_WEBTEMPLATE_H_
