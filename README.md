## SQM Meter using ESP32, TSL2591 and MQTT

Simple SQM meter that sends SQM values to a MQTT broker using an ESP32. I have included ArduinoOTA for remote firmware updates as once sealed and mounted outside, it would be hard to take the meter down for updates using a cable.

Credit to Stub Mandrel (Neil Wyatt) for writing the code I've used heavily here

http://www.stubmandrel.co.uk/astronomy/237-sky-quality-meter

I remixed the case from here to cover up the holes and make it a bit better at not letting rain inside the housing. I sealed any opening and the seam between the lid and the main cover body with hot glue.:

https://theawesomegarage.com/blog/multisensor-with-3d-printed-enclosure-for-esphome-and-home-assistant

**Wiring**

Use Adafruit's documentation to wire up the sensor. I used the Adafruit Stemma 4-pin connectors and cables so wiring was a breeze.
https://learn.adafruit.com/adafruit-tsl2591/wiring-and-test

![image](https://github.com/chvvkumar/ESP32-TSL2591-MQTT-SQM/assets/16548147/fe016c09-51f3-4132-8bee-128670cd174f)


**Libraries**

Libraries should be installed first and these can be found by searching the library installer in Arduino IDE for the headers at the top of the main program.

**Secrets**

Secrets are stored in the `arduino_secrets.h` file, customize it to your needs.

**ArduinoOTA Login**

admin/admin


**Home Assistant MQTT Sensor**

    - name: "SQM Meter"
      state_topic: "Astro/SQMMeter"
      value_template: "{{ value_json.SQM }}"
      json_attributes_topic: "Astro/SQMMeter"
      json_attributes_template: "{{ value_json | tojson }}"
      unique_id: bac9xl4sfggfdwpo56vj2g8ye
      unit_of_measurement: "mag/arcsec²"


Screenshots:

![image](https://github.com/chvvkumar/ESP32-TSL2591-MQTT-SQM/assets/16548147/3af5694d-77a5-41e7-b100-ee946fabea12)

