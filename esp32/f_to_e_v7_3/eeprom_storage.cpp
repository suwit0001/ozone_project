// File: eeprom_storage.cpp
#include <EEPROM.h>
#include "eeprom_storage.h"
#include "utils.h"

#define EEPROM_SSID_ADDR     1
#define EEPROM_PASS_ADDR     34
#define EEPROM_MODE_ADDR     67
#define EEPROM_FLAG_ADDR     0
#define EEPROM_OZONE_ADDR    68

WifiConfig wifiConfig;

void saveWifiConfigToEEPROM(const char* ssid, const char* password, uint8_t mode, float ozoneTarget) {
  EEPROM.write(EEPROM_FLAG_ADDR, 1);
  for (int i = 0; i < 32; i++) {
    EEPROM.write(EEPROM_SSID_ADDR + i, ssid[i]);
    EEPROM.write(EEPROM_PASS_ADDR + i, password[i]);
  }
  EEPROM.write(EEPROM_MODE_ADDR, mode);
  byte* ozonePtr = (byte*)(void*)&ozoneTarget;
  for (int i = 0; i < sizeof(float); i++) {
    EEPROM.write(EEPROM_OZONE_ADDR + i, ozonePtr[i]);
  }
  EEPROM.commit();
  logEvent("EEPROM", "Config saved: SSID=" + String(ssid) + ", Mode=" + String(mode) + ", O3 Target=" + String(ozoneTarget));
}

void loadWifiConfigFromEEPROM() {
  wifiConfig.configured = EEPROM.read(EEPROM_FLAG_ADDR) == 1;
  if (!wifiConfig.configured) return;
  for (int i = 0; i < 32; i++) {
    wifiConfig.ssid[i] = EEPROM.read(EEPROM_SSID_ADDR + i);
    wifiConfig.password[i] = EEPROM.read(EEPROM_PASS_ADDR + i);
  }
  wifiConfig.ssid[31] = '\0';
  wifiConfig.password[31] = '\0';
  wifiConfig.mode = EEPROM.read(EEPROM_MODE_ADDR);
  byte* ozonePtr = (byte*)(void*)&wifiConfig.ozoneTarget;
  for (int i = 0; i < sizeof(float); i++) {
    ozonePtr[i] = EEPROM.read(EEPROM_OZONE_ADDR + i);
  }
  logEvent("EEPROM", "Config loaded: SSID=" + String(wifiConfig.ssid) + ", Mode=" + String(wifiConfig.mode) + ", O3 Target=" + String(wifiConfig.ozoneTarget));
}

void clearEEPROMConfig() {
  for (int i = 0; i < 128; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  logEvent("EEPROM", "All config cleared");
}