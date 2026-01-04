# Esp32 Thermostat

This is the code and case design for a custom made thermostat for a 5-wire HVAC interface made out of an esp32-s microcontroller, 3 relays, a DS18B20 temperature sensor, and a LM2596HV step-down voltage regulator.

The microcontroller code is currently setup to connect to a WPA-2 secured wifi network with the SSID and password programmed into the microcontroller in `NetworkConfig.h`.

On successful connection to an wifi network the microcontroller will begin to act as a REST API server on port 80 over http with no authentication and respond to HTTP GET, and PATCH requests for page `/`. The code expects that a serial device will be available to the microcontroller for debug output, the device's IP address will be printed there.

The case is designed for very specific breakout boards for each of the components.
It is designed for the following parts:
- Microcontroller [https://www.amazon.com/dp/B0CR5Y2JVD](https://www.amazon.com/dp/B0CR5Y2JVD)
- Relays [https://www.amazon.com/dp/B07XGZSYJV](https://www.amazon.com/dp/B07XGZSYJV)
- Temperature sensor [https://www.amazon.com/dp/B0BPFYQT8C](https://www.amazon.com/dp/B0BPFYQT8C)
- Voltage regulator [https://www.amazon.com/dp/B09F8SLCP9](https://www.amazon.com/dp/B09F8SLCP9)

# Wiring

All VCC pins from relay boards, temperature sensor breakout board, and microcontroller dev board (3.3V pin not 5V pin) should be connected to the + side of the voltage regulator output. All GND pins from those boards should be connected to the - side of the voltage regulator output.

Temperature sensor data pin should be connected to 15 (15).\
Fan relay IN connects to TX (1).\
Cooling relay IN connects to 22 (22).\
Heating relay IN connects to 23 (23).

For the GPIO pins no resistors are needed.

The thermostat 5-wire common should be connected to the NC pin of all of the relays and also one of the voltage regulator inputs, and the thermostat 5-wire 24V power should be conected to the ON pin of the relays as well as the unused one of the voltage regulator inputs.

# Required Libraries

* Arduino Esp32
* Dallas Temperature
* OneWire

# Photos

## After verifying all parts fit and wiring a few of the relays together

![Part fitted into case without wires connecting them](https://github.com/Qnec/Esp32Thermostat/tree/main/images/FittedCleaned.jpg)
