
#include <FastLED.h>
#include "Button.h"
#include "MPU6886.h"

#define NUM_LEDS         25
#define BRIGHTNESS        1
#define IMU_THRESHOLDS  0.7

Button Btn = Button(39, true, 10);
MPU6886 IMU;


CRGB leds[NUM_LEDS];


float accX = 0, accY = 0, accZ = 0;
float gyroX = 0, gyroY = 0, gyroZ = 0;
float temp = 0;
bool IMU6886Flag = false;

uint32_t data[5][5];


#define PIN       27 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 25 // Popular NeoPixel ring size

// Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ400);

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        27 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 25 // Popular NeoPixel ring size


bool _2048_random_next() {
  long rnd = random(16);
  Serial.printf("random: %d\r\n", rnd);
  // start with rnd
  for (int i = rnd; i < rnd + 5 * 5; i++) {
    size_t pos = i % 25;
    size_t row = pos / 5;
    size_t col = pos % 5;
    uint32_t value = data[row][col];
    if (value == 0) {
      Serial.printf("change %d,%d from 0 to 2\r\n", row, col, value);
      data[row][col] = 2;
      return false;
    }
    // else try next position
  }
  Serial.printf("end of game, return true\r\n");
  return true;
}

void _2048_dump() {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 5; col++) {
      Serial.printf("%d ", data[row][col]);
    }
    Serial.printf("\r\n");
  }
}

void setup() {
  //  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  //  pixels.setBrightness(8); // Set BRIGHTNESS to about 1/5 (max = 255)
  //  pixels.clear(); // Set all pixel colors to 'off'

  FastLED.addLeds<NEOPIXEL, 27>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);

  if (IMU.Init() != 0) {
    IMU6886Flag = false;
  } else {
    IMU6886Flag = true;
  }
  int v = analogRead(5);
  Serial.printf("seed: %d\r\n", v);
  randomSeed(v);
  // initialize two 2 block
  _2048_random_next();
  _2048_random_next();
}

void _2048_led_show() {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 5; col++) {
      uint32_t value = data[row][col];
      Serial.printf("%d,%d=%d ", row, col, value);
      if (value == 0) {
        CRGB*a = &leds[row * 5 + col];
        a->setRGB(0, 0, 0);
        // pixels.setPixelColor(row * 5 + col, pixels.Color(0, 0, 0));
        // M5.dis.drawpix(row * 5 + col, 0x000000);
      } else {
        CRGB*a = &leds[row * 5 + col];
        a->setRGB(0x80, 0x00, 0x80);
        // pixels.setPixelColor(row * 5 + col, pixels.Color(0x01, 0, 0));
        // M5.dis.drawpix(row * 5 + col, 0x010000);
      }
    }
    Serial.printf("\r\n");
  }
  // FastLED.show();
  FastLED.delay(25);
  // pixels.show();   // Send the updated pixel colors to the hardware.
}

enum Direction {Up, Down, Left, Right};

bool _2048_direction_left() {
  bool changed = false;
  for (int row = 0; row < 5; row++) {
    uint32_t* p = data[row];
    for (int col = 4; col > 0; col--) {
      if (p[col - 1] == 0 && p[col] > 0) {
        changed = true;
        p[col - 1] = p[col];
        p[col] = 0;
      }
    }
    _2048_dump();
  }
  return changed;
}

bool _2048_direction_right() {
  bool changed = false;
  for (int row = 0; row < 5; row++) {
    uint32_t* p = data[row];
    for (int col = 0; col < 4; col++) {
      if (p[col + 1] == 0 && p[col] > 0) {
        changed = true;
        p[col + 1] = p[col];
        p[col] = 0;
      }
    }
    _2048_dump();
  }
  return changed;
}

bool _2048_direction_up() {
  bool changed = false;
  for (int col = 0; col < 5; col++) {
    for (int row = 4; row > 0; row--) {
      if (data[row - 1][col] == 0 && data[row][col] > 0) {
        changed = true;
        data[row - 1][col] = data[row][col];
        data[row][col] = 0;
      }
    }
    _2048_dump();
  }
  return changed;
}
bool _2048_direction_down() {
  bool changed = false;
  for (int col = 0; col < 5; col++) {
    for (int row = 0; row < 4; row++) {
      if (data[row + 1][col] == 0 && data[row][col] > 0) {
        changed = true;
        data[row + 1][col] = data[row][col];
        data[row][col] = 0;
      }
    }
    _2048_dump();
  }
  return changed;
}
void _2048_change_direction(Direction d) {
  bool changed = false;
  switch (d) {
    case Up:
      changed = _2048_direction_up();
      break;
    case Down:
      changed = _2048_direction_down();
      break;
    case Left:
      changed = _2048_direction_left();
      break;
    case Right:
      changed = _2048_direction_right();
      break;
  }
  if (changed) {
    _2048_random_next();
  }
}

void loop() {
  if (IMU6886Flag == true) {
    IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
    IMU.getAccelData(&accX, &accY, &accZ);
    IMU.getTempData(&temp);
    Serial.printf("%.2f,%.2f,%.2f o/s \r\n", gyroX, gyroY, gyroZ);
    Serial.printf("%.2f,%.2f,%.2f mg\r\n", accX * 1000, accY * 1000, accZ * 1000);
    Serial.printf("Temperature : %.2f C \r\n", temp);
  }
  if (accX > IMU_THRESHOLDS && abs(accY) < 0.6) {
    Serial.printf("right\r\n");
    _2048_change_direction(Right);
  }
  if (accX < -IMU_THRESHOLDS && abs(accY) < 0.6) {
    Serial.printf("left\r\n");
    _2048_change_direction(Left);
  }
  if (accY > IMU_THRESHOLDS && abs(accX) < 0.6) {
    Serial.printf("down\r\n");
    _2048_change_direction(Down);
  }
  if (accY < -IMU_THRESHOLDS && abs(accX) < 0.6) {
    Serial.printf("up\r\n");
    _2048_change_direction(Up);
  }
  // _2048_random_next();
  _2048_led_show();
  // _2048_dump();
  Btn.read();
  delay(100);
  if (Btn.wasPressed()) {
    ESP.restart();
  }
}
