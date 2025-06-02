#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <Arduino.h>

void setupBluetooth();
void handleBluetoothCommands();
void parseBluetoothCommand(String cmd);

#endif
