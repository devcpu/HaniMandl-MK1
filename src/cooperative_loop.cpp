/*
 * Copyright (c) 2025 - 2026 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /cooperative_loop.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description: Cooperative main-loop tick functions, event system, NTP,
 * timezone
 * -----
 * Created Date: 2026-03-06
 * Author: Johannes G.  Arlt (janusz)
 */

#include "cooperative_loop.h"

#include <ArduinoJson.h>
#include <Glass.h>
#include <HMConfig.h>
#include <HX711.h>
#include <MQTTHelper.h>
#include <WebServerX.h>  // for AsyncWebSocketClient definition
#include <WiFi.h>
#include <appconfig.h>
#include <esp_ota_ops.h>
#include <handleServo.h>
#include <loadcell.h>  // safeGetUnits prototype
#include <math.h>

#include "persistence.h"  // glass_count persistence

// Global state flags (replace FreeRTOS EventGroup)
bool g_wifiConnected = false;
bool g_ntpSynced = false;
bool g_wifiApMode = false;

// Queues
RingBuffer<EventMessage, 32> g_eventQueue;
RingBuffer<BuzzerPattern, 4> g_buzzerQueue;

// extern globals from main
extern HX711 scale;                         // provided in main.cpp
extern float weight_current;                // global cache in main.cpp
extern Glass glass;                         // glass state tracker from main.cpp
extern AsyncWebSocketClient* globalClient;  // from WebServerX.cpp

// Forward declarations
static bool pushEvent(const EventMessage& ev);
static void buildSnapshot(String& out);

// expose last timeout streak (updated in tickSensor)
static uint32_t g_lastScaleTimeoutStreak = 0;

// ---------------- Timezone Lookup -----------------
/**
 * @brief Mapping from UTC offset (minutes) to POSIX TZ string.
 * Includes DST rules for zones that commonly observe it.
 */
struct TzEntry {
  int16_t offsetMin;  // UTC offset in minutes
  const char* posix;  // POSIX TZ string
};

static const TzEntry TZ_TABLE[] = {
    {-720, "UTC+12"},                       // UTC-12  (Baker Island)
    {-660, "UTC+11"},                       // UTC-11  (Samoa)
    {-600, "UTC+10"},                       // UTC-10  (Hawaii)
    {-540, "AKST9AKDT,M3.2.0,M11.1.0"},     // UTC-9   (Alaska)
    {-480, "PST8PDT,M3.2.0,M11.1.0"},       // UTC-8   (Pacific)
    {-420, "MST7MDT,M3.2.0,M11.1.0"},       // UTC-7   (Mountain)
    {-360, "CST6CDT,M3.2.0,M11.1.0"},       // UTC-6   (Central US)
    {-300, "EST5EDT,M3.2.0,M11.1.0"},       // UTC-5   (Eastern US)
    {-240, "AST4ADT,M3.2.0,M11.1.0"},       // UTC-4   (Atlantic)
    {-180, "UTC+3"},                        // UTC-3   (Argentina)
    {-120, "UTC+2"},                        // UTC-2   (South Georgia)
    {-60, "UTC+1"},                         // UTC-1   (Azores)
    {0, "UTC0"},                            // UTC+0   (GMT/UTC)
    {60, "CET-1CEST,M3.5.0,M10.5.0/3"},     // UTC+1   (Central Europe)
    {120, "EET-2EEST,M3.5.0/3,M10.5.0/4"},  // UTC+2   (Eastern Europe)
    {180, "MSK-3"},                         // UTC+3   (Moscow)
    {240, "UTC-4"},                         // UTC+4   (Gulf/Dubai)
    {300, "PKT-5"},                         // UTC+5   (Pakistan)
    {330, "IST-5:30"},                      // UTC+5:30 (India)
    {360, "UTC-6"},                         // UTC+6   (Bangladesh/Dhaka)
    {420, "ICT-7"},                         // UTC+7   (Indochina)
    {480, "CST-8"},                         // UTC+8   (China)
    {540, "JST-9"},                         // UTC+9   (Japan)
    {600, "AEST-10AEDT,M10.1.0,M4.1.0/3"},  // UTC+10  (Australia East)
    {660, "UTC-11"},                        // UTC+11  (Solomon Islands)
    {720, "NZST-12NZDT,M9.5.0,M4.1.0/3"},   // UTC+12  (New Zealand)
    {780, "UTC-13"},                        // UTC+13  (Tonga)
    {840, "UTC-14"},                        // UTC+14  (Line Islands)
};
static constexpr uint8_t TZ_TABLE_SIZE = sizeof(TZ_TABLE) / sizeof(TZ_TABLE[0]);

