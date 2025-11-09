#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WebServer.h>

// === OLED Configuration ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define SDA_PIN 21
#define SCL_PIN 22

// === WiFi AP Configuration ===
const char* ssid = "ESP32_EYES";
const char* password = "12345678";
WebServer server(80);

// === States ===
bool eyesOn = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/start", []() {
    eyesOn = true;
    drawEyesCenter();
    server.send(200, "text/plain", "Eyes ON");
  });
  server.on("/stop", []() {
    eyesOn = false;
    display.clearDisplay();
    display.display();
    server.send(200, "text/plain", "Eyes OFF");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}

void drawEyesCenter() {
  display.clearDisplay();
  // Left angry eye
  display.fillTriangle(20, 30, 50, 20, 50, 40, SH110X_WHITE);
  display.fillTriangle(20, 30, 50, 40, 20, 50, SH110X_WHITE);

  // Right angry eye
  display.fillTriangle(78, 20, 108, 30, 108, 50, SH110X_WHITE);
  display.fillTriangle(78, 20, 78, 40, 108, 30, SH110X_WHITE);

  display.display();
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html><html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>ESP32 Angry Eyes</title>
      <style>
        body { display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; background: #111; color: white; font-family: sans-serif; }
        button { padding: 20px; margin: 20px; font-size: 2rem; border: none; border-radius: 10px; cursor: pointer; }
        .on { background: green; color: white; }
        .off { background: red; color: white; }
      </style>
    </head>
    <body>
      <h1>ESP32 Angry Eyes</h1>
      <button class="on" onclick="fetch('/start')">START</button>
      <button class="off" onclick="fetch('/stop')">STOP</button>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}
