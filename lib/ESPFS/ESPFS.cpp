/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /ESPFS.cpp
 * Project: /home/jan/git/esp32/HaniMandl-MK1/lib/ESPFS
 * Description:
 * -----
 * Created Date: 2023-08-12 17:47
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2023-09-05 13:49
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include <Arduino.h>
#include <ESPFS.h>

ESPFS::ESPFS() {}

bool ESPFS::setup() {
  if (SPIFFS.begin(true)) {
    log_d("SPIFFS is mounted");
    mounted = true;
    return mounted;
  } else {
    log_e("Failed to mount SPIFFS! Did you upload filesystem image?");
    log_w("Try to format SPIFFS.");
    SPIFFS.format();
    if (SPIFFS.begin(true)) {
      log_d("SPIFFS is mounted");
      mounted = true;
      return mounted;
    } else {
      log_e(
          "Failed to mount SPIFFS! Did you upload filesystem image? Give up!");
      return mounted;
    }
  }
}

bool ESPFS::isMounted() {
  if (mounted) {
    return mounted;
  } else {
    return setup();
  }
}

/**
 * The function `readString` reads the content of a file from the SPIFFS
 * file system and returns it as a string.
 *
 * @param path The `path` parameter is a `String` that represents the file path
 * of the file to be read from SPIFFS (SPI Flash File System).
 *
 * @return The function `readSPIFFS2String` returns a `String` object.
 */
String ESPFS::readString(const String &path) {
  if (!isMounted()) {
    return String("");
  }
  if (!SPIFFS.exists(path)) {
    log_e("ERROR, %s does not exist.", path);
    return String("");
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

void ESPFS::writeString(const String &path, const String &data) {
  if (!isMounted()) {
    return;
  }

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    log_e("Failed to open file %s for writing", path);
    return;
  }
  if (file.print(data)) {
    log_d("File %s written", path);
  } else {
    log_e("Error writing to file %s!", path);
  }
  return;
}