/**
 * @brief Get POSIX TZ string for a given UTC offset in minutes.
 * @param offsetMin UTC offset in minutes (-720 to +840)
 * @return POSIX TZ string, defaults to "UTC0" for unknown offsets
 */
static const char* getTzString(int16_t offsetMin) {
  for (uint8_t i = 0; i < TZ_TABLE_SIZE; ++i) {
    if (TZ_TABLE[i].offsetMin == offsetMin) return TZ_TABLE[i].posix;
  }
  return "UTC0";
}

// ---------------- NTP FSM -----------------
namespace {
enum class NTPState { Idle, Request, Wait, Failed };
NTPState ntpState = NTPState::Idle;
uint32_t ntpRef = 0;
uint8_t ntpRetries = 0;
struct tm timeinfo;

void applyNtpConfig() {
  HMConfig& cfg = HMConfig::instance();
  const char* tz = getTzString(cfg.utc_offset);
  configTzTime(tz, cfg.ntp_server, "pool.ntp.org");
  log_i("[NTP] TZ=%s (UTC%+d min) server=%s", tz, cfg.utc_offset,
        cfg.ntp_server);
}
}  // namespace

void tickNTP() {
  switch (ntpState) {
    case NTPState::Idle:
      log_i("[NTP] Starting sync");
      applyNtpConfig();
      ntpRef = millis();
      ntpRetries = 0;
      ntpState = NTPState::Wait;
      break;
    case NTPState::Wait:
      if (getLocalTime(&timeinfo)) {
        char buffer[32];  // flawfinder: ignore
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
        log_i("[NTP] Synced successfully: %s", buffer);
        strlcpy(HMConfig::instance().date_filling, buffer,
                sizeof(HMConfig::instance().date_filling));
        g_ntpSynced = true;
        emitNtpSynced();
        ntpState = NTPState::Idle;
      } else if (millis() - ntpRef >= NTP_WAIT_MS) {
        if (++ntpRetries >= 10) {
          log_w("[NTP] Failed after 10 retries, will retry in 5 minutes");
          ntpState = NTPState::Failed;
          ntpRef = millis();
        } else {
          log_d("[NTP] Retry %d/10", ntpRetries);
          ntpState = NTPState::Request;
        }
      }
      break;
    case NTPState::Request:
      applyNtpConfig();
      ntpRef = millis();
      ntpState = NTPState::Wait;
      break;
    case NTPState::Failed:
      if (millis() - ntpRef >= NTP_RETRY_INTERVAL_MS) {
        log_i("[NTP] Retrying after cooldown period");
        ntpState = NTPState::Idle;
      }
      break;
  }
}

// --------------- Tick Functions --------------------

void tickSensor() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick < 100) return;
  lastTick = millis();

  // Moving average (small FIFO) for smoothing
  static float maBuf[MA_WINDOW] = {0};
  static uint8_t maIndex = 0;
  static uint8_t maCount = 0;

  // Timeout / not-ready streak tracking
  static uint32_t consecutiveTimeouts = 0;

  if (scale.is_ready()) {
    float v = safeGetUnits(1, 50);  // short guarded read
    if (!isnan(v)) {
      // Success: reset timeout streak
      if (consecutiveTimeouts != 0) {
        consecutiveTimeouts = 0;
        g_lastScaleTimeoutStreak = 0;
      }

      // Insert into moving average buffer
      maBuf[maIndex] = v;
      maIndex = (maIndex + 1) % MA_WINDOW;
      if (maCount < MA_WINDOW) maCount++;

      // Compute simple average
      float sum = 0.f;
      for (uint8_t i = 0; i < maCount; ++i) sum += maBuf[i];
      float avg = sum / maCount;

      weight_current = avg;
      HMConfig::instance().weight_current = static_cast<int16_t>(avg);
      emitWeight(avg);
    } else {
      consecutiveTimeouts++;
      g_lastScaleTimeoutStreak = consecutiveTimeouts;
      if (consecutiveTimeouts % TIMEOUT_EVENT_THRESHOLD == 0) {
        emitScaleTimeout(consecutiveTimeouts);
      }
    }
  } else {
    consecutiveTimeouts++;
    g_lastScaleTimeoutStreak = consecutiveTimeouts;
    if (consecutiveTimeouts % TIMEOUT_EVENT_THRESHOLD == 0) {
      emitScaleTimeout(consecutiveTimeouts);
    }
  }
}

