## SQM Meter using ESP32, TSL2591 and MQTT

Simple SQM meter that sends SQM values to a MQTT broker using an ESP32 and ESPHome

I remixed the case from here to cover up the holes and make it a bit better at not letting rain inside the housing. I sealed any opening and the seam between the lid and the main cover body with hot glue.:

https://theawesomegarage.com/blog/multisensor-with-3d-printed-enclosure-for-esphome-and-home-assistant

**Wiring**

Use Adafruit's documentation to wire up the sensor. I used the Adafruit Stemma 4-pin connectors and cables so wiring was a breeze.
https://learn.adafruit.com/adafruit-tsl2591/wiring-and-test

![image](https://github.com/chvvkumar/ESP32-TSL2591-MQTT-SQM/assets/16548147/fe016c09-51f3-4132-8bee-128670cd174f)


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

