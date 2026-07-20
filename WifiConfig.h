#pragma once
#include <Arduino.h>

enum class WifiMode { AP, STA };

struct WifiSettings {
  WifiMode mode = WifiMode::AP;
  String ssid;
  String password;
};

class WifiConfig {
public:
  // lädt gespeicherte Konfiguration und verbindet (STA oder AP mit Fallback)
  void begin();

  WifiSettings getSettings() const { return _settings; }
  bool isConnected() const;
  String getStatusText() const;

  bool save(const WifiSettings& settings);
  void load();

private:
  WifiSettings _settings;
  bool _apActive = false;

  void startAP();
  bool startSTA();
};
