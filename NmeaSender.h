#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>

class NmeaSender {
public:
  void begin();

  // sendet einen $WIMWV NMEA-0183-Satz mit der Windgeschwindigkeit (m/s)
  // per UDP an NMEA_UDP_TARGET_IP:NMEA_UDP_TARGET_PORT
  void sendWindSpeed(float speedMs);

private:
  WiFiUDP _udp;
  static uint8_t computeChecksum(const String& sentenceBody);
};
