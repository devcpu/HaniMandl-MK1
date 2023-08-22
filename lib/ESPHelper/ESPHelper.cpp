/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPHelper.cpp
 * Project: /home/janusz/git/esp32/HaniMandl-MK1/lib/ESPHelper
 * Description:
 * -----
 * Created Date: 2023-08-17 00:02
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2023-08-22 17:03
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <ESPHelper.h>

/**
 * The function `getChipId` returns a string that combines the hexadecimal
 * representation of the upper 16 bits and lower 32 bits of the ESP chip ID.
 *
 * @return String ESP32 chip id
 */
String ESPHelper::getChipId() {
  char ssid1[33];  // flawfinder: ignore
  char ssid2[33];  // flawfinder: ignore
  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid1, sizeof(ssid1), "%04X", chip);
  snprintf(ssid2, sizeof(ssid1), "%08X", (uint32_t)chipid);
  return String(ssid1) + String(ssid2);
}

String ESPHelper::getFlashMode() {
  FlashMode_t ideMode = ESP.getFlashChipMode();
  return String((ideMode == FM_QIO    ? "QIO"
                 : ideMode == FM_QOUT ? "QOUT"
                 : ideMode == FM_DIO  ? "DIO"
                 : ideMode == FM_DOUT ? "DOUT"
                                      : "UNKNOWN"));
}

/**
 * The function `getSystemInfoTable` returns a table containing various system
 * information such as software version, build date and time, SDK version,
 * uptime, chip revision, flash chip size, sketch size, and free heap size.
 *
 * @return a string that contains a table of system information.
 */
