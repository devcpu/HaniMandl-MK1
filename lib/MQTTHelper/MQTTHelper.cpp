/*
 * File: /MQTTHelper.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description: MQTT client wrapper for sending filling data
 * -----
 * Created Date: 2025-11-27
 * Author: Johannes G. Arlt (janusz)
 * -----
 * Copyright (c) 2025 Johannes Arlt (devcpu) Berlin, Germany
 */

#include "MQTTHelper.h"

#include <sys/time.h>
#include <time.h>

MQTTHelper::MQTTHelper()
    : wifiClient(), wifiClientSecure(), mqttClient(wifiClient) {}

void MQTTHelper::begin() {
  HMConfig& cfg = HMConfig::instance();

  // Check if MQTT is configured
  if (strlen(cfg.mqtt_server.server_ip) == 0) {
    log_w("MQTT server IP not configured");
    initialized = false;
    return;
  }

  useTLS = cfg.mqtt_server.server_tls;

  // Configure appropriate client based on TLS setting
  if (useTLS) {
    log_i("MQTT: Using TLS (insecure mode - no certificate validation)");
    wifiClientSecure.setInsecure();  // No certificate validation as requested
    wifiClientSecure.setTimeout(SOCKET_TIMEOUT_MS /
                                1000);  // Set socket timeout
    mqttClient.setClient(wifiClientSecure);
  } else {
    log_i("MQTT: Using plain TCP");
    wifiClient.setTimeout(SOCKET_TIMEOUT_MS / 1000);  // Set socket timeout
    mqttClient.setClient(wifiClient);
  }

  // Parse port (default 1883 for plain, 8883 for TLS)
  uint16_t port = 1883;
  if (strlen(cfg.mqtt_server.server_port) > 0) {
    port = atoi(cfg.mqtt_server.server_port);  // flawfinder: ignore
  } else {
    port = useTLS ? 8883 : 1883;
  }

  log_i("MQTT: Configuring server %s:%d", cfg.mqtt_server.server_ip, port);
  mqttClient.setServer(cfg.mqtt_server.server_ip, port);

  // Set buffer size for JSON payload (default 256 is too small)
  mqttClient.setBufferSize(512);

  initialized = true;

  // Try initial connection
  reconnect();
}

bool MQTTHelper::isActive() {
  HMConfig& cfg = HMConfig::instance();
  return initialized && strlen(cfg.mqtt_server.server_ip) > 0;
}

bool MQTTHelper::reconnect() {
  if (!initialized) {
    return false;
  }

  // Don't try too often
  uint32_t now = millis();
  if (now - lastReconnectAttempt < RECONNECT_INTERVAL_MS) {
    return false;
  }
  lastReconnectAttempt = now;

  if (mqttClient.connected()) {
    return true;
  }

  HMConfig& cfg = HMConfig::instance();

  log_i("MQTT: Attempting connection to %s...", cfg.mqtt_server.server_ip);

  // Generate unique client ID
  String clientId = "HaniMandl-";
  clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

  bool connected = false;
  if (strlen(cfg.mqtt_server.server_user) > 0) {
    // Connect with credentials
    connected =
        mqttClient.connect(clientId.c_str(), cfg.mqtt_server.server_user,
                           cfg.mqtt_server.server_passwd);
  } else {
    // Connect without credentials
    connected = mqttClient.connect(clientId.c_str());
  }

  if (connected) {
    log_i("MQTT: Connected successfully as %s", clientId.c_str());
  } else {
    log_e("MQTT: Connection failed, rc=%d", mqttClient.state());
  }

  return connected;
}

String MQTTHelper::getISOTimestamp() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    // Fallback if NTP not synced
    log_w("Failed to get time, using millis");
    char buf[32];  // flawfinder: ignore
    snprintf(buf, sizeof(buf), "1970-01-01 00:00:%lu", millis() / 1000);
    return String(buf);
  }

  // Format: "YYYY-MM-DD HH:MM:SS" - readable for humans and MySQL
  char buf[32];  // flawfinder: ignore
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

bool MQTTHelper::sendFillingData(uint16_t weight_actual, uint16_t weight_target,
                                 uint32_t glassCount) {
  if (!isActive()) {
    log_w("MQTT not active, skipping send");
    return false;
  }

  // Non-blocking: Don't try to send if not connected
  // Reconnect happens in background via loop()
  if (!mqttClient.connected()) {
    log_w("MQTT: Not connected, message will be lost (consider local backup)");
    return false;
  }

  HMConfig& cfg = HMConfig::instance();

  // Build JSON payload
  StaticJsonDocument<384> doc;

  doc["fillingdate"] = getISOTimestamp();

  // bucket is nullable - send null if -1
  if (cfg.bucket_number >= 0) {
    doc["bucket"] = cfg.bucket_number;
  } else {
    doc["bucket"] = (char*)nullptr;  // JSON null
  }

  doc["weight_actual"] = weight_actual;  // Tatsächliches Gewicht nach 5s
  doc["weight_target"] = weight_target;  // Konfiguriertes Soll-Gewicht
  doc["count"] = 1;                      // Always 1 per message as requested

  // Calculate batch_number from date_filling + 2 years
  char batch_buf[16];  // flawfinder: ignore
  cfg.getBatchNumber(batch_buf, sizeof(batch_buf));
  doc["batch_number"] = batch_buf;  // Chargennummer (Batch/Los)

  doc["harvestdate"] = cfg.harvest_date;
  doc["harvestnumber"] = cfg.harvest_number;

  // Serialize to string
  String payload;
  serializeJson(doc, payload);

  // Get configured topic (default to "bienen/sahfm" if empty)
  const char* topic = cfg.mqtt_server.server_topic;
  if (strlen(topic) == 0) {
    topic = "bienen/sahfm";
  }

  log_i("MQTT: Publishing to %s: %s", topic, payload.c_str());

  // Publish with QoS=1 and retain=true as requested
  uint32_t tStart = millis();
  bool success = mqttClient.publish(topic, payload.c_str(), true);
  uint32_t duration = millis() - tStart;

  if (success) {
    log_i("MQTT: Message sent successfully in %lums (QoS=1, retain=true)",
          duration);
    if (duration > 1000) {
      log_w("MQTT: Publish took %lums (slow network?)", duration);
    }
  } else {
    log_e("MQTT: Failed to send message after %lums, state=%d", duration,
          mqttClient.state());
  }

  return success;
}

void MQTTHelper::loop() {
  if (!isActive()) {
    return;
  }

  if (!mqttClient.connected()) {
    reconnect();
  } else {
    mqttClient.loop();
  }
}
