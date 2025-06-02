// File: f_to_e_v7_3.ino
#include <EEPROM.h>
#include "eeprom_storage.h"
#include "at_comm_esp01.h"
#include "bluetooth_handler.h"
#include "relay_control.h"
#include "ozone_sensor.h"
#include "web_server.h"
#include "utils.h"

#define EEPROM_SIZE 128

void setup() {
  Serial.begin(115200);
  logEvent("SYS", "System booting...");
  EEPROM.begin(EEPROM_SIZE);
  loadWifiConfigFromEEPROM();
  setupRelayAndFan();
  setupBluetooth();
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  logEvent("SYS", "Serial2 initialized for ESP-01");

  if (wifiConfig.configured) {
    logEvent("WIFI", "Found config. Attempting to connect...");
    connectESP01ToWiFi(wifiConfig.ssid, wifiConfig.password);
  } else {
    logEvent("WIFI", "No config found. Awaiting SETWIFI command.");
  }

  setupServer();
}

void loop() {
  handleBluetoothCommands();
  updateOzoneSensor();
}
