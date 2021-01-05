/**
 * Example ESP32 clock using a round TFT display.
 * 
 * @see https://github.com/moononournation/Arduino_GFX/blob/master/examples/Clock/Clock.ino
 */

#include "general-inc.h"
#include "clock-display.h"
#include "wifi-manager.h"


void setup(void) {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  prepare_wifi();
  prepare_clock();
}

void loop() {
  draw_clock(millis());
  wifi_loop();
}
