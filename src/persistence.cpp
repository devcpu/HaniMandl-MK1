/*
 * Copyright (c) 2025 - 2026 Johannes G. Arlt - Berlin - Germany
 * License MIT License
 * -----
 * File: /persistence.cpp
 * Project: Simple Automatic Honey Filling Machine
 * Description: Glass count persistence via ESP32 Preferences (wear-leveled NVS)
 * -----
 * Created Date: 2025-10-03
 * Author: Johannes G.  Arlt (janusz)
 */

#include "persistence.h"

#include <HMConfig.h>

namespace {
Preferences prefs;
bool initialized = false;
bool glassDirty = false;
uint32_t lastPersistMillis = 0;
uint32_t lastPersistedCount = 0;

constexpr uint32_t GLASS_WRITE_BATCH = 5;  // write every 5 new glasses
constexpr uint32_t GLASS_WRITE_MAX_INTERVAL =
    5UL * 60UL * 1000UL;  // max 5 minutes
}  // namespace

namespace Persistence {

bool init() {
  if (initialized) return true;
  if (!prefs.begin("hm", false)) {
    log_e("Preferences begin failed");
    return false;
  }
  lastPersistedCount = prefs.getUInt("gcnt", 0);
  HMConfig::instance().glass_count = lastPersistedCount;  // load into config
  lastPersistMillis = millis();
  initialized = true;
  glassDirty = false;
  return true;
}

void markGlassCountDirty() { glassDirty = true; }

uint32_t getLastPersistedGlassCount() { return lastPersistedCount; }

static void persistNow(uint32_t current) {
  if (!initialized) return;
  prefs.putUInt("gcnt", current);
  lastPersistedCount = current;
  lastPersistMillis = millis();
  glassDirty = false;
  log_i("[Persist] glass_count saved=%u", current);
}

void persistenceTick() {
  if (!initialized) return;
  uint32_t now = millis();
  uint32_t current = HMConfig::instance().glass_count;
  if (!glassDirty) return;  // nothing to do

  bool batchReached = (current - lastPersistedCount) >= GLASS_WRITE_BATCH;
  bool intervalExceeded = (now - lastPersistMillis) >= GLASS_WRITE_MAX_INTERVAL;
  if (batchReached || intervalExceeded) {
    persistNow(current);
  }
}

}  // namespace Persistence
