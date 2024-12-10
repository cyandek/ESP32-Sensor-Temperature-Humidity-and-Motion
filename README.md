# ESP32-Sensor-Temperature-Humidity-and-Motion
ESP32 project to get temperature, humidity, and motion readings and publish them to an MQTT server on Home Assistant. 


secrets.h will need to be updated for your WiFi credentials, MQTT server IP address, MQTT credentials, and potentially the MQTT Topic.


WiFi can be configured by the Espressif Esptouch app if you uncommenting the WiFi_SmartConfig function.



**Hardware:**

ESP32 developement board

DHT11 Temperature and Humidity Sensor Module

HC-SR501 PIR Motion Sensor Module



**Software:**

Mosquitto MQTT Broker Home Assistant integration

https://www.home-assistant.io/integrations/mqtt


