/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: StringHelper.cpp
 * Project: HaniMandl-MK1
 * Description: Helper functions to replace Arduino String with char arrays
 * -----
 * Created Date: 2025-10-02
 * Author: GitHub Copilot
 */

#include "StringHelper.h"

// Global temporary buffer for string operations
char temp_buffer[256];

void int_to_str(int value, char* buffer, size_t buffer_size) {
  snprintf(buffer, buffer_size, "%d", value);
}

void float_to_str(float value, char* buffer, size_t buffer_size, int precision) {
  snprintf(buffer, buffer_size, "%.*f", precision, value);
}

void safe_strcpy(char* dest, const char* src, size_t dest_size) {
  if (dest == nullptr || src == nullptr || dest_size == 0) return;
  strlcpy(dest, src, dest_size);
}

void safe_strcat(char* dest, const char* src, size_t dest_size) {
  if (dest == nullptr || src == nullptr || dest_size == 0) return;
  strlcat(dest, src, dest_size);
}

int str_to_int(const char* str, int default_value) {
  if (str == nullptr || !is_numeric(str)) return default_value;
  return atoi(str);
}

float str_to_float(const char* str, float default_value) {
  if (str == nullptr) return default_value;
  char* endptr;
  float result = strtof(str, &endptr);
  return (endptr == str) ? default_value : result;
}

bool is_numeric(const char* str) {
  if (str == nullptr || *str == '\0') return false;

  // Handle negative numbers
  if (*str == '-') str++;

  bool has_digit = false;
  bool has_dot = false;

  while (*str) {
    if (*str >= '0' && *str <= '9') {
      has_digit = true;
    } else if (*str == '.' && !has_dot) {
      has_dot = true;
    } else {
      return false;
    }
    str++;
  }

  return has_digit;
}

char* get_temp_buffer() {
  return temp_buffer;
}

void clear_temp_buffer() {
  temp_buffer[0] = '\0';
}
