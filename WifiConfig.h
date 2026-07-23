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

  // regelmäßig in loop() aufrufen (z.B. alle 10s):
  // prüft im STA-Modus die Verbindung und stößt bei Bedarf einen
  // Reconnect an; nach zu vielen Fehlversuchen wird neu gestartet
  void checkConnection();

private:
  WifiSettings _settings;
  bool _apActive = false;
  uint8_t _reconnectFailCount = 0;

  void startAP();
  bool startSTA();
};
