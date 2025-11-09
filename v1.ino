#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WebServer.h>

// === Function Prototypes ===
void drawEyes(int offsetXLeft, int offsetXRight, int offsetY, bool closeLeft, bool closeRight);
void drawSingleEye(int x, int y, bool closed, bool isLeft);
void handleBlink();
void handleRollEyes();
void setupServerRoutes();
void handleRoot();

// === OLED Configuration ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === WiFi AP Configuration ===
const char* ssid = "ESP32_EYES";
const char* password = "12345678";  // Min 8 chars
WebServer server(80);

// === Animation States ===
enum EyeState { IDLE, BLINK, LOOK_LEFT, LOOK_RIGHT, WINK_LEFT, WINK_RIGHT, LOOK_UP, LOOK_DOWN, ROLL_EYES };
EyeState currentState = IDLE;

// === Eye Animation Parameters ===
struct EyeParams {
  int leftX;  // Base X for left eye
  int rightX; // Base X for right eye
  int y;      // Base Y for eyes
  int size;   // Eye size
};

EyeParams eyeParams = {20, 78, 30, 30};
unsigned long lastAnimationTime = 0;
const int ANIMATION_DELAY = 100;
const int BLINK_DURATION = 200;
const int ROLL_EYES_DURATION = 1000; // Duration for one full eye roll cycle
int rollEyesPhase = 0; // Tracks phase of eye roll animation

// === Setup ===
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for Serial to initialize

  // Initialize I2C and OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;); // Halt on failure
  }
  display.clearDisplay();
  drawEyes(0, 0, 0, false, false);
  display.display();

  // Start WiFi AP
  if (!WiFi.softAP(ssid, password)) {
    Serial.println(F("Failed to start AP"));
    for (;;); // Halt on failure
  }
  IPAddress IP = WiFi.softAPIP();
  Serial.print(F("Access Point started. Connect to WiFi: "));
  Serial.println(ssid);
  Serial.print(F("AP IP address: "));
  Serial.println(IP);

  // Setup web server routes
  setupServerRoutes();
  server.begin();
  Serial.println(F("Web server started"));
}

// === Loop ===
void loop() {
  server.handleClient();

  unsigned long currentTime = millis();
  if (currentTime - lastAnimationTime < ANIMATION_DELAY) return;
  lastAnimationTime = currentTime;

  switch (currentState) {
    case BLINK:
      handleBlink();
      break;
    case WINK_LEFT:
      drawEyes(0, 0, 0, true, false);
      currentState = IDLE;
      break;
    case WINK_RIGHT:
      drawEyes(0, 0, 0, false, true);
      currentState = IDLE;
      break;
    case LOOK_LEFT:
      drawEyes(-5, -5, 0, false, false);
      break;
    case LOOK_RIGHT:
      drawEyes(5, 5, 0, false, false);
      break;
    case LOOK_UP:
      drawEyes(0, 0, -5, false, false);
      break;
    case LOOK_DOWN:
      drawEyes(0, 0, 5, false, false);
      break;
    case ROLL_EYES:
      handleRollEyes();
      break;
    default:
      drawEyes(0, 0, 0, false, false);
      break;
  }
  display.display();
}

// === Web Server Routes ===
void setupServerRoutes() {
  server.on("/", handleRoot);
  server.on("/blink", []() {
    currentState = BLINK;
    server.send(200, "text/plain", "Blinking!");
  });
  server.on("/left", []() {
    currentState = LOOK_LEFT;
    server.send(200, "text/plain", "Looking Left!");
  });
  server.on("/right", []() {
    currentState = LOOK_RIGHT;
    server.send(200, "text/plain", "Looking Right!");
  });
  server.on("/up", []() {
    currentState = LOOK_UP;
    server.send(200, "text/plain", "Looking Up!");
  });
  server.on("/down", []() {
    currentState = LOOK_DOWN;
    server.send(200, "text/plain", "Looking Down!");
  });
  server.on("/roll", []() {
    currentState = ROLL_EYES;
    rollEyesPhase = 0; // Reset roll animation
    server.send(200, "text/plain", "Rolling Eyes!");
  });
  server.on("/wink_left", []() {
    currentState = WINK_LEFT;
    server.send(200, "text/plain", "Winking Left!");
  });
  server.on("/wink_right", []() {
    currentState = WINK_RIGHT;
    server.send(200, "text/plain", "Winking Right!");
  });
  server.on("/stop", []() {
    currentState = IDLE;
    rollEyesPhase = 0; // Reset any ongoing animation state
    drawEyes(0, 0, 0, false, false); // Force redraw to center
    display.display();
    server.send(200, "text/plain", "Stopped!");
  });
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });
}

