#include "Thermostat.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#define MAX_ULONG (0UL - 1UL)

Thermostat::Thermostat(uint8_t f, uint8_t h, uint8_t c, uint8_t t, unsigned long hcl) {
  fanRelay = f;
  heaterRelay = h;
  coolerRelay = c;
  temperaturePin = t;
  setpoint = 23.3f;
  fanAlwaysOn = false;
  useHeater = false;
  useCooler = false;
  hardwareCooldownLength = hcl;
  lastFanStatusChange = millis();
  lastHeaterStatusChange = millis();
  lastCoolerStatusChange = millis();
  fanOn = false;
  heaterOn = false;
  coolerOn = false;
  tempWire = OneWire(temperaturePin);
  sensor = DallasTemperature(&tempWire);
  sensor.begin();
}

String Thermostat::serialize() {
  String output = "{";
  output.reserve(205);
  output+="\"temperature\":"+String(temp);
  output+=",\"heating\":"+String(getHeating());
  output+=",\"cooling\":"+String(getCooling());
  output+=",\"fanning\":"+String(getFanning());
  output+=",\"setpoint\":"+String(setpoint);
  output+=",\"hardwareCooldownDuration\":"+String(hardwareCooldownLength);
  output+=",\"useHeater\":"+String(useHeater);
  output+=",\"useCooler\":"+String(useCooler);
  output+=",\"useFan\":"+String(fanAlwaysOn);
  output+=",\"lastHeaterToggle\":"+String(lastHeaterStatusChange);
  output+=",\"lastCoolerToggle\":"+String(lastCoolerStatusChange);
  output+=",\"lastFanToggle\":"+String(lastFanStatusChange);
  output+="}";
  return output;
}

void Thermostat::pollSensors() {
  temp = getTemperature();
}

float Thermostat::getTemperature() {
  sensor.requestTemperatures();
  return sensor.getTempCByIndex(0);
}

bool Thermostat::getHeating() {
  return heaterOn;
}

bool Thermostat::getCooling() {
  return coolerOn;
}

bool Thermostat::getFanning() {
  return fanOn;
}

void Thermostat::updateThermostat() {
  pollSensors();
  if(useHeater && temp < setpoint) {
    //Serial.println("trying to heat");
    turnCoolerOff();
    turnHeaterOn();
  } else if(useCooler && temp > setpoint) {
    //Serial.println("trying to cool");
    turnHeaterOff();
    turnCoolerOn();
  } else {
    //Serial.println("trying to nothing");
    turnHeaterOff();
    turnCoolerOff();
  }
  if(getHeating() || getCooling() || fanAlwaysOn) {
    turnFanOn();
  } else {
    turnFanOff();
  }
}

void Thermostat::turnHeaterOn() {
  if(durationMillis(lastHeaterStatusChange, millis()) > hardwareCooldownLength && !getCooling() && !getHeating()) {
    Serial.println("turning heater on");
    heaterOn = true;
    digitalWrite(heaterRelay, HIGH);
    lastHeaterStatusChange = millis();
  }
}

void Thermostat::turnCoolerOn() {
  if(durationMillis(lastCoolerStatusChange, millis()) > hardwareCooldownLength && !getCooling() && !getHeating()) {
    Serial.println("turning cooler on");
    coolerOn = true;
    digitalWrite(coolerRelay, HIGH);
    lastCoolerStatusChange = millis();
  }
}

void Thermostat::turnFanOn() {
  if(durationMillis(lastFanStatusChange, millis()) > hardwareCooldownLength && !getFanning()) {
    Serial.println("turning fan on");
    fanOn = true;
    digitalWrite(fanRelay, HIGH);
    lastFanStatusChange = millis();
  }
}

void Thermostat::turnHeaterOff() {
  if(durationMillis(lastHeaterStatusChange, millis()) > hardwareCooldownLength && getHeating()) {
    Serial.println("turning heater off");
    heaterOn = false;
    digitalWrite(heaterRelay, LOW);
    lastHeaterStatusChange = millis();
  }
}

void Thermostat::turnCoolerOff() {
  if(durationMillis(lastCoolerStatusChange, millis()) > hardwareCooldownLength && getCooling()) {
    Serial.println("turning cooler off");
    coolerOn = false;
    digitalWrite(coolerRelay, LOW);
    lastCoolerStatusChange = millis();
  }
}

void Thermostat::turnFanOff() {
  if(durationMillis(lastFanStatusChange, millis()) > hardwareCooldownLength && getFanning()) {
    Serial.println("turning fan off");
    fanOn = false;
    digitalWrite(fanRelay, LOW);
    lastFanStatusChange = millis();
  }
}

unsigned long Thermostat::durationMillis(unsigned long before, unsigned long after) {
  if(after < before) {
    return (MAX_ULONG-before)+after;
  } else {
    return after-before;
  }
}
