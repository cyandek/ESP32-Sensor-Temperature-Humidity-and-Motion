#include "WiFi.h"
#include "secrets.h"
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiMulti.h>

// Function declarations
void WiFi_SmartConfig();  // Function to conenct to WiFi via app
void WiFi_Hardcoded();    // Function to conenct to WiFi via hardcoded SSID and Password
void Get_Temp_Humidity(); // Function to get Temperature and Humidity readings
void Get_Motion();        // Function to get motion sensor readings
WiFiMulti wifiMulti;

// Sensor declarations
#define DHTTYPE DHT11     // Temp/Humidity sensor module
#define DHTPIN 23         // Temp/Humidity pin
#define PIRPIN 2          // Motion sensor pin
float Temp, Humidity = 0; // Temperature and Humidity variables
int PIRState = LOW;       // Set initial Motion Sensor state to clear. LOW = Clear HIGH = Motion
int val = 0;              // Variable for reading the pin status
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
long lastTemp, lastMotion = 0; // Variables used to calculate intervals
int interval = 10000;          // Interval for temperature and humidity readings
int PIRinterval = 1000;        // Interval for motion sensor readings
bool debug = true;             // switch to debug via terminal

void setup() {
  Serial.begin(115200);

  // Declare PIR sensor as input
  pinMode(PIRPIN, INPUT);

  // Connect to WiFi
  //  WiFi_SmartConfig();     Uncomment to connect via Espressif Esptouch app
  WiFi_Hardcoded();           // Comment out function call if using WiFi_SmartConfig();

  // initialize the Temperature and Humidity sensor
  dht.begin();

  // initialize the mqtt server
  client.setServer(mqtt_server, 1883);
}

// Reconnect to MQTT server
void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Sensor", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    // Call function to reconnect to mqtt server
    reconnect();
  }
  client.loop();

  if ((millis() - lastTemp) > interval) {

    // reset timer
    lastTemp = millis();

    // Call function to get Temperature and Humidity
    Get_Temp_Humidity();
  }

  if ((millis() - lastMotion) > PIRinterval) {

    // reset timer
    lastMotion = millis();

    // Call function to detect motion
    Get_Motion();
  }
}

// Function to configure the WiFI via the Espressif Esptouch app
void WiFi_SmartConfig() {

  // Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  // Wait for SmartConfig packet from mobile
  if (debug == true) {
    Serial.println("Waiting for SmartConfig.");
  }

  while (!WiFi.smartConfigDone()) {
    delay(1000);

    if (debug == true) {
      Serial.print(".");
    }
  }

  if (debug == true) {
    Serial.println("");
    Serial.println("SmartConfig received.");

    // Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debug == true) {
      Serial.print(".");
    }
  }

  if (debug == true) {
    Serial.println("WiFi Connected.");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

// Function to configure the WiFi via hardcoded credentials in secrets.h
void WiFi_Hardcoded() {

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
    WiFi.mode(WIFI_STA); 
    WiFi.disconnect();
    WiFi.reconnect();
  }

  // Print connected to wifi
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);

  if (debug == true) {
    Serial.println("Wifi Connected");
  }
  delay(100);
}

// Function to get Temperature and Humidity readings
void Get_Temp_Humidity() {

  // Temperature in Fahrenheit
  Temp = dht.readTemperature(true);

  // Delay because sensor needs time between readings
  delay(3000);

  // Convert the value to a char array
  char tempString[8];
  dtostrf(Temp, 1, 2, tempString);

  if (debug == true) {
    Serial.print("Temperature: ");
    Serial.println(tempString);
  }

  client.publish(Temp_Topic, tempString);

  Humidity = dht.readHumidity();

  // Convert the value to a char array
  char HumidString[8];
  dtostrf(Humidity, 1, 2, HumidString);

  if (debug == true) {
    Serial.print("Humidity: ");
    Serial.println(HumidString);
  }

  client.publish(Humid_Topic, HumidString);
}

// Function to get motion readings
void Get_Motion() {

  // Set variable to state of motion sensor
  val = digitalRead(PIRPIN);

  if (val == HIGH) {
    client.publish(Motion_Topic, "Motion Detected");

    if (debug == true) {
      Serial.println("Motion Detected");
    }
  }

  if (val == LOW) {
    client.publish(Motion_Topic, "Clear");

    if (debug == true) {
      Serial.println("Clear");
    }
  }
}
