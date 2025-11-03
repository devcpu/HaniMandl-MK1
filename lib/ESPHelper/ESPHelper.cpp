/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPHelper.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2023-08-17 00:02
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-03 17:34
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <ESPHelper.h>
#include <esp_chip_info.h>
#include <esp_spi_flash.h>
#include <esp_system.h>
#ifdef ESP32
#include <esp_heap_caps.h>
#endif

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
// TODO implemented: add max free block size + heap fragmentation metrics
Table2RData* ESPHelper::getSystemInfoTable(void) {
#ifdef ESP32
  static Table2RData systemdata[40];
  int i = 0;
  systemdata[i++] = {String("SoftwareVersion:"), HMConfig::instance().version};
  systemdata[i++] = {String("Build DateTime:"), getBuildDateAndTime()};
  systemdata[i++] = {String("SDKVersion:"), String(ESP.getSdkVersion())};
  systemdata[i++] = {String("Uptime:"),
                     String(millis() / 1000 / 60, DEC) + "min"};
  systemdata[i++] = {String("ESP32 Chip ID:"), ESPHelper::getChipId()};
  systemdata[i++] = {String("Chip Revision:"), String(ESP.getChipRevision())};
  systemdata[i++] = {String("Chip Model:"), String(ESP.getChipModel())};
  systemdata[i++] = {String("Chip Cores:"), String(ESP.getChipCores())};
  systemdata[i++] = {String("Chip Features:"), ESPHelper::getFeatureSummary()};
  systemdata[i++] = {String("Reset Reason CPU0:"),
                     ESPHelper::getResetReason(rtc_get_reset_reason(0))};
  systemdata[i++] = {String("Reset Reason CPU1:"),
                     ESPHelper::getResetReason(rtc_get_reset_reason(1))};
  systemdata[i++] = {String("CpuFreqMHz:"),
                     String(ESP.getCpuFreqMHz()) + "MHz"};
  systemdata[i++] = {String("CycleCount:"), String(ESP.getCycleCount())};
  // Flash basics
  systemdata[i++] = {String("FlashChipSize:"),
                     String(ESP.getFlashChipSize() / 1024 / 1024) + "MB"};
  systemdata[i++] = {String("FlashChipSpeed:"),
                     String(ESP.getFlashChipSpeed() / 1000 / 1000) + "MHz"};
  systemdata[i++] = {String("FlashChipMode:"), String(ESP.getFlashChipMode())};
  // Sketch usage
  uint32_t sketchSize = ESP.getSketchSize();
  uint32_t freeSketch = ESP.getFreeSketchSpace();
  char flashLayout[48];
  snprintf(flashLayout, sizeof(flashLayout), "%lukB used / %lukB free",
           (unsigned long)(sketchSize / 1024),
           (unsigned long)(freeSketch / 1024));
  systemdata[i++] = {String("Flash Layout:"), String(flashLayout)};
  systemdata[i++] = {String("SketchSize:"), String(sketchSize / 1024) + "kB"};
  systemdata[i++] = {String("FreeSketchSpace:"),
                     String(freeSketch / 1024) + "kB"};
  systemdata[i++] = {String("SketchMD5:"), String(ESP.getSketchMD5())};
  // Heap metrics
  systemdata[i++] = {String("HeapSize:"),
                     String(ESP.getHeapSize() / 1024) + "kB"};
  systemdata[i++] = {String("FreeHeap:"),
                     String(ESP.getFreeHeap() / 1024) + "kB"};
  systemdata[i++] = {String("MaxAllocHeap:"),
                     String(ESP.getMaxAllocHeap() / 1024) + "kB"};
  systemdata[i++] = {String("MinFreeHeap:"),
                     String(ESP.getMinFreeHeap() / 1024) + "kB"};
  size_t largest_free_block =
      heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
  size_t total_free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  uint32_t fragPct = 0;
  if (total_free_heap > 0 && largest_free_block <= total_free_heap) {
    fragPct = (uint32_t)((100ULL * (total_free_heap - largest_free_block)) /
                         total_free_heap);
  }
  systemdata[i++] = {String("MaxFreeBlockSize:"),
                     String(largest_free_block / 1024) + "kB"};
  systemdata[i++] = {String("HeapFragmentation:"), String(fragPct) + "%"};
  // Termination marker
  systemdata[i] = {String(), String()};
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
  systemdata[13] = {String(), String()};
#endif
  return systemdata;
  // String systemInfoTable = table2DGenerator(systemdata, sizeof(systemdata) /
  // sizeof(systemdata[0]), true); return systemInfoTable; // + mainmenue; FIXME
}

// ---- Extended helpers ----
String ESPHelper::getShortId() {
  uint64_t mac = ESP.getEfuseMac();  // 48-bit value in low bits
  char buf[13];                      // 12 hex chars + NUL
// Use inttypes.h macro for portability instead of hard-coded %llX
#include <inttypes.h>
  snprintf(buf, sizeof(buf), "%012" PRIX64, (uint64_t)mac & 0xFFFFFFFFFFFFULL);
  return String(buf);
}

String ESPHelper::getChipModelString() {
#ifdef ESP32
  return String(ESP.getChipModel());
#else
  return String("UNKNOWN");
#endif
}

String ESPHelper::getFeatureSummary() {
#ifdef ESP32
  esp_chip_info_t info;
  esp_chip_info(&info);
  String f;
  if (info.features & CHIP_FEATURE_WIFI_BGN) f += "WiFi ";
  if (info.features & CHIP_FEATURE_BT) f += "BT ";
  if (info.features & CHIP_FEATURE_BLE) f += "BLE ";
  if (info.features & CHIP_FEATURE_EMB_FLASH)
    f += "embFlash ";
  else
    f += "extFlash ";
  // esp_spiram_is_initialized is the Arduino-ESP32 symbol for PSRAM presence
  if (esp_spiram_is_initialized())
    f += "PSRAM";
  else
    f += "noPSRAM";
  f.trim();
  return f;
#else
  return String("n/a");
#endif
}

// printChipInfo removed (info now fully integrated into table)

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

  // Sicher: Monatskürzel hat immer 3 Buchstaben (Jan..Dec) -> Feldbreite
  // begrenzen Vorher: sscanf(mdate, "%s %d %d", bdt, &day, &year); // konnte
  // theoretisch mehr lesen
  sscanf(mdate, "%3s %d %d", bdt, &day,
         &year);  // cppcheck fixed: add field width

  const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
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

// void ESPHelper::reboot(AsyncWebServerRequest *request) {
//   request->redirect("/rebootinfo");  // TODO(janusz): refactor path
//   delay(3000);
//   ESPHelper::disconnect();
//   ESPHelper::restartESP();
// }

void ESPHelper::disconnect() {
  // Implement disconnect functionality here
}

void ESPHelper::restartESP() {
  // ESP.restart(); // FIXME
}