void tickServo() {
  HMConfig& cfg = HMConfig::instance();
  auto beforeFS = cfg.fs;
  auto beforeRM = cfg.run_modus;
  handleWeightAndServo(weight_current);
  if (cfg.fs != beforeFS) {
    emitFillingStatus(static_cast<uint8_t>(cfg.fs));
  }
  if (cfg.run_modus != beforeRM) {
    emitRunMode(static_cast<uint8_t>(cfg.run_modus));
  }
}

void tickWiFi() {
  static uint32_t lastTick = 0;
  static bool initialized = false;
  static uint32_t wifiStart = 0;
  static bool wifiConnecting = false;
  static uint32_t lastReconnectAttempt = 0;
  static bool apMode = false;
  const uint32_t WIFI_TIMEOUT_MS = 15000;
  const uint32_t WIFI_RETRY_INTERVAL_MS = 30000;

  // One-time initialization on first call
  if (!initialized) {
    initialized = true;
    if (WiFi.status() == WL_CONNECTED) {
      log_i("[WiFi] Already connected at start: %s",
            WiFi.localIP().toString().c_str());
      g_wifiConnected = true;
      strlcpy(HMConfig::instance().localIP, WiFi.localIP().toString().c_str(),
              sizeof(HMConfig::instance().localIP));
      emitWifiState(true, false);
      tickNTP();
    } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
      log_i("[WiFi] Running in AP mode - NTP will not be available");
      apMode = true;
      g_wifiApMode = true;
      emitWifiState(false, true);
    }
  }

  // No SSID configured -> stay in AP mode, no STA retries
  if (strlen(WIFI_SSID) == 0) return;

  if (millis() - lastTick < 500) return;
  lastTick = millis();

  bool connected = WiFi.status() == WL_CONNECTED;

  if (apMode) {
    if (millis() - lastReconnectAttempt > WIFI_RETRY_INTERVAL_MS) {
      log_i("[WiFi] AP mode retry STA connect...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      wifiStart = millis();
      wifiConnecting = true;
      apMode = false;
      lastReconnectAttempt = millis();
    }
    return;
  }

  if (!wifiConnecting && !connected) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiStart = millis();
    wifiConnecting = true;
    log_i("[WiFi] Trying to connect to STA...");
  }

  if (wifiConnecting) {
    if (connected) {
      g_wifiConnected = true;
      wifiConnecting = false;
      lastReconnectAttempt = millis();
      log_i("[WiFi] Connected: %s", WiFi.localIP().toString().c_str());
      strlcpy(HMConfig::instance().localIP, WiFi.localIP().toString().c_str(),
              sizeof(HMConfig::instance().localIP));
      emitWifiState(true, false);
      tickNTP();
    } else if (millis() - wifiStart > WIFI_TIMEOUT_MS) {
      log_w("[WiFi] STA connect timeout, starting AP mode.");
      wifiConnecting = false;
      WiFi.mode(WIFI_AP_STA);
      if (WiFi.softAP(WIFI_AP_NAME, WIFI_AP_PASSWORD, 1, 0, 1)) {
        g_wifiApMode = true;
        apMode = true;
        log_i("[WiFi] AP mode active SSID=%s", WIFI_AP_NAME);
        emitWifiState(false, true);
      }
      lastReconnectAttempt = millis();
    }
  } else if (connected) {
    // Maintain NTP sync attempts
    if (g_wifiConnected && !g_ntpSynced) {
      tickNTP();
    } else if (g_ntpSynced) {
      static uint32_t lastNtpOkLog = 0;
      if (millis() - lastNtpOkLog > 60000) {
        log_d("[NTP] Already synced, time=%ld", (long)time(nullptr));
        lastNtpOkLog = millis();
      }
    }
    // Process MQTT loop when connected
    MQTTHelper::instance().loop();
  } else {
    if (millis() - lastReconnectAttempt > WIFI_RETRY_INTERVAL_MS) {
      lastReconnectAttempt = millis();
    }
  }
}

// Buzzer tick: consumes patterns and plays them non-blocking
void tickBuzzer() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick < 20) return;
  lastTick = millis();

  static BuzzerPattern pat;
  static uint8_t currentCount = 0;
  static uint32_t tRef = 0;
  enum class BState { Idle, InitDelay, Beep, Gap };
  static BState state = BState::Idle;

  switch (state) {
    case BState::Idle:
      if (g_buzzerQueue.pop(pat)) {
        currentCount = 0;
        if (pat.initialDelay) {
          tRef = millis();
          state = BState::InitDelay;
        } else {
          state = BState::Beep;
        }
      }
      break;
    case BState::InitDelay:
      if (millis() - tRef >= pat.initialDelay) {
        state = BState::Beep;
      }
      break;
    case BState::Beep:
      tone(PIN_BUZZER, pat.freq, pat.durationMs);
      tRef = millis();
      state = BState::Gap;
      break;
    case BState::Gap:
      if (millis() - tRef >= pat.durationMs + pat.gapMs) {
        if (++currentCount >= pat.count) {
          state = BState::Idle;
        } else {
          state = BState::Beep;
        }
      }
      break;
  }
}

