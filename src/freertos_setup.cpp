#include "freertos_setup.h"

#include <HMConfig.h>
#include <HX711.h>
#include <WiFi.h>
#include <handleServo.h>

QueueHandle_t qWeight = nullptr;
EventGroupHandle_t egSystem = nullptr;

TaskHandle_t taskSensorHandle = nullptr;
TaskHandle_t taskServoHandle = nullptr;
TaskHandle_t taskWiFiHandle = nullptr;

// extern globals from main
extern HX711 scale;  // provided in main.cpp

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
  const TickType_t period = pdMS_TO_TICKS(100);  // 10 Hz
  TickType_t last = xTaskGetTickCount();
  WeightSample sample;
  for (;;) {
    sample.value = scale.get_units(1);
    sample.ts = millis();
    if (qWeight) {
      // overwrite pattern: send latest (queue length = 1)
      xQueueOverwrite(qWeight, &sample);
    }
    vTaskDelayUntil(&last, period);
  }
}

static void servoTask(void* p) {
  WeightSample sample;
  for (;;) {
    if (qWeight &&
        xQueueReceive(qWeight, &sample, pdMS_TO_TICKS(50)) == pdPASS) {
      handleWeightAndServo(sample.value);
    } else {
      // still call servo logic with last known value? optional
    }
    taskYIELD();
  }
}

static void wifiTask(void* p) {
  // Non-blocking WiFi connect attempt
  static uint32_t wifiStart = 0;
  static bool wifiConnecting = false;
  const uint32_t WIFI_TIMEOUT_MS = 15000;

  for (;;) {
    if (!wifiConnecting && WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      wifiStart = millis();
      wifiConnecting = true;
    }
    if (wifiConnecting) {
      if (WiFi.status() == WL_CONNECTED) {
        xEventGroupSetBits(egSystem, EV_WIFI_CONNECTED);
        wifiConnecting = false;
        // trigger NTP sync start
        tickNTP();
      } else if (millis() - wifiStart > WIFI_TIMEOUT_MS) {
        // timeout -> could fallback to AP or retry later
        wifiConnecting = false;  // let loop restart after short delay
      }
    } else {
      // if connected and NTP not yet
      EventBits_t bits = xEventGroupGetBits(egSystem);
      if ((bits & EV_WIFI_CONNECTED) && !(bits & EV_NTP_SYNCED)) {
        tickNTP();
      }
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

void startSystemTasks() {
  if (!qWeight) qWeight = xQueueCreate(1, sizeof(WeightSample));
  if (!egSystem) egSystem = xEventGroupCreate();

  xTaskCreatePinnedToCore(sensorTask, "sensorTask", 4096, nullptr, 2,
                          &taskSensorHandle, 0);
  xTaskCreatePinnedToCore(servoTask, "servoTask", 4096, nullptr, 3,
                          &taskServoHandle, 1);
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 4096, nullptr, 2,
                          &taskWiFiHandle, 0);
}
