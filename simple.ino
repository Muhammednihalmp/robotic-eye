#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// OLED size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create SH1106 display object
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(0x3C, true)) { // true = reset display
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  drawEyes();
  display.display();
}

void loop() {
  // Nothing
}

void drawEyes() {
  display.clearDisplay();

  // Left eye
  display.fillTriangle(20, 30, 50, 20, 50, 40, SH110X_WHITE);
  display.fillTriangle(20, 30, 50, 40, 20, 50, SH110X_WHITE);

  // Right eye
  display.fillTriangle(78, 20, 108, 30, 108, 50, SH110X_WHITE);
  display.fillTriangle(78, 20, 78, 40, 108, 30, SH110X_WHITE);
}
