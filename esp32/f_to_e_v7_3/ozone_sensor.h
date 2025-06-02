// File: ozone_sensor.h
#ifndef OZONE_SENSOR_H
#define OZONE_SENSOR_H

void updateOzoneSensor();
float convertToPPM(int analogValue);
float getCurrentOzone();

#endif