// === Web Page with Tailwind CSS ===
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Eye Control</title>
  <script src="https://cdn.tailwindcss.com"></script>
  <style>
    body { background-color: #1a202c; color: #e2e8f0; }
    .container { max-width: 600px; }
    button { transition: all 0.2s; }
    button:hover { transform: scale(1.05); }
  </style>
</head>
<body class="flex items-center justify-center min-h-screen">
  <div class="container mx-auto p-4 text-center">
    <h1 class="text-3xl font-bold mb-6">👀 ESP32 Eye Control</h1>
    <div class="grid grid-cols-2 gap-4">
      <button onclick="fetch('/blink')" class="bg-blue-500 hover:bg-blue-600 text-white font-semibold py-2 px-4 rounded">Blink</button>
      <button onclick="fetch('/wink_left')" class="bg-green-500 hover:bg-green-600 text-white font-semibold py-2 px-4 rounded">Wink Left</button>
      <button onclick="fetch('/wink_right')" class="bg-green-500 hover:bg-green-600 text-white font-semibold py-2 px-4 rounded">Wink Right</button>
      <button onclick="fetch('/left')" class="bg-purple-500 hover:bg-purple-600 text-white font-semibold py-2 px-4 rounded">Look Left</button>
      <button onclick="fetch('/right')" class="bg-purple-500 hover:bg-purple-600 text-white font-semibold py-2 px-4 rounded">Look Right</button>
      <button onclick="fetch('/up')" class="bg-purple-500 hover:bg-purple-600 text-white font-semibold py-2 px-4 rounded">Look Up</button>
      <button onclick="fetch('/down')" class="bg-purple-500 hover:bg-purple-600 text-white font-semibold py-2 px-4 rounded">Look Down</button>
      <button onclick="fetch('/roll')" class="bg-indigo-500 hover:bg-indigo-600 text-white font-semibold py-2 px-4 rounded">Roll Eyes</button>
      <button onclick="fetch('/stop')" class="bg-red-500 hover:bg-red-600 text-white font-semibold py-2 px-4 rounded">Stop</button>
    </div>
  </div>
</body>
</html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// === Animation Functions ===
void drawEyes(int offsetXLeft, int offsetXRight, int offsetY, bool closeLeft, bool closeRight) {
  display.clearDisplay();
  drawSingleEye(eyeParams.leftX + offsetXLeft, eyeParams.y + offsetY, closeLeft, true);
  drawSingleEye(eyeParams.rightX + offsetXRight, eyeParams.y + offsetY, closeRight, false);
}

void drawSingleEye(int x, int y, bool closed, bool isLeft) {
  if (closed) {
    display.drawLine(x, y + 5, x + eyeParams.size, y + 5, SH110X_WHITE);
  } else {
    // Draw triangular eyes
    display.fillTriangle(x, y, x + eyeParams.size, y - 10, x + eyeParams.size, y + 10, SH110X_WHITE);
    display.fillTriangle(x, y, x + eyeParams.size, y + 10, x, y + 20, SH110X_WHITE);
  }
}

void handleBlink() {
  drawEyes(0, 0, 0, true, true);
  display.display();
  delay(BLINK_DURATION);
  drawEyes(0, 0, 0, false, false);
  currentState = IDLE;
}

void handleRollEyes() {
  const int phases = 8; // Number of steps in the roll animation
  int offsetX = 5 * cos(2 * PI * rollEyesPhase / phases);
  int offsetY = 5 * sin(2 * PI * rollEyesPhase / phases);
  drawEyes(offsetX, offsetX, offsetY, false, false);
  rollEyesPhase = (rollEyesPhase + 1) % phases;
  if (rollEyesPhase == 0) {
    currentState = IDLE; // Return to idle after one full cycle
  }
}