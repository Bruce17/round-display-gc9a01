/**
 * Example ESP32 clock using a round TFT display.
 * 
 * @see https://github.com/moononournation/Arduino_GFX/blob/master/examples/Clock/Clock.ino
 */

#include "general-inc.h"
// #include "clock-display.h"
#include "gauge-display.h"
#include "wifi-manager.h"
#include "time.h"

const char* ntpServer = "de.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

/*
void printLocalTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
  #ifdef DEBUG
    Serial.println("Failed to obtain time");
  #endif
    return;
  }

#ifdef DEBUG
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
#endif
}

void drawClockWithCurrentTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
  #ifdef DEBUG
    Serial.println("Failed to obtain time");
  #endif
    return;
  }

#ifdef DEBUG
#if DEBUG_TIME
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  Serial.print("hh = "); Serial.print(timeinfo.tm_hour);
  Serial.print("\tmm = "); Serial.print(timeinfo.tm_min);
  Serial.print("\tss = "); Serial.print(timeinfo.tm_sec);
  Serial.println();
#endif
#endif

  draw_clock_tm(&timeinfo);
}
*/


void setup(void) {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // FIXME: disabled only for faster gauge debugging!
  // prepare_wifi();
  // prepare_clock();

//   // Get time from NTP server
//   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
// #ifdef DEBUG
//   printLocalTime();
// #endif

  /*** gauge example ***/
  prepare_gauge();
}

void loop() {
  // FIXME: disabled only for faster gauge debugging!
  // wifi_loop();

  /*** clock example ***/

  // Update clock with time from NTP
  // if (millis() % 1000 == 0) {
  //   drawClockWithCurrentTime();
  // }

  if (millis() % 10000 == 0) {
    draw_gauge();
  }
}
