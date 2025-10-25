#include <WiFiNINA.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "arsh";       // Replace with your hotspot name
const char* password = "12345678";      // Replace with your hotspot password

// MQTT broker details
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Ultrasonic sensor pins
#define TRIG_PIN 2
#define ECHO_PIN 3

String lastStatus = "";  // Tracks previous bin status

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqttServer, mqttPort);
}

void loop() {
  // Measure distance
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  // Determine bin status
  String currentStatus = "";
  if (distance < 6) {
    currentStatus = "FULL";
  } else if (distance < 12) {
    currentStatus = "HALF";
  } else {
    currentStatus = "EMPTY";
  }

  // Only react if status has changed AND is HALF or FULL
  if ((currentStatus == "HALF" || currentStatus == "FULL") && currentStatus != lastStatus) {
    // Connect to MQTT if not already
    if (!client.connected()) {
      client.connect("BinClient");
    }

    String message = "Bin1:" + currentStatus + ":30.5180,76.6612";
    client.publish("bin/status", message.c_str());
    Serial.println("MQTT → " + message);
    lastStatus = currentStatus;
  }

  // Wait a bit before checking again (no flooding)
  delay(1000); // You can increase this to 10s or 30s if needed
}