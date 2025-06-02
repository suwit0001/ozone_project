// File: relay_control.cpp
#include <Arduino.h>
#include "relay_control.h"
#include "utils.h"

#define RELAY_PIN 12
#define FAN_PIN 14

bool relayStatus = false;
bool fanStatus = false;

void setupRelayAndFan() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  setRelay(false);
  setFan(false);
  logEvent("INIT", "Relay and Fan pins initialized");
}

void setRelay(bool on) {
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  relayStatus = on;
  logEvent("RELAY", on ? "ON" : "OFF");
}

void setFan(bool on) {
  digitalWrite(FAN_PIN, on ? HIGH : LOW);
  fanStatus = on;
  logEvent("FAN", on ? "ON" : "OFF");
}

bool getRelayStatus() {
  return relayStatus;
}

bool getFanStatus() {
  return fanStatus;
}