## SQM Meter using ESP32, TSL2591 and MQTT

Simple SQM meter that sends SQM values to a MQTT broker using an ESP32.

**Wiring**

Use Adafruit's documentation to wire up the sensor. I used the Adafruit Stemma 4-pin connectors and cables so wiring was a breeze.
https://learn.adafruit.com/adafruit-tsl2591/wiring-and-test

**Libraries**

Libraries should be installed first and these can be found by searching the library installer in Arduino IDE for the headers at the top of the main program.

**Secrets**

Secrets are stored in the `arduino_secrets.h` file, customize it to your needs.

**ArduinoOTA Login**

admin/admin
