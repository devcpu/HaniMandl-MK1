/*
 * Copyright (c) 2025 - 2026 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /cooperative_loop.h
 * Project: Simple Automatic Honey Filling Machine
 * Description: Cooperative main-loop tick functions (replaces FreeRTOS tasks)
 * -----
 * Created Date: 2026-03-06
 * Author: Johannes G.  Arlt (janusz)
 */

#ifndef SRC_COOPERATIVE_LOOP_H_
#define SRC_COOPERATIVE_LOOP_H_

#include <Arduino.h>

/**
 * @brief Minimal lock-free ring buffer for single-producer / single-consumer.
 * @tparam T Element type
 * @tparam N Capacity (must be > 0)
 */
template <typename T, uint8_t N>
class RingBuffer {
 public:
  bool push(const T& item) {
    uint8_t next = (head_ + 1) % N;
    if (next == tail_) return false;  // full
    buf_[head_] = item;
    head_ = next;
    return true;
  }
  bool pop(T& item) {
    if (head_ == tail_) return false;  // empty
    item = buf_[tail_];
    tail_ = (tail_ + 1) % N;
    return true;
  }
  bool empty() const { return head_ == tail_; }

 private:
  T buf_[N];
  volatile uint8_t head_ = 0;
  volatile uint8_t tail_ = 0;
};

struct WeightSample {
  float value;
  uint32_t ts;
};

// --- Global state flags (replace FreeRTOS EventGroup) ---
extern bool g_wifiConnected;
extern bool g_ntpSynced;
extern bool g_wifiApMode;

// Buzzer pattern command
struct BuzzerPattern {
  uint16_t freq;          // Hz
  uint16_t durationMs;    // tone duration per beep
  uint8_t count;          // number of beeps
  uint16_t gapMs;         // gap between beeps
  uint32_t initialDelay;  // delay before first beep
};

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

// Queues
extern RingBuffer<EventMessage, 32> g_eventQueue;
extern RingBuffer<BuzzerPattern, 4> g_buzzerQueue;

// Emit functions
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

/// @brief Initialize persistence layer. Call once from setup().
void initCooperativeLoop();
/// @brief Read HX711 at 10 Hz, moving average, emit weight events. Interval:
/// 100ms.
void tickSensor();
/// @brief Run filling FSM (handleWeightAndServo), emit status changes. Every
/// loop.
void tickServo();
/// @brief WiFi reconnect, AP fallback, NTP sync, MQTT loop. Interval: 500ms.
void tickWiFi();
/// @brief Non-blocking buzzer pattern player. Interval: 20ms.
void tickBuzzer();
/// @brief Drain event queue and broadcast JSON to WebSocket clients. Every
/// loop.
void tickWsDispatch();
/// @brief Persistence tick, OTA validation, heap stats. Interval: 1000ms.
void tickHousekeeping();
/// @brief Non-blocking NTP sync FSM. Called from tickWiFi().
void tickNTP();

bool enqueueBuzzerPattern(uint16_t freq, uint16_t durationMs, uint8_t count,
                          uint16_t gapMs, uint32_t initialDelay = 0);
// Expose snapshot sender for WebServerX websocket onConnect handler
void sendSnapshotInternal();

#endif  // SRC_COOPERATIVE_LOOP_H_