Table2RData *ESPHelper::getSystemInfoTable(void) {
#ifdef ESP32
  static Table2RData systemdata[30];
  systemdata[0] = {String("SoftwareVersion:"), HMConfig::instance().version};
  systemdata[1] = {String("Build DateTime:"), getBuildDateAndTime()};
  systemdata[2] = {String("SDKVersion:"), String(ESP.getSdkVersion())};
  systemdata[3] = {String("Uptime:"),
                   String(millis() / 1000 / 60, DEC) + "min"};
  //   systemdata[4] = {String("Chip Revision:"),
  //   String(ESP.getChipRevision())};
  systemdata[5] = {String("ESP32 Chip ID:"), ESPHelper::getChipId()};
  systemdata[6] = {String("Reset Reason CPU0:"),
                   ESPHelper::getResetReason(rtc_get_reset_reason(0))};
  systemdata[7] = {String("Reset Reason CPU1:"),
                   ESPHelper::getResetReason(rtc_get_reset_reason(1))};
  systemdata[8] = {String("CpuFreqMHz:"), String(ESP.getCpuFreqMHz()) + "MHz"};
  systemdata[9] = {String("CycleCount:"), String(ESP.getCycleCount())};
  systemdata[10] = {String("FlashChipMode:"), String(ESP.getFlashChipMode())};
  systemdata[11] = {String("FlashChipSize:"),
                    String(ESP.getFlashChipSize() / 1024 / 1024) + "MB"};
  systemdata[12] = {String("FlashChipSpeed:"),
                    String(ESP.getFlashChipSpeed() / 1024 / 1024) + "MHz"};
  systemdata[13] = {String("SketchSize:"),
                    String(ESP.getSketchSize() / 1024) + "kB"};
  systemdata[14] = {String("FreeSketchSpace:"),
                    String(ESP.getFreeSketchSpace() / 1024) + "kB"};
  systemdata[15] = {String("SketchMD5:"), String(ESP.getSketchMD5())};
  systemdata[16] = {String("HeapSize:"),
                    String(ESP.getHeapSize() / 1024) + "kB"};
  systemdata[17] = {String("FreeHeap:"),
                    String(ESP.getFreeHeap() / 1024) + "kB"};
  systemdata[18] = {String("MaxAllocHeap:"),
                    String(ESP.getMaxAllocHeap() / 1024) + "kB"};
  systemdata[19] = {String("MinFreeHeap:"),
                    String(ESP.getMinFreeHeap() / 1024) + "kB"};
  systemdata[20] = {String("Flash ide  size:"),
                    String(ESP.getFlashChipSize() / 1024) + "kB"};
  systemdata[21] = {String("Flash ide speed:"),
                    String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"};
  systemdata[22] = {String("Flash ide mode:"), ESPHelper::getFlashMode()};
  systemdata[23] = {String("Sketch size: "),
                    String(ESP.getSketchSize() / 1024) + "kB"};
  systemdata[24] = {String("Free sketch size:"),
                    String(ESP.getFreeSketchSpace() / 1024) + "kB"};
  systemdata[25] = {String("Free heap:"),
                    String(ESP.getFreeHeap() / 1024) + "kB"};
#elif defined(ESP8266)
  static Table2RData systemdata[15];
  systemdata[0] = {String("SoftwareVersion:"), HMConfig::instance().version};
  systemdata[1] = {String("Build DateTime:"), ESPHelper::getBuildDateAndTime()};
  systemdata[2] = {String("SDKVersion:"), String(ESP.getSdkVersion())};
  systemdata[3] = {String("Uptime:"),
                   String(millis() / 1000 / 60, DEC) + "min"};
  systemdata[4] = {String("Flash real id:"), String(ESP.getFlashChipId(), HEX)};
  systemdata[5] = {String("Flash real size:"),
                   String(ESP.getFlashChipRealSize() / 1024) + "kB"};
  systemdata[6] = {String("ResetReason:"), String(ESP.getResetReason())};
  systemdata[7] = {String("Flash ide  size:"),
                   String(ESP.getFlashChipSize() / 1024) + "kB"};
  systemdata[8] = {String("Flash ide speed:"),
                   String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"};
  systemdata[9] = {String("Flash ide mode:"), ESPHelper::getFlashMode()};
  systemdata[10] = {String("Sketch size: "),
                    String(ESP.getSketchSize() / 1024) + "kB"};
  systemdata[11] = {String("Free sketch size:"),
                    String(ESP.getFreeSketchSpace() / 1024) + "kB"};
  systemdata[12] = {String("Free heap:"),
                    String(ESP.getFreeHeap() / 1024) + "kB"};
#endif
  return systemdata;
  // String systemInfoTable = table2DGenerator(systemdata, sizeof(systemdata) /
  // sizeof(systemdata[0]), true); return systemInfoTable; // + mainmenue; FIXME
}

/**
 * The function GetBuildDateAndTime returns a string representing the build date
 * and time in the format "YYYY-MM-DD HH:MM:SS".
 *
 * @return a string that represents the build date and time in the format
 * "YYYY-MM-DD HH:MM:SS".
 */
String ESPHelper::getBuildDateAndTime() {
  char bdt[45];             // flawfinder: ignore   // "2017-03-07 11:08:02"
  char mdate[] = __DATE__;  // "Mar  7 2017"
  int month = 0;
  int day = 0;
  int year = 0;

  sscanf(mdate, "%s %d %d", bdt, &day, &year);  // flawfinder: ignore

  const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  for (int i = 0; i < 12; i++) {
    if (strcmp(bdt, monthNames[i]) == 0) {
      month = i + 1;
      break;
    }
  }
  snprintf(bdt, sizeof(bdt), "%04d-%02d-%02d %s", year, month, day, __TIME__);

  return String(bdt);
}

#ifdef ESP32
/// @brief Convert int RESET_REASON in more human readable string
/// @param reason
/// @return more human readable reset reason
String ESPHelper::getResetReason(RESET_REASON reason) {
  String retvar;
  switch (reason) {
    case 1:
      retvar = "POWERON_RESET";
      break; /**<1, Vbat power on reset*/
    case 3:
      retvar = "SW_RESET";
      break; /**<3, Software reset digital core*/
    case 4:
      retvar = "OWDT_RESET";
      break; /**<4, Legacy watch dog reset digital core*/
    case 5:
      retvar = "DEEPSLEEP_RESET";
      break; /**<5, Deep Sleep reset digital core*/
    case 6:
      retvar = "SDIO_RESET";
      break; /**<6, Reset by SLC module, reset digital core*/
    case 7:
      retvar = "TG0WDT_SYS_RESET";
      break; /**<7, Timer Group0 Watch dog reset digital core*/
    case 8:
      retvar = "TG1WDT_SYS_RESET";
      break; /**<8, Timer Group1 Watch dog reset digital core*/
    case 9:
      retvar = "RTCWDT_SYS_RESET";
      break; /**<9, RTC Watch dog Reset digital core*/
    case 10:
      retvar = "INTRUSION_RESET";
      break; /**<10, Instrusion tested to reset CPU*/
    case 11:
      retvar = "TGWDT_CPU_RESET";
      break; /**<11, Time Group reset CPU*/
    case 12:
      retvar = "SW_CPU_RESET";
      break; /**<12, Software reset CPU*/
    case 13:
      retvar = "RTCWDT_CPU_RESET";
      break; /**<13, RTC Watch dog Reset CPU*/
    case 14:
      retvar = "EXT_CPU_RESET";
      break; /**<14, for APP CPU, reseted by PRO CPU*/
    case 15:
      retvar = "RTCWDT_BROWN_OUT_RESET";
      break; /**<15, Reset when the vdd voltage is not stable*/
    case 16:
      retvar = "RTCWDT_RTC_RESET";
      break; /**<16, RTC Watch dog reset digital core and rtc module*/
    default:
      retvar = "NO_MEAN";
  }
  return retvar;
}
#endif

void ESPHelper::reboot(AsyncWebServerRequest *request) {
  request->redirect("/rebootinfo");  // TODO(janusz): refactor path
  delay(3000);
  ESPHelper::disconnect();
  ESPHelper::restartESP();
}

void ESPHelper::disconnect() {
  // Implement disconnect functionality here
}

void ESPHelper::restartESP() {
  // ESP.restart(); // FIXME
}
