/*
 * Copyright (c) 2025 STRATO AG Berlin, Germany
 *  All rights reserved
 * -----
 * File: /freertos_setup.h
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2025-10-03 13:04
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-17 22:30
 * Modified By: Johannes G.  Arlt (janusz)
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
constexpr EventBits_t EV_WIFI_AP_MODE = 0x04;  // Access Point active

extern QueueHandle_t qWeight;  // Overwrite queue (size 1)
extern EventGroupHandle_t egSystem;
// Event queue for WebSocket incremental updates
extern QueueHandle_t qEvents;  // bounded queue for state change events

// Task handles (optional use for notifications)
extern TaskHandle_t taskSensorHandle;
extern TaskHandle_t taskServoHandle;
extern TaskHandle_t taskWiFiHandle;
extern TaskHandle_t taskBuzzerHandle;
extern TaskHandle_t taskWsDispatchHandle;

// Buzzer pattern command
struct BuzzerPattern {
  uint16_t freq;          // Hz
  uint16_t durationMs;    // tone duration per beep
  uint8_t count;          // number of beeps
  uint16_t gapMs;         // gap between beeps
  uint32_t initialDelay;  // delay before first beep
};

extern QueueHandle_t qBuzzer;  // queue for buzzer patterns

// --- Event Push System ---
enum class EventType : uint8_t {
  WeightDelta,
  FillingStatusChange,
  RunModeChange,
  WifiState,
  NtpSynced,
  GlassCount,
  Heartbeat,
  HeapStats,
  ScaleTimeout
};

struct EventMessage {
  EventType type;
  uint32_t ts;
  union {
    float fValue;
    int32_t iValue;
    uint8_t u8;
  } data;
};

bool emitWeight(float w);
bool emitFillingStatus(uint8_t fs);
bool emitRunMode(uint8_t rm);
bool emitWifiState(bool connected, bool apMode);
bool emitNtpSynced();
bool emitGlassCount(uint32_t gc);
bool emitHeapStats();
bool emitScaleTimeout(uint32_t consecutiveTimeouts);

// Monitoring counters
extern volatile uint32_t g_eventQueueOverflows;

void startSystemTasks();
void tickNTP();  // forward for WiFi task usage
bool enqueueBuzzerPattern(uint16_t freq, uint16_t durationMs, uint8_t count,
                          uint16_t gapMs, uint32_t initialDelay = 0);
// Expose snapshot sender for WebServerX websocket onConnect handler
void sendSnapshotInternal();

#endif  // SRC_FREERTOS_SETUP_H_
