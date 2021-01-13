#include "../../include/general-inc.h"
#include <Arduino_GFX_Library.h>

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

#define RANGE_MIN     400
#define RANGE_MAX     2000

// angle in degree
#define MIN_ANGLE     -225
#define MAX_ANGLE     45

#define SIXTIETH_RADIAN 0.10471976
#define RIGHT_ANGLE_RADIAN 1.5707963


void prepare_gauge();

void draw_gauge();
void draw_gauge_mark();
void draw_gauge_needle(int16_t val);

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
