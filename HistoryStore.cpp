#include "HistoryStore.h"

void HistoryStore::begin() {
  _head = 0;
  _count = 0;
  _lastSampleMs = millis();
}

void HistoryStore::addSample(float speedMs, uint32_t timestamp) {
  _buffer[_head] = {timestamp, speedMs};
  _head = (_head + 1) % HISTORY_LENGTH;
  if (_count < HISTORY_LENGTH) _count++;
}

void HistoryStore::update(float currentSpeedMs, uint32_t timestamp) {
  unsigned long now = millis();
  if (now - _lastSampleMs >= HISTORY_INTERVAL_MS) {
    _lastSampleMs = now;
    addSample(currentSpeedMs, timestamp);
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
