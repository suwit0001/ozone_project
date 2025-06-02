// File: ozone_sensor.cpp
#include "ozone_sensor.h"
#define OZONE_SENSOR_PIN 36

float currentOzonePPM = 0.0;

float convertToPPM(int analogValue) {
  float voltage = (analogValue / 4095.0) * 3.3;
  float ppm = (voltage - 0.4) * (10.0 / (2.0 - 0.4));
  if (ppm < 0) ppm = 0;
  return ppm;
}

void updateOzoneSensor() {
  int analogValue = analogRead(OZONE_SENSOR_PIN);
  currentOzonePPM = convertToPPM(analogValue);
}

float getCurrentOzone() {
  return currentOzonePPM;
}
