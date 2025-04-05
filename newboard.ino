#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include "SystemFont5x7.h"

#define ROW 2
#define COLUMN 2
#define FONT SystemFont5x7

DMD led_module(COLUMN, ROW);  // COLUMN first, then ROW

void scan_module() {
  led_module.scanDisplayBySPI();
}

void setup() {
  Serial.begin(115200);
  Timer1.initialize(2000);
  Timer1.attachInterrupt(scan_module);
  led_module.clearScreen(true);
  led_module.selectFont(FONT);
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      led_module.clearScreen(true);

      const int charWidth = 6;   // 5px + 1px spacing
      const int charHeight = 8;  // 7px + 1px spacing
      const int screenWidth = 32 * COLUMN;
      const int screenHeight = 16 * ROW;

      int charsPerLine = screenWidth / charWidth;
      int totalLines = ceil((float)msg.length() / charsPerLine);
      int textHeight = totalLines * charHeight;

      int startScrollY = screenHeight;
      int endScrollY = -textHeight + charHeight; // Ensure last line is fully shown

      for (int scrollY = startScrollY; scrollY >= endScrollY; scrollY--) {
        led_module.clearScreen(true);

        for (int i = 0; i < totalLines; i++) {
          int startIdx = i * charsPerLine;
          int endIdx = min(startIdx + charsPerLine, msg.length());
          String line = msg.substring(startIdx, endIdx);
          int y = scrollY + (i * charHeight);

          if (y >= -charHeight && y < screenHeight) {
            led_module.drawString(0, y, line.c_str(), line.length(), GRAPHICS_NORMAL);
          }
        }

        delay(100);
      }
    }
  }
}
