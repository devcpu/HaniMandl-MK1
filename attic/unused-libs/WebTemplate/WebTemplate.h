/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: WebTemplate.h
 * Project: uHuFiMa
 * Description:
 * -----
 * Created Date: 2023-08-16 23:33
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2024-10-05 00:00
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_WEBTEMPLATE_WEBTEMPLATE_H_
#define LIB_WEBTEMPLATE_WEBTEMPLATE_H_

#include <Arduino.h>
#include <ESPHelper.h>
#include <HMConfig.h>

static const char mainmenue[] PROGMEM =
    "<br /><form action='.' method='get'><button>Hauptmeü</button></form><br />";
static const char* const htmltitle = HONEY_FARM_NAME;
static const char* const h3title = PROGRAMM_NAME;

const char* DefaultTemplating(const char* var);

const char* DefaultProcessor(const char* var);
const char* FillingTemplating(const char* var);
const char* SetupFillingTemplating(const char* var);
const char* SetupTemplating(const char* var);
const char* CalibrateTemplating(const char* var);
const char* SetupWlanTemplating(const char* var);
const char* UpdateFirmwareTemplating(const char* var);
const char* SystemInfoTemplating(const char* var);
const char* JSTemplating(const char* var);

String optionsFieldGenerator(String selected, const char *name,
                             String data[][2], uint8_t size);
bool isNumber(String val);
String table2DGenerator(Table2RData *systemdata, boolean bold);

#endif  // LIB_WEBTEMPLATE_WEBTEMPLATE_H_
