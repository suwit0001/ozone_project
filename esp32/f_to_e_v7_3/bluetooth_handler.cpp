// File: bluetooth_handler.cpp
#include "BluetoothSerial.h"
#include "eeprom_storage.h"
#include "at_comm_esp01.h"
#include "relay_control.h"
#include "utils.h"

BluetoothSerial SerialBT;
String btBuffer;

void setupBluetooth() {
  if (!SerialBT.begin("ESP32-BT")) {
    logEvent("BT", "Failed to start Bluetooth");
  } else {
    logEvent("BT", "Bluetooth started as 'ESP32-BT'");
  }
}

void handleBluetoothCommands() {
  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n') {
      btBuffer.trim();
      if (btBuffer.length() > 0) {
        logEvent("BT", "Received command: " + btBuffer);
        parseBluetoothCommand(btBuffer);
      }
      btBuffer = "";
    } else {
      btBuffer += c;
    }
  }
}

void parseBluetoothCommand(String cmd) {
  if (cmd.startsWith("SETWIFI:")) {
    int comma = cmd.indexOf(',');
    if (comma != -1) {
      String ssid = cmd.substring(8, comma);
      String pass = cmd.substring(comma + 1);
      ssid.trim();
      pass.trim();
      logEvent("WIFI", "SETWIFI received: SSID=" + ssid);
      connectESP01ToWiFi(ssid.c_str(), pass.c_str());
      saveWifiConfigToEEPROM(ssid.c_str(), pass.c_str(), 0, wifiConfig.ozoneTarget);
    }
  } else if (cmd == "CLEARCONFIG") {
    clearEEPROMConfig();
    logEvent("EEPROM", "Cleared config via BT");
  } else if (cmd.startsWith("RELAY:")) {
    setRelay(cmd.endsWith("ON"));
  } else if (cmd.startsWith("FAN:")) {
    setFan(cmd.endsWith("ON"));
  } else if (cmd.startsWith("OZONE:")) {
    float target = cmd.substring(7).toFloat();
    wifiConfig.ozoneTarget = target;
    saveWifiConfigToEEPROM(wifiConfig.ssid, wifiConfig.password, wifiConfig.mode, target);
    logEvent("O3", "Target Set From App: " + String(target, 2) + " ppm");
  } else if (cmd == "STATUS?") {
    String status = "STATUS\n";
    status += "SSID: " + String(wifiConfig.ssid) + "\n";
    status += "OZONE TARGET: " + String(wifiConfig.ozoneTarget) + "\n";
    status += "RELAY: " + String(getRelayStatus() ? "ON" : "OFF") + "\n";
    status += "FAN: " + String(getFanStatus() ? "ON" : "OFF") + "\n\n";
    SerialBT.println(status);
    logEvent("BT", "STATUS? command processed");
  }
}