bool enqueueBuzzerPattern(uint16_t freq, uint16_t durationMs, uint8_t count,
                          uint16_t gapMs, uint32_t initialDelay) {
  BuzzerPattern pat{freq, durationMs, count, gapMs, initialDelay};
  return g_buzzerQueue.push(pat);
}

// --------------- Event Push System -----------------
volatile uint32_t g_eventQueueOverflows = 0;

static bool pushEvent(const EventMessage& ev) {
  if (!g_eventQueue.push(ev)) {
    g_eventQueueOverflows++;
    if ((g_eventQueueOverflows & 0xFF) == 1) {
      log_w("Event queue overflow count=%lu",
            (unsigned long)g_eventQueueOverflows);
    }
    return false;
  }
  return true;
}

bool emitWeight(float w) {
  static uint32_t lastEmitTime = 0;
  uint32_t now = millis();
  if (now - lastEmitTime < EMIT_INTERVAL_MS) return false;

  lastEmitTime = now;
  EventMessage ev{EventType::WeightDelta, now};
  ev.data.fValue = w;
  return pushEvent(ev);
}

bool emitFillingStatus(uint8_t fs) {
  static uint8_t last = 0xFF;
  if (fs == last) return false;
  last = fs;
  EventMessage ev{EventType::FillingStatusChange, millis()};
  ev.data.u8 = fs;
  return pushEvent(ev);
}

bool emitRunMode(uint8_t rm) {
  static uint8_t last = 0xFF;
  if (rm == last) return false;
  last = rm;
  EventMessage ev{EventType::RunModeChange, millis()};
  ev.data.u8 = rm;
  return pushEvent(ev);
}

bool emitWifiState(bool connected, bool apMode) {
  EventMessage ev{EventType::WifiState, millis()};
  ev.data.u8 = (connected ? 0x1 : 0) | (apMode ? 0x2 : 0);
  return pushEvent(ev);
}

bool emitNtpSynced() {
  static bool once = false;
  if (once) return false;
  once = true;
  EventMessage ev{EventType::NtpSynced, millis()};
  ev.data.u8 = 1;
  return pushEvent(ev);
}

bool emitGlassCount(uint32_t gc) {
  EventMessage ev{EventType::GlassCount, millis()};
  ev.data.iValue = static_cast<int32_t>(gc);
  return pushEvent(ev);
}

bool emitHeapStats() {
  EventMessage ev{EventType::HeapStats, millis()};
  ev.data.iValue = static_cast<int32_t>(ESP.getFreeHeap());
  return pushEvent(ev);
}

bool emitScaleTimeout(uint32_t consecutiveTimeouts) {
  EventMessage ev{EventType::ScaleTimeout, millis()};
  ev.data.iValue = static_cast<int32_t>(consecutiveTimeouts);
  return pushEvent(ev);
}

/**
 * @brief Build a full state snapshot for (new) WebSocket clients.
 */
static void buildSnapshot(String& out) {
  StaticJsonDocument<512> doc;
  HMConfig& c = HMConfig::instance();
  doc["type"] = "snapshot";
  doc["version"] = c.version;
  doc["run_modus"] = c.runmod2string(c.run_modus);
  doc["filling_status"] = c.fillingstatus2string(c.fs);
  doc["weight_current"] = static_cast<int>(c.weight_current);
  doc["weight_honey"] = static_cast<int>(c.weight_honey);
  doc["weight_filling"] = c.weight_filling;
  doc["weight_fine"] = c.weight_fine;
  doc["glass_empty"] = c.glass_empty;
  doc["glass_count"] = c.glass_count;
  doc["date_filling"] = c.date_filling;
  doc["batch_number"] = c.batch_number;
  JsonObject servo = doc.createNestedObject("servo");
  servo["min"] = c.servodata.angle_min;
  servo["max"] = c.servodata.angle_max;
  servo["fine"] = c.servodata.angle_fine;
  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["connected"] = g_wifiConnected;
  wifi["ap_mode"] = g_wifiApMode;
  wifi["ip"] = c.localIP;
  doc["scale_timeout_streak"] = g_lastScaleTimeoutStreak;
  doc["glass_on_scale"] = !glass.isNoGlass();
  serializeJson(doc, out);
}

