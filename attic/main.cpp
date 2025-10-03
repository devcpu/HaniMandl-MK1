/*
 * Copyright (c) 2025 STRATO AG Berlin, Germany
 *  All rights reserved
 * -----
 * File: /main.cpp
 * Project: /home/janusz/git/privat/esp32/HaniMandl-MK1/attic
 * Description:
 * -----
 * Created Date: 2025-10-03 11:31
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-10-03 11:37
 * Modified By: Johannes G.  Arlt (janusz)
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <WiFi.h>

// Höchste Priorität - WiFi AP Management
TaskHandle_t wifiTaskHandle;        // Prio: 10
TaskHandle_t webServerTaskHandle;   // Prio: 8

// Hardware Tasks
TaskHandle_t weightTaskHandle;      // Prio: 6
TaskHandle_t servoTaskHandle;       // Prio: 7


QueueHandle_t weightDataQueue;
QueueHandle_t webCommandQueue;

// Weight Task → alle anderen
// Web Commands → Servo Task

void setup() {
    // ... existing setup ...

    // Tasks auf verschiedene Cores verteilen
    xTaskCreatePinnedToCore(wifiTask, "WiFi", 8192, NULL, 10, &wifiTaskHandle, 0);
    xTaskCreatePinnedToCore(webServerTask, "Web", 8192, NULL, 8, &webServerTaskHandle, 0);
    xTaskCreatePinnedToCore(weightTask, "Weight", 4096, NULL, 6, &weightTaskHandle, 1);
    xTaskCreatePinnedToCore(servoTask, "Servo", 4096, NULL, 7, &servoTaskHandle, 1);
}

void wifiTask(void *parameter) {
    while(1) {
        // WiFi AP Management
        // You can add your AP management logic here, e.g., check connection status or handle clients.
        vTaskDelay(pdMS_TO_TICKS(10)); // Schnell reagieren
    }
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000)); // Minimal load
}
