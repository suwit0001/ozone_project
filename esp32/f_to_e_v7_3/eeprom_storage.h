// File: eeprom_storage.h
#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

struct WifiConfig {
  bool configured = false;
  char ssid[32];
  char password[32];
  uint8_t mode;
  float ozoneTarget;
};

extern WifiConfig wifiConfig;

void saveWifiConfigToEEPROM(const char* ssid, const char* password, uint8_t mode, float ozoneTarget);
void loadWifiConfigFromEEPROM();
void clearEEPROMConfig();

#endif
