#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

// === OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Pupil Roll ===
int rollPhase = 0;
const int rollPhases = 40;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();

  // Pupil roll
  float angle = 2 * PI * rollPhase / rollPhases;
  int offsetX = 2 * cos(angle);
  int offsetY = 2 * sin(angle);

  drawPwnagotchiFace(offsetX, offsetY);

  display.display();

  rollPhase = (rollPhase + 1) % rollPhases;
  delay(50);
}

void drawPwnagotchiFace(int offsetX, int offsetY) {
  // === Cheek brackets ===
  // Left bracket
  display.drawCircle(35, 32, 15, SH110X_WHITE);
  display.drawCircle(93, 32, 15, SH110X_WHITE);

  // Erase parts to look like brackets:
  display.fillRect(35 - 10, 17, 10, 30, SH110X_BLACK); // Cut left side
  display.fillRect(93 + 1, 17, 10, 30, SH110X_BLACK); // Cut right side

  // === Eyes (big pupils inside) ===
  display.fillCircle(35 + offsetX, 32 + offsetY, 5, SH110X_WHITE);
  display.fillCircle(93 + offsetX, 32 + offsetY, 5, SH110X_WHITE);

  // === Shine dot on each pupil ===
  display.fillCircle(35 + offsetX - 2, 32 + offsetY - 2, 1, SH110X_BLACK);
  display.fillCircle(93 + offsetX - 2, 32 + offsetY - 2, 1, SH110X_BLACK);

  // === Cute Smile ===
  drawSmile(64, 42, 10, 20, 160);
}

// Arc smile using pixels
void drawSmile(int cx, int cy, int radius, int startAngle, int endAngle) {
  for (int i = startAngle; i < endAngle; i++) {
    float rad = i * 3.14159 / 180;
    int x = cx + radius * cos(rad);
    int y = cy + radius * sin(rad);
    display.drawPixel(x, y, SH110X_WHITE);
  }
}
