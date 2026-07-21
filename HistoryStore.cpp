#include "HistoryStore.h"

void HistoryStore::begin() {
  _head = 0;
  _count = 0;
  _lastSampleMs = millis();
  _peakSinceLastSample = 0.0f;
}

void HistoryStore::addSample(float speedMs, uint32_t timestamp) {
  _buffer[_head] = {timestamp, speedMs};
  _head = (_head + 1) % HISTORY_LENGTH;
  if (_count < HISTORY_LENGTH) _count++;
}

void HistoryStore::trackPeak(float speedMs) {
  if (speedMs > _peakSinceLastSample) {
    _peakSinceLastSample = speedMs;
  }
}

void HistoryStore::update(uint32_t timestamp) {
  unsigned long now = millis();
  if (now - _lastSampleMs >= HISTORY_INTERVAL_MS) {
    _lastSampleMs = now;
    // Speichert den höchsten Wert, der seit der letzten Abtastung
    // aufgetreten ist - nicht nur den aktuellen Momentanwert. So gehen
    // kurze Böen zwischen zwei 60s-Abtastungen nicht verloren.
    addSample(_peakSinceLastSample, timestamp);
    _peakSinceLastSample = 0.0f;
  }
}

HistoryPoint HistoryStore::getMax() const {
  HistoryPoint result{0, 0};
  for (size_t i = 0; i < _count; i++) {
    const HistoryPoint& p = at(i);
    if (p.speedMs > result.speedMs) result = p;
  }
  return result;
}
