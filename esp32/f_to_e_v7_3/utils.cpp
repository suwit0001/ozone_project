
// File: utils.cpp
#include "utils.h"

void logEvent(const String& type, const String& message) {
  unsigned long ms = millis();
  unsigned long s = (ms / 1000) % 60;
  unsigned long m = (ms / 60000) % 60;
  unsigned long h = (ms / 3600000);
  char timeBuffer[12];
  sprintf(timeBuffer, "%02lu:%02lu:%02lu", h, m, s);
  Serial.println("[LOG] " + String(timeBuffer) + " | " + type + " | " + message);
}
