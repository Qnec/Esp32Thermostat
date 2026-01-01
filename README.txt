# Esp32 Thermostat

This is the code and case design for a custom made thermostat for a 5-wire HVAC interface made out of an esp32-s microcontroller, 3 relays, a DS18B20 temperature sensor, and a LM2596HV step-down voltage regulator.

The microcontroller code is currently setup to connect to a WPA-2 secured wifi network with the SSID and password programmed into the microcontroller in `main.ino`.

On successful connection to an wifi network the microcontroller will begin to act as a REST API server on port 80 over http with no authentication and respond to HTTP GET, and PATCH requests for page `/`.

The case is designed for very specific breakout boards for each of the components.
It is designed for the following parts:
- Microcontroller [https://www.amazon.com/dp/B0CR5Y2JVD](https://www.amazon.com/dp/B0CR5Y2JVD)
- Relays [https://www.amazon.com/dp/B07XGZSYJV](https://www.amazon.com/dp/B07XGZSYJV)
- Temperature sensor [https://www.amazon.com/dp/B0BPFYQT8C](https://www.amazon.com/dp/B0BPFYQT8C)
- Voltage regulator [https://www.amazon.com/dp/B09F8SLCP9](https://www.amazon.com/dp/B09F8SLCP9)
