#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

#define ROW 4
#define COLUMN 1
#define FONT Arial_Black_16

DMD led_module(ROW, COLUMN);

void scan_module() {
  led_module.scanDisplayBySPI();
}

void setup() {
  Serial.begin(115200);  // Match ESP32 baud rate
  Timer1.initialize(2000);
  Timer1.attachInterrupt(scan_module);
  led_module.clearScreen(true);
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      led_module.clearScreen(true);
      led_module.selectFont(FONT);
      led_module.drawMarquee(msg.c_str(), msg.length(), (32 * ROW), 0);
      long start = millis();
      long timing = start;
      boolean flag = false;
      while (!flag) {
        if ((timing + 50) < millis()) {
          flag = led_module.stepMarquee(-1, 0);
          timing = millis();
        }
      }
    }
  }
}
