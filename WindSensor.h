#pragma once
#include <Arduino.h>

struct WindValues {
  float freqHz;
  float speedMs;
  float speedKmh;
  float speedKn;
  uint8_t beaufort;
};

class WindSensor {
public:
  void begin(uint8_t pin);
  void update();  // regelmäßig in loop() aufrufen
  WindValues getValues() const { return _values; }

private:
  static void IRAM_ATTR isrHandler();
  static volatile uint32_t _pulseCount;

  uint8_t _pin = 0;
  unsigned long _lastMeasureMs = 0;
  WindValues _values = {0, 0, 0, 0, 0};

  static float freqToSpeed(float freq);
  static uint8_t speedToBeaufort(float ms);
};
