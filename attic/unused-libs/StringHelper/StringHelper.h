/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: StringHelper.h
 * Project: HaniMandl-MK1
 * Description: Helper functions to replace Arduino String with char arrays
 * -----
 * Created Date: 2025-10-02
 * Author: GitHub Copilot
 */

#ifndef LIB_STRINGHELPER_STRINGHELPER_H_
#define LIB_STRINGHELPER_STRINGHELPER_H_

#ifndef LIB_STRINGHELPER_STRINGHELPER_H_
#define LIB_STRINGHELPER_STRINGHELPER_H_

#include <Arduino.h>
#include <appconfig.h>

// Helper buffer for temporary string operations
extern char temp_buffer[256];

// Convert integer to char array
void int_to_str(int value, char* buffer, size_t buffer_size);

// Convert float to char array with specified precision
void float_to_str(float value, char* buffer, size_t buffer_size, int precision = 2);

// Safe string copy with bounds checking
void safe_strcpy(char* dest, const char* src, size_t dest_size);

// Safe string concatenation with bounds checking
void safe_strcat(char* dest, const char* src, size_t dest_size);

// Convert const char* to int safely
int str_to_int(const char* str, int default_value = 0);

// Convert const char* to float safely
float str_to_float(const char* str, float default_value = 0.0f);

// Check if string contains only numeric characters
bool is_numeric(const char* str);

// Get temporary buffer for string operations
char* get_temp_buffer();

// Clear temporary buffer
void clear_temp_buffer();

#endif// LIB_STRINGHELPER_STRINGHELPER_H_


#endif// LIB_STRINGHELPER_STRINGHELPER_H_
