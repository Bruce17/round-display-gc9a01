#include "../../include/general-inc.h"
#include <Arduino_GFX_Library.h>

#define DEBUG_GAUGE   false

/**
 * Prepare TFT display pins
 */

#define TFT_RST     33
#define TFT_BL      22
#define TFT_SCK     18 // SPI serial clock (SCK)
#define TFT_MOSI    23 // SPI MOSI (master ouput to slave input)
#define TFT_MISO    -1 // SPI MOSI (master input to slave output)
#define TFT_CS      5  // Chip select/slave select
// #define TFT_CS      -1 // for display without CS pin
#define TFT_DC      2  // RS/DC SPI signal
// #define TFT_DC      -1 // for display without DC pin (9-bit SPI)


/**
 * Define gauge parameters
 */

#define BACKGROUND BLACK
#define MARK_COLOR WHITE

// Defines the gauge range. This is needed to convert a gauge display value into the proper angle (see angle range [MIN_ANGLE, MAX_ANGLE]).
// Define values as float to avoid int conversions during calculations.
#define RANGE_MIN     400.0
#define RANGE_MAX     2000.0

// angle in degree
#define MIN_ANGLE     135.0
#define MAX_ANGLE     405.0

// Define color range for gauge
#define COLOR_RANGE_SIZE    3
const float color_range[2][COLOR_RANGE_SIZE] = {
  {RANGE_MIN, 950.0, 1250.0},
  {GREEN, ORANGE, RED}
};

#define NEEDLE_LINE       1
#define NEEDLE_TRIANGLE   2
#define GAUGE_NEEDLE      NEEDLE_TRIANGLE

// Colors

#define NEEDLE_COLOR      ORANGE

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define RADIAN_TO_DEGREES   57.29577951308232087679   // shortend of 57.295779513082320876798154814105 = 180 / PI
#define DEGREES_TO_RADIAN   0.01745329251994329576    // shortend of 0.01745329251994329576923690768489 = PI / 180

#define degToRad(angleInDegrees) ((angleInDegrees) * DEGREES_TO_RADIAN)
#define radToDeg(angleInRadians) ((angleInRadians) * RADIAN_TO_DEGREES)


void prepare_gauge();

void draw_gauge();
void draw_gauge_mark();
void draw_gauge_needle(int16_t val, int16_t original_value);
uint16_t color_from_range(int16_t val);

/**
 * Convert a input value from one range into a target range in degrees and return its value.
 *  
 * @param val Convert this value into the target range.
 *
 * @return Returns the transformed input value.
 * 
 * @see https://stackoverflow.com/questions/19057341/translate-numbers-from-a-range-to-another-range
 */
int16_t transform_val_to_deg(int16_t val);
