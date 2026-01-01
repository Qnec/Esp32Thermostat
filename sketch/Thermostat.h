#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#define MAX_ULONG (0UL - 1UL)
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class Thermostat {
  public:
  float setpoint;
  bool fanAlwaysOn;
  uint8_t fanRelay;
  unsigned long lastFanStatusChange;
  bool fanOn = false;
  bool useHeater;
  uint8_t heaterRelay;
  unsigned long lastHeaterStatusChange;
  bool heaterOn = false;
  bool useCooler;
  uint8_t coolerRelay;
  unsigned long lastCoolerStatusChange;
  bool coolerOn = false;
  float temp = 0.0f;
  uint8_t temperaturePin;
  OneWire tempWire;
  DallasTemperature sensor;
  unsigned long hardwareCooldownLength;
  Thermostat(uint8_t f, uint8_t h, uint8_t c, uint8_t t, unsigned long hcl);
  String serialize();
  void pollSensors();
  float getTemperature();
  bool getHeating();
  bool getCooling();
  bool getFanning();
  void updateThermostat();
  void turnHeaterOn();
  void turnCoolerOn();
  void turnFanOn();
  void turnHeaterOff();
  void turnCoolerOff();
  void turnFanOff();
  unsigned long durationMillis(unsigned long before, unsigned long after);
};

#endif
