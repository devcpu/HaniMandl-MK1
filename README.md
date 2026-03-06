# Simple Automatic Honey Filling Machine (HaniMandl MK1)

Automatische Honig-Abfuellmaschine auf ESP32-Basis mit Servo-Ventilsteuerung,
Waagenintegration (HX711), Web-UI und optionaler MQTT-Anbindung.

## Features

- Automatisches Abfuellen mit konfigurierbarem Zielgewicht und Feinfuellung
- Handmodus (Offen/Fein/Geschlossen) per Web-UI
- Echtzeit-Gewichtsanzeige via WebSocket
- Emergency-Stop per Touch-Sensor (GPIO27)
- WiFi STA + AP-Fallback (ohne Credentials startet der ESP im AP-Modus)
- NTP-Zeitsynchronisation mit konfigurierbarer Zeitzone (WebUI)
- OTA Firmware- und Filesystem-Update mit Rollback-Schutz
- MQTT-Anbindung fuer Fuellungsdokumentation (optional)
- Batch-Tracking (Chargennummer aus Fuelldatum + 2 Jahre)
- Glasszaehler mit Flash-Persistence (wear-leveled)

## Hardware

| Komponente | Pin | Beschreibung |
|---|---|---|
| HX711 DOUT | GPIO5 | Waagenmodul Datenleitung |
| HX711 SCK | GPIO17 | Waagenmodul Taktleitung |
| Servo | GPIO33 | Honig-Ventilsteuerung |
| Buzzer | GPIO16 | Akustische Signale |
| Touch Stop | GPIO27 (T7) | Emergency-Stop Touchpad |
| WiFi LED | GPIO21 | Status-LED (optional) |

## Ersteinrichtung

1. Firmware flashen: `pio run -t upload`
2. Filesystem flashen: `pio run -t uploadfs`
3. ESP startet im AP-Modus (SSID: "Honey Filling Machine")
4. Mit dem AP verbinden und Web-UI oeffnen (IP: 192.168.4.1)
5. Unter "Konfiguration WLan" das Heim-WLAN konfigurieren
6. Waage kalibrieren unter "Waage kalibrieren"

### WiFi-Credentials (optional)

Fuer automatische STA-Verbindung ohne WebUI-Setup:

```bash
cp credentials.h.template credentials.h
# Werte eintragen
pio run -t upload
```

## Build & Flash

```bash
pio run                    # Firmware kompilieren
pio run -t upload          # Firmware flashen
pio run -t buildfs         # Filesystem-Image bauen
pio run -t uploadfs        # Filesystem flashen (ueberschreibt SPIFFS!)
```

**Achtung:** `uploadfs` ueberschreibt die Laufzeit-Konfiguration (`hmconfig.json`)
auf dem Geraet. Nur ausfuehren wenn Web-Assets geaendert wurden. Die Kalibrierung
muss danach wiederholt werden.

## Konfiguration

Die Laufzeit-Konfiguration wird in `/hmconfig.json` auf SPIFFS gespeichert.
Beim ersten Start wird aus `hmconfig_template.json` geseedet. Alle Einstellungen
sind ueber die Web-UI aenderbar:

- Fuellgewicht, Feinfuellgewicht, Glasgewicht
- Servo-Winkel (Min/Max/Fein)
- Zeitzone und NTP-Server
- MQTT-Server (IP, User, Passwort, Topic, TLS)
- Statische IP-Konfiguration

## Architektur

Kooperativer Main-Loop mit `millis()`-basierten Tick-Funktionen:

| Funktion | Intervall | Aufgabe |
|---|---|---|
| `tickSensor()` | 100ms | HX711 lesen, Moving Average, Events |
| `tickServo()` | jeden Loop | Fuell-FSM, Servo-Steuerung |
| `tickWiFi()` | 500ms | WiFi-Reconnect, NTP, MQTT |
| `tickBuzzer()` | 20ms | Non-blocking Buzzer-Patterns |
| `tickWsDispatch()` | jeden Loop | Event-Queue -> WebSocket broadcast |
| `tickHousekeeping()` | 1000ms | Persistence, OTA-Validierung, Heap-Stats |

## Lizenz

MIT License - Copyright (c) 2023 - 2026 Johannes G. Arlt, Berlin

## Doxygen

```bash
sudo apt install doxygen graphviz
doxygen Doxyfile
open docs/doxygen/html/index.html
```
