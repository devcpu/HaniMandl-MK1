# FIX.md - Review Findings

Aus Code-Review vom 2026-03-06. Sortiert nach Prioritaet.

## Blocker

- [ ] **S2** Secrets in Git-History (`attic/config.json` mit MQTT-Passwort/Token). Aus Repo entfernen, `git filter-repo`, Credentials rotieren.
- [x] **S3** `split()` kein Bounds-Check auf `key_value_idx` - Stack Corruption (`WebServerX.cpp:789`). Fix: `if (key_value_idx >= 3) break;`
- [x] **DOC4** Copyright "STRATO AG" in `cooperative_loop.h` und `persistence.h` - privates Projekt. Auf "Johannes G. Arlt" korrigieren.
- [x] **UX1** WLAN-Formular Parameter-Namen passen nicht zum Backend (html: `ip_local` vs backend: `ip_address`). Namen angleichen.
- [x] **UX2** MQTT/API-Felder in `setupwlan.html` werden im Backend nicht gelesen. Handler erweitern.

## Hoch

- [x] **CQ1** `data[len] = 0` OOB-Write im WebSocket-Handler (`WebServerX.cpp:920`). Fix: `String::concat(data, len)`.
- [x] **CQ2** `glass_count` ist `uint16_t`, Overflow nach 65535 (`HMConfig.h:167`). Auf `uint32_t` geaendert.
- [x] **CQ3** `setTaraWeight(uint16_t)` wird mit `float` aufgerufen (`Glass.cpp:35`). Parameter auf `float` geaendert.
- [x] **CQ4** `delay(10000)` in `setup()` blockiert alles inkl. Notaus (`main.cpp:88`). Non-blocking `otaValidateOnce()` in `tickHousekeeping()`.
- [x] **UX3** `<dev>` statt `<div>` Tags in `setupwlan.html` (Zeilen 10,22,25,37). Zu `<div>` korrigieren.
- [x] **UX4** Versionsnummer-Inkonsistenz: appconfig `v 0.4` vs `master.html` `v 0.2`. Aus Template-Variable beziehen.
- [x] **DOC5** README.md: komplett neu geschrieben mit Ziel, Hardware, Ersteinrichtung, Architektur, Lizenz.

## Mittel

- [x] **CQ5** `MQTTServerData` Aggregate-Init hat 6 Werte fuer 7 Felder. Fix: `= {}` (value-initialization).
- [x] **CQ6** `ESPFS::readString()` zeichenweise mit `+=`. Fix: `file.readString()`.
- [x] **CQ7** `template_response_buffer[512]` global. Kommentar zur Einschraenkung (single-threaded event loop).
- [x] **CQ11** WebSocket-Config-Werte umgehen `validateAndFix()`. Fix: Aufruf nach WS-Config-Aenderungen.
- [ ] **S7** HTTP GET fuer zustandsaendernde Ops - CSRF-anfaellig. Separate Iteration (alle Formulare + HTML).
- [ ] **S8** Kein CORS/DNS-Rebinding-Schutz, kein Origin-Check. Separate Iteration.
- [x] **UX5** Keine Erfolgsrueckmeldung nach Formular-Submit. Fix: Redirect mit `?saved=1` + gruener Banner.
- [ ] **UX6** Kalibrierungs-Fehler nur im Serial-Log, nicht in WebUI. `%ERRORMSG%`-Div nutzen.
- [x] **UX7** First-Run-Wizard greift nie. Fix: `beekeeping` Default auf `""`.
- [x] **UX8** `log_e()` fuer normale Betriebsmeldungen missbraucht. Fix: Korrekte Levels in WebServerX + handleServo.
- [ ] **UX11** Kein Schutz bei gleichzeitigem Zugriff mehrerer Clients. Separate Iteration.
- [x] **DOC6** Veraltete Referenz auf `freertos_setup.cpp` in `WebServerX.cpp`. Bereinigt.

## Niedrig

- [x] **CQ8** `readJsonConfig()` liest Template doppelt. Fix: `bool seededFromTemplate` Flag.
- [x] **CQ9** `first_run` global. Fix: `static bool followUpActive` im FOLLOW_UP-Block.
- [x] **CQ12** `#pragma once` + `#ifndef` redundant. Fix: `#pragma once` entfernt.
- [ ] **UX9** Sprach-Mix Deutsch/Englisch in UI. Separate Iteration.
- [ ] **UX10** `weight_fine` wird bei `weight_filling`-Aenderung still ueberschrieben. Separate Iteration.
- [x] **DOC7** `cooperative_loop.cpp` und `persistence.cpp` Kopfkommentare ergaenzt.
- [x] **DOC8** Tick-Funktionen in `cooperative_loop.h` mit `@brief` dokumentiert.
- [x] **DOC9** Author "GitHub Copilot" in WebTemplateOptimized.h/.cpp korrigiert.
