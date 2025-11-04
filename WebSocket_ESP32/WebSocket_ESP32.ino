#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "time.h"

#define WEBSOCKETS_LOGLEVEL 3

#define trig 32
#define echo 33

const char* ssid = "LIC02";
const char* password = "lic02@@2024";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Connected to WebSocket server");
      break;
    case WStype_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
      break;
  }
}

float readDistanceCM() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000); 
  float distance = duration * 0.0343 / 2; 
  return distance;
}

void setup() {
  Serial.begin(115200);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "Time synced: %Y-%m-%d %H:%M:%S");

 // Mudar para o tunnelchannel

  webSocket.beginSSL("rare-planes-peel.loca.lt", 443, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
  // webSocket.setInsecure(); // uncomment in case of SSL errors
}

void loop() {
  webSocket.loop();

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }

  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000 && webSocket.isConnected()) {
    float distance = readDistanceCM();


    if (distance > 0 && distance < 400) {
      StaticJsonDocument<200> doc;
      doc["distance_cm"] = distance;

      String json;
      serializeJson(doc, json);
      webSocket.sendTXT(json);

      Serial.print(" Sent: ");
      Serial.print(distance);
      Serial.println(" cm");
    } else {
      Serial.println(" Invalid distance reading");
    }

    lastSend = millis();
  }
}