extern void broadcastText(const String& msg);  // from WebServerX.cpp

void sendSnapshotInternal() {
  String payload;
  buildSnapshot(payload);
  broadcastText(payload);
}

/**
 * @brief Drain event queue and broadcast UI delta events.
 * Processes max 8 events per call. Heartbeat every 10s.
 */
void tickWsDispatch() {
  static uint32_t lastHeartbeat = millis();
  EventMessage ev;
  uint8_t processed = 0;
  while (processed < 8 && g_eventQueue.pop(ev)) {
    processed++;
    if (wsClientCount() == 0) continue;
    StaticJsonDocument<192> doc;
    switch (ev.type) {
      case EventType::WeightDelta:
        doc["t"] = "w";
        doc["v"] = ev.data.fValue;
        doc["h"] = static_cast<int>(HMConfig::instance().weight_honey);
        doc["glass_on_scale"] = !glass.isNoGlass();
        break;
      case EventType::FillingStatusChange:
        doc["t"] = "fs";
        doc["v"] = HMConfig::instance().fillingstatus2string(
            static_cast<FillingStatus>(ev.data.u8));
        break;
      case EventType::RunModeChange:
        doc["t"] = "rm";
        doc["v"] = HMConfig::instance().runmod2string(
            static_cast<RunModus>(ev.data.u8));
        break;
      case EventType::WifiState: {
        bool conn = ev.data.u8 & 0x1;
        bool ap = ev.data.u8 & 0x2;
        doc["t"] = "ws";
        doc["connected"] = conn;
        doc["ap"] = ap;
        break;
      }
      case EventType::NtpSynced:
        doc["t"] = "ntp";
        doc["synced"] = true;
        {
          char batch_buf[16];  // flawfinder: ignore
          if (HMConfig::instance().getBatchNumber(batch_buf,
                                                  sizeof(batch_buf))) {
            doc["batch_number"] = batch_buf;
          }
        }
        break;
      case EventType::GlassCount:
        doc["t"] = "gc";
        doc["v"] = HMConfig::instance().glass_count;
        break;
      case EventType::HeapStats:
        doc["t"] = "hs";
        doc["free"] = ESP.getFreeHeap() / 1024;
        doc["min"] = ESP.getMinFreeHeap() / 1024;
        doc["alloc"] = ESP.getMaxAllocHeap() / 1024;
        break;
      case EventType::ScaleTimeout:
        doc["t"] = "st";
        doc["v"] = ev.data.iValue;
        break;
      case EventType::Heartbeat:
        doc["t"] = "hb";
        doc["ts"] = ev.ts;
        break;
    }
    String out;
    serializeJson(doc, out);
    broadcastText(out);
  }

  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL_MS) {
    EventMessage hb{EventType::Heartbeat, millis()};
    pushEvent(hb);
    lastHeartbeat = millis();
  }
}

/**
 * @brief OTA rollback validation, called once after 10s uptime.
 * Non-blocking replacement for the former delay(10000) in setup().
 */
static void otaValidateOnce() {
  static bool done = false;
  if (done || millis() < 10000) return;
  done = true;

  const esp_partition_t* running = esp_ota_get_running_partition();
  esp_ota_img_states_t ota_state;
  if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK) return;

  if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
    log_i("OTA: First boot after update - validating firmware...");
    bool systemsOK = true;

    if (WiFi.status() == WL_NO_SHIELD || WiFi.getMode() == WIFI_MODE_NULL) {
      log_e("OTA Validation FAILED: WiFi system not initialized");
      systemsOK = false;
    } else {
      log_i("OTA Validation: WiFi stack OK (connected=%d)",
            WiFi.status() == WL_CONNECTED);
    }

    if (systemsOK) {
      log_i("OTA: Firmware validation PASSED - marking as valid");
      esp_ota_mark_app_valid_cancel_rollback();
    } else {
      log_e("OTA: Firmware validation FAILED - rollback on next reboot!");
    }
  } else if (ota_state == ESP_OTA_IMG_VALID) {
    log_i("OTA: Running validated firmware (partition: %s)", running->label);
  }
}

void tickHousekeeping() {
  static uint32_t lastTick = 0;
  static uint32_t lastHeap = 0;

  if (millis() - lastTick < 1000) return;
  lastTick = millis();

  otaValidateOnce();
  Persistence::persistenceTick();

  if (millis() - lastHeap >= 60000) {
    emitHeapStats();
    lastHeap = millis();
  }
}

void initCooperativeLoop() { Persistence::init(); }
