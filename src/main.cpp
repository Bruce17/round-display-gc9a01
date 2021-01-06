/**
 * Example ESP32 clock using a round TFT display.
 * 
 * @see https://github.com/moononournation/Arduino_GFX/blob/master/examples/Clock/Clock.ino
 */

#include "general-inc.h"
#include "clock-display.h"
#include "wifi-manager.h"
#include "time.h"


const char* ntpServer = "de.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


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



void setup(void) {
#ifdef DEBUG
  Serial.begin(115200);
#endif

  prepare_wifi();
  prepare_display();

  // If wifi is not connected, restart
  if (WiFi.status() == WL_CONNECT_FAILED) {
  #ifdef DEBUG
    Serial.println("No WiFi -> restart ESP");
  #endif
    esp_restart();
  }

  // Get time from NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#ifdef DEBUG
  printLocalTime();
#endif
}

void loop() {
  wifi_loop();

  // Update clock with time from NTP
  if (millis() % 1000 == 0) {
    drawClockWithCurrentTime();
  }
}
