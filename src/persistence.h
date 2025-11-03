/*
 * Copyright (c) 2025 STRATO AG Berlin, Germany
 *  All rights reserved
 * -----
 * File: /persistence.h
Project: Simple Automatic Honey Filling Machine
 * Description:
 * -----
 * Created Date: 2025-10-03 18:08
 * Author: Johannes G.  Arlt (janusz)
 * -----
 * Last Modified: 2025-11-03 17:39
 * Modified By: Johannes G.  Arlt (janusz)
 */

#ifndef SRC_PERSISTENCE_H_
#define SRC_PERSISTENCE_H_

#include <Arduino.h>
#include <Preferences.h>

// Simple persistence for glass count with wear-level protection by batching
// writes. Strategy:
//  - Keep in RAM: current value (already in HMConfig::glass_count)
//  - Dirty flag + lastPersistMillis
//  - Mark dirty on increment
//  - Housekeeping task calls persistenceTick() periodically
//  - Conditions to write: (glass_count - lastPersistedCount) >=
//  GLASS_WRITE_BATCH
//                          OR millis() - lastPersistMillis >=
//                          GLASS_WRITE_MAX_INTERVAL
//  - Use Preferences namespace "hm" key "gcnt"
//  - Writes are idempotent; failure is logged

namespace Persistence {

bool init();
void markGlassCountDirty();
void persistenceTick();
uint32_t getLastPersistedGlassCount();

}  // namespace Persistence

#endif  // SRC_PERSISTENCE_H_
