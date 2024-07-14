#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

#define WIFI_SSID "TP-Link_0633"
#define WIFI_PASS "PASS_W"

WebServer server(80);

const int adcPin = 1;  

float readVoltage() {
  int adcValue = analogRead(adcPin);
  return (adcValue/4095.0) * 3.3;
}

void sendHtml() {
  String response = R"(
  <!DOCTYPE html><html>
    <head>
      <title>ESP32-S3 Web Server Demo</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        html { font-family: Arial; text-align: center; }
        body { display: inline-flex; flex-direction: column; }
        h1 { margin-bottom: 1.2em; }
      </style>
    </head>
    <body>
      <h1>ESP32-S3 Web Server</h1>
      <p>Hey buddy</p>
      <p>Voltage reading: <span id='reading'></span>V</p>
      <button onclick="getReading() ">Update Reading</button>
      <script>
        function getReading() {
            fetch('/reading')
            .then(response => response.text())
            .then(reading => document.getElementById('reading').textContent = reading);
        }
        getReading();
        </script>
    </body>
  </html>
  )";

  server.send(200, "text/html", response);
}

void handleHello() {
  server.send(200, "text/plain", "Hello from ESP32-S3!");
}

void handleReading() {
    float reading = readVoltage();
    server.send(200, "text/plain", String(reading, 2));  // Two decimal places
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", sendHtml);
  server.on("/hello", handleHello); 
  server.on("/reading", handleReading);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(2);
}
