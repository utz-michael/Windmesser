#pragma once

// =========================================================
//  Windmesser - Konfiguration
//  Adafruit Feather HUZZAH ESP8266
// =========================================================

// ---- Pin Konfiguration ----
// GPIO12 = D6 auf dem Feather HUZZAH ESP8266
// Hier liegt das digitale Frequenzsignal des Windsensors an.
#define SENSOR_PIN 12

// ---- Sensor-Kennlinie (linear) ----
#define FREQ_MIN_HZ      0.0f
#define FREQ_MAX_HZ      700.0f
#define SPEED_MIN_MS      0.0f
#define SPEED_MAX_MS      70.0f

// ---- Messintervalle ----
#define MEASURE_INTERVAL_MS   1000UL     // Frequenz wird alle 1s neu berechnet
#define HISTORY_INTERVAL_MS   60000UL    // ein Messpunkt für die Historie alle 60s
#define HISTORY_LENGTH         1440      // 1440 * 60s = 24h

// ---- NTP / Zeitzone ----
#define NTP_SERVER1  "pool.ntp.org"
#define NTP_SERVER2  "de.pool.ntp.org"
// Mitteleuropäische Zeit inkl. automatischer Sommerzeitumstellung
#define TZ_INFO      "CET-1CEST,M3.5.0,M10.5.0/3"

// ---- WLAN ----
#define AP_SSID_PREFIX          "Windmesser-Setup"
#define AP_PASSWORD             "windmesser"   // mind. 8 Zeichen
#define STA_CONNECT_TIMEOUT_MS  15000UL
#define CONFIG_FILE             "/wifi_config.json"

#define WEBSERVER_PORT 80
