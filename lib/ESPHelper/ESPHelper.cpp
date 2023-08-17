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
 * Last Modified: 2023-08-17 17:37
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
  char ssid1[33];
  char ssid2[33];
  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid1, sizeof(ssid1), "%04X", chip);
  snprintf(ssid2, sizeof(ssid1), "%08X", (uint32_t)chipid);
  return String(ssid1) + String(ssid2);
}

/**
 * table2DGenerator.
 *
 * @author	JA
 * @param	SystemData systemdata[]
 * @param	boolean	bold
 * @return	mixed
 */
String table2DGenerator(SystemData systemdata[], uint8_t size, boolean bold) {
  String tdstart("<tr><td>");
  String tdmittle("</td><td>");
  if (bold) {
    tdstart = "<tr><td><b>";
    tdmittle = "</b></td><td>";
  }
  String tdend("</td></tr>");
  String retvar("<table>");

  for (uint8_t z = 0; z < size; z++) {
    retvar +=
        tdstart + systemdata[z].label + tdmittle + systemdata[z].value + tdend;
  }
  retvar += "</table>";
  return retvar;
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
SystemData *ESPHelper::getSystemInfoTable(void) {
  static SystemData systemdata[30];
  systemdata[0] = {"SoftwareVersion", cfg.version};
  systemdata[1] = {"Build DateTime: ", getBuildDateAndTime()};
  systemdata[2] = {"SDKVersion: ", String(ESP.getSdkVersion())};
  systemdata[3] = {"Uptime: ", String(millis() / 1000 / 60, DEC) + "min"};
#ifdef ESP32
  systemdata[4] = {"Chip Revision:", String(ESP.getChipRevision())};
  systemdata[5] = {"ESP32 Chip ID:", getChipId()};
  systemdata[6] = {"Reset Reason CPU0: ",
                   getResetReason(rtc_get_reset_reason(0))};
  systemdata[7] = {"Reset Reason CPU1: ",
                   getResetReason(rtc_get_reset_reason(1))};
  systemdata[8] = {"CpuFreqMHz: ", String(ESP.getCpuFreqMHz()) + "MHz"};
  systemdata[9] = {"CycleCount: ", String(ESP.getCycleCount())};
  systemdata[10] = {"FlashChipMode: ", String(ESP.getFlashChipMode())};
  systemdata[11] = {"FlashChipSize: ",
                    String(ESP.getFlashChipSize() / 1024 / 1024) + "MB"};
  systemdata[12] = {"FlashChipSpeed: ",
                    String(ESP.getFlashChipSpeed() / 1024 / 1024) + "MHz"};
  systemdata[13] = {"SketchSize: ", String(ESP.getSketchSize() / 1024) + "kB"};
  systemdata[14] = {"FreeSketchSpace: ",
                    String(ESP.getFreeSketchSpace() / 1024) + "kB"};
  systemdata[15] = {"SketchMD5: ", String(ESP.getSketchMD5())};
  systemdata[16] = {"HeapSize: ", String(ESP.getHeapSize() / 1024) + "kB"};
  systemdata[17] = {"FreeHeap: ", String(ESP.getFreeHeap() / 1024) + "kB"};
  systemdata[18] = {"MaxAllocHeap: ",
                    String(ESP.getMaxAllocHeap() / 1024) + "kB"};
  systemdata[19] = {"MinFreeHeap: ",
                    String(ESP.getMinFreeHeap() / 1024) + "kB"};
  systemdata[20] = {"Flash ide  size:",
                    String(ESP.getFlashChipSize() / 1024) + "kB"};
  systemdata[21] = {"Flash ide speed:",
                    String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"};
  systemdata[22] = {"Flash ide mode:", ESPHelper::getFlashMode()};
  systemdata[23] = {"Sketch size:", String(ESP.getSketchSize() / 1024) + "kB"};
  systemdata[24] = {"Free sketch size:",
                    String(ESP.getFreeSketchSpace() / 1024) + "kB"};
  systemdata[25] = {"Free heap:", String(ESP.getFreeHeap() / 1024) + "kB"};
#elif defined(ESP8266)
  systemdata[4] = {"Flash real id:", String(ESP.getFlashChipId(), HEX)};
  systemdata[5] = {"Flash real size:",
                   String(ESP.getFlashChipRealSize() / 1024) + "kB"};
  systemdata[6] = {"ResetReason", String(ESP.getResetReason())};
  systemdata[7] = {"Flash ide  size:",
                   String(ESP.getFlashChipSize() / 1024) + "kB"};
  systemdata[8] = {"Flash ide speed:",
                   String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"};
  systemdata[9] = {"Flash ide mode:", ESPHelper::getFlashMode()};
  systemdata[10] = {"Sketch size:", String(ESP.getSketchSize() / 1024) + "kB"};
  systemdata[11] = {"Free sketch size:",
                    String(ESP.getFreeSketchSpace() / 1024) + "kB"};
  systemdata[12] = {"Free heap:", String(ESP.getFreeHeap() / 1024) + "kB"};
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
  char bdt[45];             // "2017-03-07 11:08:02"
  char mdate[] = __DATE__;  // "Mar  7 2017"
  int month = 0;
  int day = 0;
  int year = 0;

  sscanf(mdate, "%s %d %d", bdt, &day, &year);

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
