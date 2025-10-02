/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: WebTemplateOptimized.h
 * Project: HaniMandl-MK1
 * Description: Optimized version of WebTemplate without String usage
 * -----
 * Created Date: 2025-10-02
 * Author: GitHub Copilot
 */

#ifndef LIB_WEBTEMPLATEOPTIMIZED_WEBTEMPLATEOPTIMIZED_H_
#define LIB_WEBTEMPLATEOPTIMIZED_WEBTEMPLATEOPTIMIZED_H_

#include <Arduino.h>
#include <ESPFS.h>
#include <HMConfig.h>
#include <appconfig.h>

// Template response buffer for web responses
extern char template_response_buffer[512];

// Core optimized template function
const char *DefaultTemplatingOptimized(const char *var);

// AsyncWebServer wrapper functions (convert String to const char*)
String DefaultTemplatingWrapper(const String &var);
String FillingTemplatingWrapper(const String &var);
String SetupFillingTemplatingWrapper(const String &var);
String SetupTemplatingWrapper(const String &var);
String CalibrateTemplatingWrapper(const String &var);
String SetupWlanTemplatingWrapper(const String &var);
String UpdateFirmwareTemplatingWrapper(const String &var);
String SystemInfoTemplatingWrapper(const String &var);
String JSTemplatingWrapper(const String &var);

// Helper functions
void formatIntToBuffer(int value);
void formatFloatToBuffer(float value, int precision = 2);
const char *getPageTitle(const char *page);
const char *getPageBody(const char *page);

// Utility function (was in WebTemplate.cpp)
bool isNumber(String val);

#endif  // LIB_WEBTEMPLATEOPTIMIZED_WEBTEMPLATEOPTIMIZED_H_
