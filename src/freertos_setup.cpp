#include "freertos_setup.h"

#include <ArduinoJson.h>
#include <HMConfig.h>
#include <HX711.h>
#include <WebServerX.h>  // for AsyncWebSocketClient definition
#include <WiFi.h>
#include <appconfig.h>
#include <handleServo.h>
#include <loadcell.h>  // safeGetUnits prototype
#include <math.h>

#include "persistence.h"  // glass_count persistence

QueueHandle_t qWeight = nullptr;
QueueHandle_t qBuzzer = nullptr;
QueueHandle_t qEvents = nullptr;  // event queue for websocket push
EventGroupHandle_t egSystem = nullptr;

TaskHandle_t taskSensorHandle = nullptr;
TaskHandle_t taskServoHandle = nullptr;
TaskHandle_t taskWiFiHandle = nullptr;
TaskHandle_t taskBuzzerHandle = nullptr;
TaskHandle_t taskWsDispatchHandle = nullptr;

// extern globals from main
extern HX711 scale;                         // provided in main.cpp
extern float weight_current;                // global cache in main.cpp
extern AsyncWebSocketClient* globalClient;  // from WebServerX.cpp

// Forward declarations for event push helpers
/* EventMessage is a C++ structure that represents an event with
a specific type (EventType),
a timestamp (ts), and
associated data stored in a union.
The union allows the event to hold one of several data types:
  a floating-point value (fValue),
  a 32-bit integer (iValue), or an
  8-bit unsigned integer (u8).
*/
static bool pushEvent(const EventMessage& ev);
bool emitWeight(float w);
bool emitFillingStatus(uint8_t fs);
bool emitRunMode(uint8_t rm);
bool emitWifiState(bool connected, bool apMode);
bool emitNtpSynced();
bool emitGlassCount(uint32_t gc);
bool emitHeapStats();
bool emitScaleTimeout(uint32_t consecutiveTimeouts);
static void wsDispatchTask(void* p);
static void buildSnapshot(String& out);
void sendSnapshotInternal();

// expose last timeout streak (updated in sensorTask)
static uint32_t g_lastScaleTimeoutStreak =
    0;  // current consecutive timeout streak

// ---------------- NTP FSM -----------------
namespace {
enum class NTPState { Idle, Request, Wait };
NTPState ntpState = NTPState::Idle;
uint32_t ntpRef = 0;
uint8_t ntpRetries = 0;
struct tm timeinfo;
const uint32_t NTP_WAIT_MS = 2000;  // previously blocking delay(2000)
}  // namespace

void tickNTP() {
  switch (ntpState) {
    case NTPState::Idle:
      configTime(3600, 3600, "pool.ntp.org");  // TODO: move to config
      ntpRef = millis();
      ntpState = NTPState::Wait;
      break;
    case NTPState::Wait:
      if (getLocalTime(&timeinfo)) {
        // success
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
        strlcpy(HMConfig::instance().date_filling, buffer,
                sizeof(HMConfig::instance().date_filling));
        xEventGroupSetBits(egSystem, EV_NTP_SYNCED);
        emitNtpSynced();
        ntpState = NTPState::Idle;  // could stay synced
      } else if (millis() - ntpRef >= NTP_WAIT_MS) {
        if (++ntpRetries >= 10) {
          ntpState = NTPState::Idle;  // give up for now
        } else {
          ntpState = NTPState::Request;  // retry
        }
      }
      break;
    case NTPState::Request:
      configTime(3600, 3600, "pool.ntp.org");
      ntpRef = millis();
      ntpState = NTPState::Wait;
      break;
  }
}

// --------------- Tasks --------------------
static void sensorTask(void* p) {
  // Periodic polling at 10 Hz
  const TickType_t period = pdMS_TO_TICKS(100);
  TickType_t last = xTaskGetTickCount();
  WeightSample sample;

  // Moving average (small FIFO) for smoothing
  constexpr uint8_t MA_WINDOW = 5;  // kleiner FIFO
  float maBuf[MA_WINDOW] = {0};
  uint8_t maIndex = 0;
  uint8_t maCount = 0;  // number of valid entries

  // Timeout / not-ready streak tracking
  uint32_t consecutiveTimeouts = 0;
  constexpr uint32_t TIMEOUT_EVENT_THRESHOLD =
      5;  // emit every 5 consecutive failures

  for (;;) {
    bool gotSample = false;

    if (scale.is_ready()) {
      float v = safeGetUnits(1, 50);  // short guarded read
      if (!isnan(v)) {
        // Success: reset timeout streak
        if (consecutiveTimeouts != 0) {
          consecutiveTimeouts = 0;
          g_lastScaleTimeoutStreak = 0;  // expose reset
        }

        // Insert into moving average buffer
        maBuf[maIndex] = v;
        maIndex = (maIndex + 1) % MA_WINDOW;
        if (maCount < MA_WINDOW) maCount++;

        // Compute simple average
        float sum = 0.f;
        for (uint8_t i = 0; i < maCount; ++i) sum += maBuf[i];
        float avg = sum / maCount;

        sample.value = avg;
        sample.ts = millis();
        if (qWeight) {
          xQueueOverwrite(qWeight, &sample);
        }
        weight_current = sample.value;
        HMConfig::instance().weight_current =
            static_cast<int16_t>(sample.value);
        emitWeight(sample.value);  // emit smoothed value
        gotSample = true;
      } else {
        // Reading timed out despite is_ready() earlier (rare but guarded)
        consecutiveTimeouts++;
        g_lastScaleTimeoutStreak = consecutiveTimeouts;
        if (consecutiveTimeouts % TIMEOUT_EVENT_THRESHOLD == 0) {
          emitScaleTimeout(consecutiveTimeouts);
        }
      }
    } else {
      // HX711 not ready this cycle -> treat as missed sample (counts toward
      // streak)
      consecutiveTimeouts++;
      g_lastScaleTimeoutStreak = consecutiveTimeouts;
      if (consecutiveTimeouts % TIMEOUT_EVENT_THRESHOLD == 0) {
        emitScaleTimeout(consecutiveTimeouts);
      }
    }

    (void)gotSample;  // reserved for potential adaptive scheduling

    vTaskDelayUntil(&last, period);
  }
}

