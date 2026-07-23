#include "NmeaSender.h"
#include "config.h"

void NmeaSender::begin() {
  // WiFiUDP benötigt für reines Senden kein explizites begin() mit
  // festem lokalen Port - beginPacket()/endPacket() reichen aus.
}

uint8_t NmeaSender::computeChecksum(const String& sentenceBody) {
  // NMEA-0183-Prüfsumme: XOR aller Zeichen zwischen "$" und "*"
  uint8_t checksum = 0;
  for (size_t i = 0; i < sentenceBody.length(); i++) {
    checksum ^= (uint8_t)sentenceBody[i];
  }
  return checksum;
}

void NmeaSender::sendWindSpeed(float speedMs) {
  // $--MWV,Windwinkel,Referenz,Windgeschwindigkeit,Einheit,Status*Checksumme
  //
  // Talker "WI" = Weather Instruments (üblich für reine Wettersensoren)
  // Windwinkel: wird von diesem Sensor NICHT gemessen (kein Windfahnen-
  //   Sensor vorhanden) und daher als Platzhalter 0.0 gesendet.
  //   Empfangende Software sollte dieses Feld ignorieren bzw. es ist
  //   dort entsprechend zu kennzeichnen, dass keine Richtung vorliegt.
  // Referenz "R" = relativ/scheinbar (Standard für einfache Anemometer)
  // Einheit "M" = Meter pro Sekunde
  // Status "A" = Daten gültig (die Geschwindigkeitsmessung ist gültig,
  //   auch wenn der Winkel nur ein Platzhalter ist)

  char speedBuf[8];
  dtostrf(speedMs, 0, 1, speedBuf);

  String body = "WIMWV,0.0,R,";
  body += speedBuf;
  body += ",M,A";

  uint8_t cs = computeChecksum(body);
  char csBuf[3];
  snprintf(csBuf, sizeof(csBuf), "%02X", cs);

  String sentence = "$" + body + "*" + String(csBuf) + "\r\n";

  IPAddress targetIp;
  if (!targetIp.fromString(NMEA_UDP_TARGET_IP)) {
    Serial.println("[NMEA] Ungültige Ziel-IP in config.h");
    return;
  }

  _udp.beginPacket(targetIp, NMEA_UDP_TARGET_PORT);
  _udp.write((const uint8_t*)sentence.c_str(), sentence.length());
  _udp.endPacket();
}
