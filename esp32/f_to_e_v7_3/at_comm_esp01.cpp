// File: at_comm_esp01.cpp
#include "at_comm_esp01.h"
#include "eeprom_storage.h"
#include "utils.h"

bool sendAT(String cmd, String expected, int timeout = 3000) {
  Serial2.println(cmd);
  logEvent("AT", "Sent: " + cmd);
  long start = millis();
  String response = "";
  while (millis() - start < timeout) {
    while (Serial2.available()) {
      char c = Serial2.read();
      response += c;
    }
    if (response.indexOf(expected) != -1) {
      logEvent("AT", "Received: " + expected);
      return true;
    }
  }
  logEvent("AT", "TIMEOUT waiting for: " + expected);
  return false;
}

void connectESP01ToWiFi(const char* ssid, const char* password) {
  if (!sendAT("AT", "OK")) return;
  if (!sendAT("AT+CWMODE=1", "OK")) return;
  String cmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  if (!sendAT(cmd, "WIFI CONNECTED", 10000)) {
    logEvent("WIFI", "Failed to connect to SSID: " + String(ssid));
    return;
  }
  if (sendAT("AT+CIFSR", "STAIP")) {
    logEvent("WIFI", "Connected to " + String(ssid) + " (IP obtained)");
  } else {
    logEvent("WIFI", "Connected to " + String(ssid) + " but failed to get IP");
  }
}
