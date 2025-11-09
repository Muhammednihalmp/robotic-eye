#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WebServer.h>

// === OLED Configuration ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define EYE_WIDTH 30
#define EYE_HEIGHT 30

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define SDA_PIN 21
#define SCL_PIN 22

// === WiFi AP Configuration ===
const char* ssid = "ESP32_EYES";
const char* password = "12345678";
WebServer server(80);

// === Animation States ===
enum EyeState {IDLE, BLINKING, LOOK_LEFT, LOOK_RIGHT, LOOK_UP, LOOK_DOWN};
EyeState currentState = IDLE;
EyeState returnState = IDLE;
unsigned long stateStartTime = 0;
uint8_t blinkPhase = 0;
int16_t eyeOffsetX = 0;
int16_t eyeOffsetY = 0;

// === Eye Positions ===
const int leftEyeX = 32;
const int rightEyeX = 96;
const int eyeY = 32;

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // Start AP mode
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web server routes
  server.on("/", handleRoot);
  server.on("/blink", []() {
    currentState = BLINKING;
    stateStartTime = millis();
    blinkPhase = 0;
    server.send(200, "text/plain", "Blinking!");
  });
  server.on("/left", []() {
    currentState = LOOK_LEFT;
    stateStartTime = millis();
    returnState = IDLE;
    server.send(200, "text/plain", "Looking Left!");
  });
  server.on("/right", []() {
    currentState = LOOK_RIGHT;
    stateStartTime = millis();
    returnState = IDLE;
    server.send(200, "text/plain", "Looking Right!");
  });
  server.on("/up", []() {
    currentState = LOOK_UP;
    stateStartTime = millis();
    returnState = IDLE;
    server.send(200, "text/plain", "Looking Up!");
  });
  server.on("/down", []() {
    currentState = LOOK_DOWN;
    stateStartTime = millis();
    returnState = IDLE;
    server.send(200, "text/plain", "Looking Down!");
  });
  server.on("/center", []() {
    currentState = IDLE;
    eyeOffsetX = 0;
    eyeOffsetY = 0;
    server.send(200, "text/plain", "Center Position!");
  });

  server.begin();
}

void loop() {
  server.handleClient();
  updateEyes();
}

void updateEyes() {
  switch (currentState) {
    case BLINKING:
      handleBlink();
      break;
    case LOOK_LEFT:
      eyeOffsetX = -8;
      if (millis() - stateStartTime > 2000) currentState = returnState;
      drawEyes(0);
      break;
    case LOOK_RIGHT:
      eyeOffsetX = 8;
      if (millis() - stateStartTime > 2000) currentState = returnState;
      drawEyes(0);
      break;
    case LOOK_UP:
      eyeOffsetY = -8;
      if (millis() - stateStartTime > 2000) currentState = returnState;
      drawEyes(0);
      break;
    case LOOK_DOWN:
      eyeOffsetY = 8;
      if (millis() - stateStartTime > 2000) currentState = returnState;
      drawEyes(0);
      break;
    default:  // IDLE
      eyeOffsetX = 0;
      eyeOffsetY = 0;
      drawEyes(0);
      // Random blinking
      if (millis() - stateStartTime > random(2000, 5000)) {
        currentState = BLINKING;
        blinkPhase = 0;
        stateStartTime = millis();
      }
      break;
  }
  display.display();
}

void handleBlink() {
  unsigned long elapsed = millis() - stateStartTime;

  if (blinkPhase == 0 && elapsed > 50) {
    drawEyes(EYE_HEIGHT / 4);  // Half-closed
    blinkPhase = 1;
  } else if (blinkPhase == 1 && elapsed > 100) {
    drawEyes(0);  // Fully closed
    blinkPhase = 2;
  } else if (blinkPhase == 2 && elapsed > 150) {
    drawEyes(EYE_HEIGHT / 4);  // Half-open
    blinkPhase = 3;
  } else if (blinkPhase == 3 && elapsed > 200) {
    currentState = returnState;
    stateStartTime = millis();
  }
}

