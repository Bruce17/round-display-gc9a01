
/*******************************************************************************
 * Start of Arduino_GFX setting
 ******************************************************************************/
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
 * clock settings
 */

#define BACKGROUND BLACK
#define MARK_COLOR WHITE
#define SUBMARK_COLOR DARKGREY // LIGHTGREY
#define HOUR_COLOR WHITE
#define MINUTE_COLOR BLUE // LIGHTGREY
#define SECOND_COLOR RED

#define SIXTIETH 0.016666667
#define TWELFTH 0.08333333
#define SIXTIETH_RADIAN 0.10471976
#define TWELFTH_RADIAN 0.52359878
#define RIGHT_ANGLE_RADIAN 1.5707963

/**
 * Defines clock update interval:
 * 
 * 1:    smooth pointer movement
 * 1000: simulate a ticking pointer like on quartz clocks every second
 */
#define UPDATE_INTERVAL 25


void draw_round_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3);

void draw_square_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3);

void write_cache_pixel(int16_t x, int16_t y, int16_t color, bool cross_check_second, bool cross_check_hour);

void draw_and_erase_cached_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color, int16_t *cache, int16_t cache_len, bool cross_check_second, bool cross_check_hour);

void redraw_hands_cached_draw_and_erase();

void prepare_display();

void draw_clock(unsigned long cur_millis);
void draw_clock_tm(struct tm* info);

void recalc_and_draw_clock();
