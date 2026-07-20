#include "WindSensor.h"
#include "config.h"

volatile uint32_t WindSensor::_pulseCount = 0;

void IRAM_ATTR WindSensor::isrHandler() {
  _pulseCount++;
}

void WindSensor::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_pin), isrHandler, RISING);
  _lastMeasureMs = millis();
}

void WindSensor::update() {
  unsigned long now = millis();
  if (now - _lastMeasureMs >= MEASURE_INTERVAL_MS) {
    noInterrupts();
    uint32_t count = _pulseCount;
    _pulseCount = 0;
    interrupts();

    float intervalS = (now - _lastMeasureMs) / 1000.0f;
    _lastMeasureMs = now;

    float freq = (intervalS > 0) ? (count / intervalS) : 0.0f;
    freq = constrain(freq, FREQ_MIN_HZ, FREQ_MAX_HZ);

    _values.freqHz   = freq;
    _values.speedMs  = freqToSpeed(freq);
    _values.speedKmh = _values.speedMs * 3.6f;
    _values.speedKn  = _values.speedMs * 1.943844f;
    _values.beaufort = speedToBeaufort(_values.speedMs);
  }
}

float WindSensor::freqToSpeed(float freq) {
  // lineare Kennlinie: 0-700Hz entspricht 0-70 m/s
  float speed = (freq - FREQ_MIN_HZ) * (SPEED_MAX_MS - SPEED_MIN_MS) /
                (FREQ_MAX_HZ - FREQ_MIN_HZ) + SPEED_MIN_MS;
  return constrain(speed, SPEED_MIN_MS, SPEED_MAX_MS);
}

uint8_t WindSensor::speedToBeaufort(float ms) {
  // Obergrenzen der Beaufort-Stufen 0-11 in m/s (Stufe 12 = Rest)
  static const float thresholds[12] = {
    0.5f, 1.6f, 3.4f, 5.5f, 8.0f, 10.8f,
    13.9f, 17.2f, 20.8f, 24.5f, 28.5f, 32.7f
  };
  for (uint8_t i = 0; i < 12; i++) {
    if (ms < thresholds[i]) return i;
  }
  return 12;
}
