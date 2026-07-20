#include "WifiConfig.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

void WifiConfig::load() {
  _settings = WifiSettings();
  if (!LittleFS.exists(CONFIG_FILE)) return;

  File f = LittleFS.open(CONFIG_FILE, "r");
  if (!f) return;

  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return;

  String mode = doc["mode"] | "AP";
  _settings.mode = (mode == "STA") ? WifiMode::STA : WifiMode::AP;
  _settings.ssid = String((const char*)(doc["ssid"] | ""));
  _settings.password = String((const char*)(doc["password"] | ""));
}

bool WifiConfig::save(const WifiSettings& settings) {
  StaticJsonDocument<512> doc;
  doc["mode"] = (settings.mode == WifiMode::STA) ? "STA" : "AP";
  doc["ssid"] = settings.ssid;
  doc["password"] = settings.password;

  File f = LittleFS.open(CONFIG_FILE, "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  _settings = settings;
  return true;
}

void WifiConfig::startAP() {
  String apName = String(AP_SSID_PREFIX) + "-" + String(ESP.getChipId(), HEX);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str(), AP_PASSWORD);
  _apActive = true;
  Serial.print("[WiFi] Access Point gestartet: ");
  Serial.println(apName);
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.softAPIP());
}

bool WifiConfig::startSTA() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(_settings.ssid.c_str(), _settings.password.c_str());
  Serial.print("[WiFi] Verbinde mit ");
  Serial.print(_settings.ssid);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < STA_CONNECT_TIMEOUT_MS) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    _apActive = false;
    Serial.print("[WiFi] Verbunden. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}

void WifiConfig::begin() {
  load();

  if (_settings.mode == WifiMode::STA && _settings.ssid.length() > 0) {
    if (!startSTA()) {
      Serial.println("[WiFi] STA-Verbindung fehlgeschlagen, starte Access Point als Fallback.");
      startAP();
    }
  } else {
    startAP();
  }
}

bool WifiConfig::isConnected() const {
  return !_apActive && WiFi.status() == WL_CONNECTED;
}

String WifiConfig::getStatusText() const {
  if (_apActive) {
    return "Access Point (" + WiFi.softAPSSID() + ") - " + WiFi.softAPIP().toString();
  }
  return "Client verbunden mit " + _settings.ssid + " - " + WiFi.localIP().toString();
}
