/**
 * @file WebServerX.h
 * @brief High level web / WebSocket interface for the Simple Automatic Honey
 * Filling Machine
 *
 * Provides HTTP routes, OTA update support and a small multi-client WebSocket
 * hub (up to 3 concurrent UI clients). It encapsulates parameter formatting,
 * request parsing and pushes state / realtime events (weight, status,
 * heartbeat) via a lightweight broadcast API.
 *
 * Design goals:
 *  - Low coupling: core filling/business logic lives in FreeRTOS tasks &
 * HMConfig; UI receives event-driven deltas.
 *  - Deterministic resource usage: fixed WS client cap (MAX 3) avoids heap
 * fragmentation.
 *  - Extensibility: broadcastText() is a single choke point for future
 * filtering / auth / rate limiting.
 *
 * @date 2023-08-12 (initial)
 * @date 2025-10-04 (Multi-Client Refactor & Doxygen documentation)
 * @author Johannes G. Arlt (janusz)
 * @copyright MIT
 */

#ifndef LIB_WEBSERVERX_WEBSERVERX_H_
#define LIB_WEBSERVERX_WEBSERVERX_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <Glass.h>
#include <HMConfig.h>
#include <SPIFFS.h>
#include <WebTemplateOptimized.h>
#include <appconfig.h>

#ifdef ESP32
#include <rom/rtc.h>
#endif

/**
 * @brief Helper structure to parse simple query / WebSocket key=value pairs.
 * Filled by split() to avoid dynamic allocation (fixed small array).
 */
struct KeyValueArray {
  /** Single key/value pair */
  struct KeyValue {
    String key;    ///< Key
    String value;  ///< Raw string value (converted/validated later)
  };
  KeyValue keyValue[3];  ///< Fixed capacity (currently needs <=3)
  int count;             ///< Number of populated entries
};

/**
 * @brief Initialize and start the HTTP / WebSocket server (port 80).
 * Registers routes (static + templated) and the WebSocket handler. Call after
 * filesystem mount and HMConfig initialization.
 */
void WebserverStart(void);

/** @brief Generate an HTML table with system information (heap, flash, chip
 * IDs). */
String getSystemInfoTable();
/** @brief Return build timestamp (compile date & time). */
String getBuildDateAndTime();

/**
 * @brief Split a KEY=VALUE&KEY2=VALUE2 string into a KeyValueArray.
 * @param rval Target structure (overwritten)
 * @param wsdata Input string
 * @return Filled structure (copy of rval)
 */
KeyValueArray split(KeyValueArray rval, String wsdata);

/** @brief Check if a string represents a simple integer/number. */
bool isNumber(String val);
// boolean validateNumber(String test);
// boolean isNumeric(String str);

/** Overloaded helpers to read and optionally persist GET parameters. */
String getWebParam(AsyncWebServerRequest* request, const char* key,
                   String* prefsvar);
String getWebParam(AsyncWebServerRequest* request, const char* key,
                   double* prefsvar);
String getWebParam(AsyncWebServerRequest* request, const char* key);

/**
 * @brief Central WebSocket event callback (connect/disconnect/data/pong/error).
 * Manages multi-client state (max 3), snapshot broadcast on connect and logs
 * IP/port. Data frames are forwarded for parsing.
 */
void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len);

/** @brief Placeholder / legacy socket push API (currently unused). */
void sendSocketData();

// Multi-client WebSocket support (added 2025-10-04)
/** @brief Number of currently connected WebSocket clients (status ==
 * WS_CONNECTED). */
uint8_t wsClientCount();
/** @brief Broadcast a text frame to all connected WebSocket clients. */
void broadcastText(const String& msg);

#ifdef ESP32
String getResetReason(RESET_REASON);
#endif
#if CORE_DEBUG_LEVEL > 4
int showRequest(AsyncWebServerRequest* request);
#endif

#endif  // LIB_WEBSERVERX_WEBSERVERX_H_