void drawEyes(int eyelidOffset) {
  display.clearDisplay();

  // Left eye
  display.fillCircle(leftEyeX + eyeOffsetX, eyeY + eyeOffsetY, EYE_WIDTH / 2, SH110X_WHITE);
  display.fillCircle(leftEyeX + eyeOffsetX, eyeY + eyeOffsetY, 4, SH110X_BLACK);

  // Right eye
  display.fillCircle(rightEyeX + eyeOffsetX, eyeY + eyeOffsetY, EYE_WIDTH / 2, SH110X_WHITE);
  display.fillCircle(rightEyeX + eyeOffsetX, eyeY + eyeOffsetY, 4, SH110X_BLACK);

  // Eyelids
  if (eyelidOffset > 0) {
    display.fillRect(leftEyeX - EYE_WIDTH / 2 + eyeOffsetX,
                     eyeY - EYE_HEIGHT / 2 + eyeOffsetY,
                     EYE_WIDTH,
                     eyelidOffset,
                     SH110X_BLACK);

    display.fillRect(rightEyeX - EYE_WIDTH / 2 + eyeOffsetX,
                     eyeY - EYE_HEIGHT / 2 + eyeOffsetY,
                     EYE_WIDTH,
                     eyelidOffset,
                     SH110X_BLACK);
  }
}

// === Modern Web Interface ===
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Eye Controller</title>
    <style>
      * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', sans-serif; }
      body {
        background: linear-gradient(135deg, #1a2a6c, #b21f1f, #fdbb2d);
        min-height: 100vh; display: flex; justify-content: center; align-items: center; padding: 20px;
      }
      .container {
        max-width: 800px; background: rgba(255, 255, 255, 0.15);
        backdrop-filter: blur(10px); border-radius: 20px;
        box-shadow: 0 10px 30px rgba(0,0,0,0.3); overflow: hidden; width: 100%;
      }
      header {
        background: rgba(0, 0, 0, 0.7); color: white; text-align: center; padding: 25px;
      }
      h1 {
        font-size: 2.5rem; margin-bottom: 10px;
        background: linear-gradient(to right, #ff7e5f, #feb47b);
        -webkit-background-clip: text; -webkit-text-fill-color: transparent;
        text-shadow: 0 2px 4px rgba(0,0,0,0.2);
      }
      .controls {
        display: grid; grid-template-columns: repeat(3, 1fr); gap: 15px; padding: 30px;
      }
      .btn {
        border: none; border-radius: 50px; padding: 20px; font-size: 1.2rem; font-weight: bold;
        cursor: pointer; transition: all 0.3s ease; box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        color: white; background: rgba(0, 0, 0, 0.4);
        display: flex; flex-direction: column; align-items: center; justify-content: center;
      }
      .btn:hover { transform: translateY(-5px); box-shadow: 0 8px 20px rgba(0,0,0,0.3); }
      .btn:active { transform: translateY(0); }
      .btn i { font-size: 2.5rem; margin-bottom: 10px; }
      .blink-btn { background: linear-gradient(45deg, #3498db, #2980b9); }
      .left-btn { background: linear-gradient(45deg, #2ecc71, #27ae60); }
      .right-btn { background: linear-gradient(45deg, #e74c3c, #c0392b); }
      .up-btn { background: linear-gradient(45deg, #9b59b6, #8e44ad); }
      .down-btn { background: linear-gradient(45deg, #f39c12, #d35400); }
      .center-btn { grid-column: span 3; background: linear-gradient(45deg, #1abc9c, #16a085); padding: 25px; }
      .status { text-align: center; padding: 20px; color: white; font-size: 1.2rem; background: rgba(0, 0, 0, 0.2); }
      @media (max-width: 600px) {
        .controls { grid-template-columns: repeat(2, 1fr); }
        .center-btn { grid-column: span 2; }
      }
    </style>
  </head>
  <body>
    <div class="container">
      <header>
        <h1>🤖 ESP32 Eye Controller</h1>
        <p>Control robotic eyes in real-time</p>
      </header>
      <div class="controls">
        <button class="btn blink-btn" onclick="sendCommand('blink')"><i>👁️</i> BLINK</button>
        <button class="btn up-btn" onclick="sendCommand('up')"><i>👆</i> UP</button>
        <button class="btn left-btn" onclick="sendCommand('left')"><i>👈</i> LEFT</button>
        <button class="btn center-btn" onclick="sendCommand('center')"><i>🎯</i> CENTER</button>
        <button class="btn right-btn" onclick="sendCommand('right')"><i>👉</i> RIGHT</button>
        <button class="btn down-btn" onclick="sendCommand('down')"><i>👇</i> DOWN</button>
      </div>
      <div class="status">
        <p>Connected to: ESP32_EYES | IP: )rawliteral";
  html += WiFi.softAPIP().toString();
  html += R"rawliteral(</p></div></div>
    <script>
      function sendCommand(cmd) {
        fetch('/' + cmd)
          .then(response => response.text())
          .then(data => console.log(data))
          .catch(err => console.error('Error:', err));
      }
    </script>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}
