/*
 * =========================================================
 *  Windmesser
 *  Adafruit Feather HUZZAH ESP8266
 * =========================================================
 *
 * Eingangssignal: digitales Frequenzsignal 0-700Hz an SENSOR_PIN
 * entspricht linear 0-70 m/s Windgeschwindigkeit.
 *
 * Ausgabe: m/s, km/h, Knoten, Beaufort über Web-Dashboard,
 * inkl. 24h-Verlaufsgrafik und Maximalwert mit Datum/Uhrzeit.
 *
 * WLAN: konfigurierbar als Access Point (Standard) oder als
 * Client (STA) über die Weboberfläche unter /config.
 *
 * Benötigte Bibliotheken (Arduino-Bibliotheksverwalter):
 *  - ESP8266 Boardpaket (Board: "Adafruit Feather HUZZAH ESP8266")
 *  - ArduinoJson (Version 6.x)
 *  - LittleFS (im ESP8266-Boardpaket enthalten)
 *
 * Verkabelung:
 *  - Sensor-Ausgang (Frequenzsignal, 3.3V-Pegel!) -> GPIO12 (D6)
 *  - GND des Sensors -> GND des Feather
 *  Falls der Sensor 5V-Logikpegel liefert, unbedingt einen
 *  Pegelwandler / Spannungsteiler verwenden, da die ESP8266-Pins
 *  nicht 5V-tolerant sind.
 * =========================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

#include "config.h"
#include "WindSensor.h"
#include "HistoryStore.h"
#include "WifiConfig.h"
#include "WebPages.h"

ESP8266WebServer server(WEBSERVER_PORT);
WindSensor windSensor;
HistoryStore historyStore;
WifiConfig wifiConfig;

bool timeSynced = false;

uint32_t currentUnixTime() {
  return (uint32_t)time(nullptr);
}

void setupTime() {
  configTime(TZ_INFO, NTP_SERVER1, NTP_SERVER2);
}

// ---------------------------------------------------------
//  Webserver-Handler
// ---------------------------------------------------------

void handleRoot() {
  server.send(200, "text/html", FPSTR(PAGE_INDEX));
}

void handleApiCurrent() {
  WindValues v = windSensor.getValues();
  StaticJsonDocument<256> doc;
  doc["hz"]  = v.freqHz;
  doc["ms"]  = v.speedMs;
  doc["kmh"] = v.speedKmh;
  doc["kn"]  = v.speedKn;
  doc["bft"] = v.beaufort;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiHistory() {
  // 1440 Punkte * ca. 20 Bytes JSON-Overhead -> ausreichend Puffer wählen
  DynamicJsonDocument doc(1024 * 20);
  JsonArray arr = doc.to<JsonArray>();
  for (size_t i = 0; i < historyStore.count(); i++) {
    const HistoryPoint& p = historyStore.at(i);
    JsonObject o = arr.createNestedObject();
    o["t"] = p.timestamp;
    o["v"] = p.speedMs;
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiMax() {
  HistoryPoint p = historyStore.getMax();
  StaticJsonDocument<128> doc;
  doc["ms"] = p.speedMs;
  doc["t"]  = p.timestamp;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleConfigGet() {
  WifiSettings s = wifiConfig.getSettings();
  String page = FPSTR(PAGE_CONFIG);
  page.replace("%STATUS%", wifiConfig.getStatusText());
  page.replace("%AP_SEL%", s.mode == WifiMode::AP ? "selected" : "");
  page.replace("%STA_SEL%", s.mode == WifiMode::STA ? "selected" : "");
  page.replace("%SSID%", s.ssid);
  page.replace("%PASSWORD%", s.password);
  server.send(200, "text/html", page);
}

void handleConfigSave() {
  WifiSettings s;
  String mode = server.arg("mode");
  s.mode = (mode == "STA") ? WifiMode::STA : WifiMode::AP;
  s.ssid = server.arg("ssid");
  s.password = server.arg("password");

  wifiConfig.save(s);

  server.send(200, "text/html",
    "<html><body style='font-family:sans-serif;background:#0f172a;color:#e2e8f0;padding:20px'>"
    "<h2>Gespeichert. Gerät startet neu...</h2></body></html>");
  delay(1500);
  ESP.restart();
}

void handleNotFound() {
  server.send(404, "text/plain", "Nicht gefunden");
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/api/current", handleApiCurrent);
  server.on("/api/history", handleApiHistory);
  server.on("/api/max", handleApiMax);
  server.on("/config", HTTP_GET, handleConfigGet);
  server.on("/config/save", HTTP_POST, handleConfigSave);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[HTTP] Webserver gestartet.");
}

// ---------------------------------------------------------
//  Setup / Loop
// ---------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[Windmesser] Start");

  if (!LittleFS.begin()) {
    Serial.println("[FS] LittleFS konnte nicht initialisiert werden!");
  }

  wifiConfig.begin();
  setupTime();

  windSensor.begin(SENSOR_PIN);
  historyStore.begin();

  setupServer();
}

void loop() {
  server.handleClient();
  windSensor.update();

  uint32_t now = currentUnixTime();
  // Plausibilitätscheck: Zeit wurde per NTP synchronisiert
  // (nur im STA-Modus mit Internetzugang möglich)
  if (now > 1700000000UL) {
    timeSynced = true;
    historyStore.update(windSensor.getValues().speedMs, now);
  }
}
