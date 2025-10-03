/*
 * Copyright (c) 2025 STRATO AG Berlin, Germany
 *  All rights reserved
 * -----
 * File: /freertos_setup.h
 * Project: /home/janusz/git/privat/esp32/HaniMandl-MK1/src
 * Description:
 * -----
 * Created Date: 2025-10-03 13:04
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified:
 * Modified By:
 */

#ifndef SRC_FREERTOS_SETUP_H_
#define SRC_FREERTOS_SETUP_H_

#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/task.h>

struct WeightSample {
  float value;
  uint32_t ts;
};

// Simple event bits
constexpr EventBits_t EV_WIFI_CONNECTED = 0x01;
constexpr EventBits_t EV_NTP_SYNCED = 0x02;

extern QueueHandle_t qWeight;  // Overwrite queue (size 1)
extern EventGroupHandle_t egSystem;

// Task handles (optional use for notifications)
extern TaskHandle_t taskSensorHandle;
extern TaskHandle_t taskServoHandle;
extern TaskHandle_t taskWiFiHandle;

void startSystemTasks();
void tickNTP();  // forward for WiFi task usage

#endif  // SRC_FREERTOS_SETUP_H_
