/*
 * File: /MQTTHelper.h
 * Project: Simple Automatic Honey Filling Machine
 * Description: MQTT client wrapper for sending filling data
 * -----
 * Created Date: 2025-11-27
 * Author: Johannes G. Arlt (janusz)
 * -----
 * Copyright (c) 2025 Johannes Arlt (devcpu) Berlin, Germany
 */

#ifndef LIB_MQTTHELPER_MQTTHELPER_H_
#define LIB_MQTTHELPER_MQTTHELPER_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HMConfig.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

class MQTTHelper {
 public:
  static MQTTHelper& instance() {
    static MQTTHelper _instance;
    return _instance;
  }

  /// @brief Initialize MQTT client with current config
  void begin();

  /// @brief Check if MQTT is configured and active
  bool isActive();

  /// @brief Reconnect to MQTT broker if disconnected
  bool reconnect();

  /// @brief Send filling data after glass completion
  /// @param weight_actual Actual weight of filled glass (after 5s settle)
  /// @param weight_target Target weight configured
  /// @param glassCount Total glass count
  /// @return true if message was sent successfully
  bool sendFillingData(uint16_t weight_actual, uint16_t weight_target,
                       uint32_t glassCount);

  /// @brief Process MQTT loop (should be called regularly)
  void loop();

 private:
  MQTTHelper();
  MQTTHelper(const MQTTHelper&);
  MQTTHelper& operator=(const MQTTHelper&);

  WiFiClient wifiClient;
  WiFiClientSecure wifiClientSecure;
  PubSubClient mqttClient;

  bool useTLS = false;
  bool initialized = false;
  uint32_t lastReconnectAttempt = 0;
  const uint32_t RECONNECT_INTERVAL_MS = 5000;  // Try reconnect every 5s
  const uint32_t SOCKET_TIMEOUT_MS = 2000;      // Socket timeout 2s
  const uint32_t PUBLISH_TIMEOUT_MS = 3000;     // Publish timeout 3s

  /// @brief Get current timestamp in ISO format
  String getISOTimestamp();
};

#endif  // LIB_MQTTHELPER_MQTTHELPER_H_
