#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

#define WIFI_SSID "TP-Link_0633"
#define WIFI_PASS "PASS"

WebServer server(80);

const int adcPin = 1;
const int wakeupPin = 2; // added pin for external wake up
const uint64_t SLEEP_DURATION = 30e6; // 30 seconds
const unsigned long AWAKE_TIME = 30000;

RTC_DATA_ATTR int bootCount = 0; // added boot count, uses RTC memory to store value

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
      <h1>ESP32 Deep Sleep Demo</h1>
      <!-- Added boot count display -->
      <p>Boot Count: <span id='bootCount'></span></p>
      <p>Voltage reading: <span id='reading'></span>V</p>
      <!-- Added sleep timer display -->
      <p>Time until sleep: <span id='sleepTimer'></span> seconds</p>
      <script>
        function getReading() {
          fetch('/reading')
            .then(response => response.json())
            .then(data => {
              document.getElementById('bootCount').textContent = data.bootCount;
              document.getElementById('reading').textContent = data.voltage;
              });
        }
        function updateSleepTimer() {
          let timeLetf = 30 - (Math.floor((Date.now()/1000) % 30));
          document.getElementById('sleepTimer').textContent = timeLetf;
        }
        getReading();
        setInterval(updateSleepTimer, 1000);
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
    String json = "{\"voltage\":" + String(reading, 2) + ",\"bootCount\":" + String(bootCount) + "}";
    server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  ++bootCount;
  Serial.println("Boot count: " + String(bootCount));

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

  // added code to wake up
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)wakeupPin, LOW);


    Serial.println("Setup ESP32 to sleep for " + String(SLEEP_DURATION / 1000000) +
  " Seconds");
}

void loop() {
  server.handleClient();

  if (millis() > AWAKE_TIME) { // Check if awake time has passed
    Serial.println("Going to deep sleep now");
    Serial.flush();
    esp_deep_sleep_start(); // Enter deep sleep
  }
}
