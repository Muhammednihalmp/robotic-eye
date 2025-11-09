#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

// === OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define SDA_PIN 21
#define SCL_PIN 22

// === WiFi Beacon ===
WebServer server(80);
bool beaconSpamActive = false;

uint8_t beaconPacket[128] = {
  0x80, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0xc0, 0x6c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x64, 0x00,
  0x01, 0x04,
  0x00, 0x08, 'F','R','E','E','_','W','I','F',
  0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,
  0x03, 0x01, 0x01
};

// === Pins ===
#define LED_PIN 2
#define BUTTON_START 12
#define BUTTON_STOP  14

// === Eye Animation State ===
int rollEyesPhase = 0;
const int rollPhases = 40;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_STOP, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  drawNormalEyes();
  display.display();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32-Spammer", "12345678");

  server.on("/", handleRoot);
  server.on("/start", []() {
    startAttack();
    server.send(200, "text/plain", "Attack started");
  });
  server.on("/stop", []() {
    stopAttack();
    server.send(200, "text/plain", "Attack stopped");
  });
  server.begin();

  Serial.println("WiFi Spammer Ready!");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();

  if (digitalRead(BUTTON_START) == LOW) {
    startAttack();
    delay(200);
  }
  if (digitalRead(BUTTON_STOP) == LOW) {
    stopAttack();
    delay(200);
  }

  if (beaconSpamActive) {
    sendBeacon();
    handleAngryEyes();
    delay(50);
  }
}

void sendBeacon() {
  for (int i = 10; i < 16; i++) beaconPacket[i] = random(256);
  for (int i = 38; i < 46; i++) if (i > 41) beaconPacket[i] = random(33, 126);
  beaconPacket[68] = 1; // Fixed channel
  esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, sizeof(beaconPacket), false);

  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>😈 WiFi Spammer</title>";
  html += "<style>body{font-family:Arial;text-align:center;margin-top:50px;}";
  html += "button{padding:10px 20px;font-size:16px;margin:10px;}</style></head>";
  html += "<body><h1>ESP32 WiFi Spammer 😈</h1>";
  html += "<p>Status: <strong>" + String(beaconSpamActive ? "ACTIVE" : "🙂") + "</strong></p>";
  html += "<button onclick=\"startSpam()\">START</button>";
  html += "<button onclick=\"stopSpam()\">STOP</button>";
  html += "<script>function startSpam(){fetch('/start').then(r=>r.text()).then(alert)}";
  html += "function stopSpam(){fetch('/stop').then(r=>r.text()).then(alert)}</script></body></html>";
  server.send(200, "text/html", html);
}

void startAttack() {
  if (!beaconSpamActive) {
    beaconSpamActive = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Attack started");
  }
}

void stopAttack() {
  if (beaconSpamActive) {
    beaconSpamActive = false;
    digitalWrite(LED_PIN, LOW);
    drawNormalEyes();
    display.display();
    Serial.println("Attack stopped");
  }
}

void handleAngryEyes() {
  display.clearDisplay();

  float angle = 2 * PI * rollEyesPhase / rollPhases;
  int offsetX = 5 * cos(angle);
  int offsetY = 3 * sin(angle);

  drawAngryEyes(offsetX, offsetY);

  display.display();

  rollEyesPhase = (rollEyesPhase + 1) % rollPhases;
}

void drawAngryEyes(int offsetX, int offsetY) {
  // Eye outlines
  display.drawCircle(32, 32, 15, SH110X_WHITE);
  display.drawCircle(96, 32, 15, SH110X_WHITE);

  // Angry brows
  display.drawLine(17, 20, 47, 10, SH110X_WHITE); // Left brow
  display.drawLine(81, 10, 111, 20, SH110X_WHITE); // Right brow

  // Pupils
  display.fillCircle(32 + offsetX, 32 + offsetY, 5, SH110X_WHITE);
  display.fillCircle(96 + offsetX, 32 + offsetY, 5, SH110X_WHITE);
}

void drawNormalEyes() {
  display.clearDisplay();
  // Friendly open eyes with small calm pupils
  display.drawCircle(32, 32, 15, SH110X_WHITE);
  display.fillCircle(32, 32, 4, SH110X_WHITE);

  display.drawCircle(96, 32, 15, SH110X_WHITE);
  display.fillCircle(96, 32, 4, SH110X_WHITE);
}
