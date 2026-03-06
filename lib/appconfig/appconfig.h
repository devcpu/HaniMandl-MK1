/*
 * Copyright (c) 2023 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /config.h
Project: simple automatic honey filling machine
 * Description:
 * -----
 * Created Date: 2023-08-12 23:30
 * Author: Johannes G.  Arlt
 * -----
 * Last Modified: 2026-03-06 17:40
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef LIB_APPCONFIG_APPCONFIG_H_
#define LIB_APPCONFIG_APPCONFIG_H_

#include <Arduino.h>

#define PROGRAMM_NAME "Honey Filling Machine"
#define SOFTWARE_VERSION "v 0.4"

// WiFi STA credentials: override via build_flags (-DWIFI_SSID=\"...\")
// or leave empty to start in AP mode for WebUI configuration.
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif
#ifndef WIFI_IP
#define WIFI_IP 0, 0, 0, 0
#endif
#ifndef WIFI_GATEWAY
#define WIFI_GATEWAY 0, 0, 0, 0
#endif
#ifndef WIFI_SUBNET
#define WIFI_SUBNET 255, 255, 255, 0
#endif
#ifndef WIFI_DNS1
#define WIFI_DNS1 0, 0, 0, 0
#endif
#ifndef WIFI_DNS2
#define WIFI_DNS2 8, 8, 8, 8
#endif

/// data if accesspoint
#define WIFI_AP_NAME "Honey Filling Machine"
#define WIFI_AP_PASSWORD "Honigkuchen"

// pin definition
#define PIN_LOADCELL_DOUT 5
#define PIN_LOADCELL_SCK 17
#define PIN_SERVO 33
#define PIN_WIFI_LED 21
#define PIN_BUZZER 16

// Emergency stop button / touch pad
#define PIN_STOP_BTN 27  // GPIO27 also touch channel T7
#define PIN_STOP_TOUCH_CHANNEL T7
#define STOP_DEBOUNCE_MS 15  // mechanical: 30–50ms; touch: 5–15ms

// calc avg from how many times read
#define LOADCELL_READ_TIMES 3

// servo data
#define SERVO_FREQUENCY 300
#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500

#define WEB_INPUT_MAX_LENGTH 32

// Touch sensor threshold for emergency stop (T7/GPIO27).
// Lower = more sensitive. Typical range 20-60, depends on pad geometry.
#define TOUCH_THRESHOLD_DEFAULT 40

// Maximum concurrent WebSocket UI clients.
// Each slot consumes ~100 bytes. Reject new connections above this limit.
#define MAX_WS_CLIENTS 3

// NTP: timeout per sync attempt before retrying (ms)
#define NTP_WAIT_MS 2000

// NTP: cooldown after 10 failed attempts before retrying again (ms)
#define NTP_RETRY_INTERVAL_MS 300000  // 5 minutes

// Weight event throttle: minimum interval between WebSocket weight pushes (ms).
// Raw sensor runs at 10 Hz, but UI only needs ~1 Hz updates.
#define EMIT_INTERVAL_MS 1000

// WebSocket heartbeat interval (ms).
// Clients use this to detect connection loss.
#define HEARTBEAT_INTERVAL_MS 10000

// Moving average window size for HX711 weight smoothing.
// Larger = smoother but slower response. 5 samples @ 10 Hz = 0.5s window.
#define MA_WINDOW 5

// Emit a ScaleTimeout event every N consecutive HX711 read failures.
// Avoids flooding the event queue on persistent sensor problems.
#define TIMEOUT_EVENT_THRESHOLD 5

#endif  // LIB_APPCONFIG_APPCONFIG_H_