static void servoTask(void* p) {
  WeightSample sample;
  for (;;) {
    if (qWeight &&
        xQueueReceive(qWeight, &sample, pdMS_TO_TICKS(50)) == pdPASS) {
      HMConfig& cfg = HMConfig::instance();
      auto beforeFS = cfg.fs;
      auto beforeRM = cfg.run_modus;
      handleWeightAndServo(sample.value);
      if (cfg.fs != beforeFS) {
        emitFillingStatus(static_cast<uint8_t>(cfg.fs));
      }
      if (cfg.run_modus != beforeRM) {
        emitRunMode(static_cast<uint8_t>(cfg.run_modus));
      }
    } else {
      // still call servo logic with last known value? optional
    }
    taskYIELD();
  }
}

static void wifiTask(void* p) {
  // Non-blocking WiFi + AP failover + periodic reconnect attempts
  static uint32_t wifiStart = 0;
  static bool wifiConnecting = false;
  static uint32_t lastReconnectAttempt = 0;
  static bool apMode = false;
  const uint32_t WIFI_TIMEOUT_MS = 15000;
  const uint32_t WIFI_RETRY_INTERVAL_MS = 30000;  // retry every 30s if failed

  for (;;) {
    EventBits_t bits = xEventGroupGetBits(egSystem);
    bool connected = WiFi.status() == WL_CONNECTED;

    if (apMode) {
      // In AP mode: periodically try STA reconnect without dropping AP
      if (millis() - lastReconnectAttempt > WIFI_RETRY_INTERVAL_MS) {
        log_i("[WiFi] AP mode retry STA connect...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        wifiStart = millis();
        wifiConnecting = true;
        apMode = false;  // try to leave AP if STA connects
        lastReconnectAttempt = millis();
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    if (!wifiConnecting && !connected) {
      // Initiate STA connection
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      wifiStart = millis();
      wifiConnecting = true;
      log_i("[WiFi] Trying to connect to STA...");
    }

    if (wifiConnecting) {
      if (connected) {
        xEventGroupSetBits(egSystem, EV_WIFI_CONNECTED);
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
        // Limit AP to 1 client (channel 1, hidden=0, max_conn=1)
        if (WiFi.softAP(WIFI_AP_NAME, WIFI_AP_PASSWORD, 1, 0, 1)) {
          xEventGroupSetBits(egSystem, EV_WIFI_AP_MODE);
          apMode = true;
          log_i("[WiFi] AP mode active SSID=%s", WIFI_AP_NAME);
          emitWifiState(false, true);
        }
        lastReconnectAttempt = millis();
      }
    } else if (connected) {
      // Maintain NTP sync attempts
      if ((bits & EV_WIFI_CONNECTED) && !(bits & EV_NTP_SYNCED)) {
        tickNTP();
      }
    } else {
      // idle failed state, wait for retry interval
      if (millis() - lastReconnectAttempt > WIFI_RETRY_INTERVAL_MS) {
        // force new attempt
        lastReconnectAttempt = millis();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// Buzzer task: consumes patterns and plays them non-blocking
static void buzzerTask(void* p) {
  BuzzerPattern pat;
  uint8_t currentCount = 0;
  uint32_t tRef = 0;
  enum class BState { Idle, InitDelay, Beep, Gap } state = BState::Idle;
  for (;;) {
    switch (state) {
      case BState::Idle:
        if (qBuzzer &&
            xQueueReceive(qBuzzer, &pat, pdMS_TO_TICKS(50)) == pdPASS) {
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
            state = BState::Idle;  // done
          } else {
            state = BState::Beep;  // next beep
          }
        }
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

bool enqueueBuzzerPattern(uint16_t freq, uint16_t durationMs, uint8_t count,
                          uint16_t gapMs, uint32_t initialDelay) {
  if (!qBuzzer) return false;
  BuzzerPattern pat{freq, durationMs, count, gapMs, initialDelay};
  return xQueueSend(qBuzzer, &pat, 0) == pdPASS;
}

// --------------- Event Push System -----------------
volatile uint32_t g_eventQueueOverflows =
    0;  // exposed for logging in WebServerX
static bool pushEvent(const EventMessage& ev) {
  if (!qEvents) return false;
  if (xQueueSend(qEvents, &ev, 0) != pdPASS) {
    g_eventQueueOverflows++;
    if ((g_eventQueueOverflows & 0xFF) == 1) {  // log gelegentlich
      log_w("Event queue overflow count=%lu",
            (unsigned long)g_eventQueueOverflows);
    }
    return false;
  }
  return true;
}

bool emitWeight(float w) {
  static uint32_t lastEmitTime = 0;
  const uint32_t EMIT_INTERVAL_MS = 1000;  // 1 second interval

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
 *
 * Contains current weight and configuration data plus WiFi status. Used for
 * initial bootstrap and explicit re-sends.
 * @param out Target JSON string (will be overwritten).
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
  doc["los_number"] = c.los_number;
  JsonObject servo = doc.createNestedObject("servo");
  servo["min"] = c.servodata.angle_min;
  servo["max"] = c.servodata.angle_max;
  servo["fine"] = c.servodata.angle_fine;
  JsonObject wifi = doc.createNestedObject("wifi");
  EventBits_t bits = xEventGroupGetBits(egSystem);
  wifi["connected"] = (bits & EV_WIFI_CONNECTED) != 0;
  wifi["ap_mode"] = (bits & EV_WIFI_AP_MODE) != 0;
  wifi["ip"] = c.localIP;
  doc["scale_timeout_streak"] = g_lastScaleTimeoutStreak;
  serializeJson(doc, out);
}

extern void broadcastText(const String& msg);  // from WebServerX.cpp
/**
 * @brief Broadcast the current snapshot to all connected WebSocket clients.
 *
 * NOTE: Currently always broadcasts. If a per-client variant is required in
 * the future, add an overload taking a target client pointer.
 */
void sendSnapshotInternal() {
  String payload;
  buildSnapshot(payload);
  broadcastText(payload);
}

/**
 * @brief FreeRTOS task: consumes the event queue and broadcasts UI delta
 * events.
 *
 * Implements a heartbeat (every 10s) and maps internal EventType values to
 * compact JSON packets (key 't'). Minimizes allocation by reusing a small
 * StaticJsonDocument per event.
 */
static void wsDispatchTask(void* p) {
  uint32_t lastHeartbeat = millis();
  const uint32_t HEARTBEAT_INTERVAL_MS =
      10000;  // 10s schneller für Client-Liveness
  for (;;) {
    EventMessage ev;
    if (xQueueReceive(qEvents, &ev, pdMS_TO_TICKS(200)) == pdPASS) {
      // If no clients are connected skip building payload
      // (broadcastText will just iterate zero entries)
      if (wsClientCount() == 0) continue;
      StaticJsonDocument<192> doc;
      switch (ev.type) {
        case EventType::WeightDelta:
          doc["t"] = "w";
          doc["v"] = ev.data.fValue;  // raw (float) weight current (still
                                      // rounded client side)
          doc["h"] = static_cast<int>(
              HMConfig::instance()
                  .weight_honey);  // integer honey weight (netto)
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
    taskYIELD();
  }
}

void startSystemTasks() {
  // Initialize persistence (glass_count)
  // (Safe to call even if already initialized)
  Persistence::init();
  if (!qWeight) qWeight = xQueueCreate(1, sizeof(WeightSample));
  if (!qBuzzer) qBuzzer = xQueueCreate(4, sizeof(BuzzerPattern));
  if (!qEvents) qEvents = xQueueCreate(32, sizeof(EventMessage));
  if (!egSystem) egSystem = xEventGroupCreate();

  xTaskCreatePinnedToCore(sensorTask, "sensorTask", 4096, nullptr, 2,
                          &taskSensorHandle, 0);
  xTaskCreatePinnedToCore(servoTask, "servoTask", 4096, nullptr, 3,
                          &taskServoHandle, 1);
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 4096, nullptr, 2,
                          &taskWiFiHandle, 0);
  xTaskCreatePinnedToCore(buzzerTask, "buzzerTask", 2048, nullptr, 1,
                          &taskBuzzerHandle, 1);
  xTaskCreatePinnedToCore(wsDispatchTask, "wsDispatch", 4096, nullptr, 2,
                          &taskWsDispatchHandle, 0);
  // Housekeeping task: persistence tick every 60s
  xTaskCreatePinnedToCore(
      [](void*) {
        TickType_t last = xTaskGetTickCount();
        uint32_t lastHeap = millis();
        for (;;) {
          Persistence::persistenceTick();
          if (millis() - lastHeap >= 60000) {  // every 60s push heap stats
            emitHeapStats();
            lastHeap = millis();
          }
          vTaskDelayUntil(
              &last,
              pdMS_TO_TICKS(
                  1000));  // check once per second for timing flexibility
        }
      },
      "housekeeping", 3072, nullptr, 1, nullptr, 0);
}
