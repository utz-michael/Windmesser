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

// ---- Messintervall ----
#define MEASURE_INTERVAL_MS   1000UL     // Frequenz wird alle 1s neu berechnet

// ---- WLAN ----
#define AP_SSID_PREFIX          "Windmesser-Setup"
#define AP_PASSWORD             "windmesser"   // mind. 8 Zeichen
#define STA_CONNECT_TIMEOUT_MS  15000UL
#define CONFIG_FILE             "/wifi_config.json"

#define WEBSERVER_PORT 80
