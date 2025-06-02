// File: relay_control.h
#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

void setupRelayAndFan();
void setRelay(bool on);
void setFan(bool on);
bool getRelayStatus();
bool getFanStatus();

#endif
