/**
 * Example ESP32 clock using a round TFT display.
 * 
 * @see https://github.com/moononournation/Arduino_GFX/blob/master/examples/Clock/Clock.ino
 */

#include "clock-display.h"


void setup(void) {
  prepare_clock();
}

void loop() {
  draw_clock(millis());
}
