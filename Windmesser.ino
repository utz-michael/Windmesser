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

#include "config.h"
#include "WindSensor.h"
#include "WifiConfig.h"
#include "NmeaSender.h"
#include "WebPages.h"

ESP8266WebServer server(WEBSERVER_PORT);
WindSensor windSensor;
WifiConfig wifiConfig;
NmeaSender nmeaSender;

// ---------------------------------------------------------
//  Webserver-Handler
// ---------------------------------------------------------

void handleRoot() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", FPSTR(PAGE_INDEX));
  server.client().stop();
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
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", out);
  server.client().stop();
}

void handleConfigGet() {
  WifiSettings s = wifiConfig.getSettings();
  String page = FPSTR(PAGE_CONFIG);
  page.replace("%STATUS%", wifiConfig.getStatusText());
  page.replace("%AP_SEL%", s.mode == WifiMode::AP ? "selected" : "");
  page.replace("%STA_SEL%", s.mode == WifiMode::STA ? "selected" : "");
  page.replace("%SSID%", s.ssid);
  page.replace("%PASSWORD%", s.password);
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", page);
  server.client().stop();
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
  server.sendHeader("Connection", "close");
  server.send(404, "text/plain", "Nicht gefunden");
  server.client().stop();
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/api/current", handleApiCurrent);
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

  windSensor.begin(SENSOR_PIN);
  nmeaSender.begin();

  setupServer();
}

unsigned long lastHeapLogMs = 0;
unsigned long lastWifiCheckMs = 0;
unsigned long lastNmeaSendMs = 0;

void loop() {
  server.handleClient();
  windSensor.update();

  // NMEA-0183-Satz per UDP senden
  unsigned long nmeaMs = millis();
  if (nmeaMs - lastNmeaSendMs >= NMEA_SEND_INTERVAL_MS) {
    lastNmeaSendMs = nmeaMs;
    nmeaSender.sendWindSpeed(windSensor.getValues().speedMs);
  }

  // alle 10s prüfen, ob die STA-Verbindung noch steht, und bei Bedarf
  // reconnecten (siehe WifiConfig::checkConnection)
  unsigned long wifiMs = millis();
  if (wifiMs - lastWifiCheckMs >= 10000UL) {
    lastWifiCheckMs = wifiMs;
    wifiConfig.checkConnection();
  }

  // Heap-Diagnose: alle 60s freien Heap + Fragmentierung loggen.
  // Ein sinkender Trend oder eine stark wachsende Fragmentierung
  // deutet auf ein Speicherleck bzw. Fragmentierungsproblem hin.
  unsigned long ms = millis();
  if (ms - lastHeapLogMs >= 60000UL) {
    lastHeapLogMs = ms;
    uint32_t freeHeap = ESP.getFreeHeap();
    uint8_t fragPct = ESP.getHeapFragmentation();
    Serial.printf("[Heap] frei: %u Bytes, Fragmentierung: %u%%\n", freeHeap, fragPct);
  }
}
