#pragma once
#include <Arduino.h>
#include "config.h"

struct HistoryPoint {
  uint32_t timestamp;  // Unix-Zeit (Sekunden)
  float speedMs;
};

class HistoryStore {
public:
  void begin();

  // fügt sofort einen Messpunkt hinzu
  void addSample(float speedMs, uint32_t timestamp);

  // fortlaufend (z.B. jede Sekunde) aufrufen: merkt sich den höchsten
  // Wert seit der letzten Historien-Abtastung, damit kurze Böen
  // zwischen zwei Abtastungen nicht verloren gehen
  void trackPeak(float speedMs);

  // prüft intern das Intervall und schreibt bei Bedarf den seit der
  // letzten Abtastung höchsten Wert (aus trackPeak) in die Historie
  void update(uint32_t timestamp);

  size_t count() const { return _count; }
  // liefert den i-ten (chronologisch ältesten zuerst) Punkt
  const HistoryPoint& at(size_t i) const {
    return _buffer[(_head + HISTORY_LENGTH - _count + i) % HISTORY_LENGTH];
  }

  // Maximum über die aktuell gespeicherte Historie (bis zu 24h)
  HistoryPoint getMax() const;

private:
  HistoryPoint _buffer[HISTORY_LENGTH];
  size_t _head = 0;
  size_t _count = 0;
  unsigned long _lastSampleMs = 0;
  float _peakSinceLastSample = 0.0f;
};
