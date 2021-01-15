#include "gauge-display.h"

// ESP32 hardware spi
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, VSPI);

// GC9A01 IPS LCD 240x240
Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);


static int16_t w, h, center;
// The needle's length to show the current value and second value is the needle's extension into the negative direction to extend it backwards over the center dot.
static int16_t needle_length, needle_length_neg;


void draw_gauge_mark() {
  // Clear screen first
  gfx->fillRect(0, 0, w, h, BACKGROUND);

  // Draw outer arc ring
  gfx->drawArc(
    // center coordinates
    center, center,
    // innter/outer radius
    center - 1, center - 2,
    // start/end angle
    MIN_ANGLE, MAX_ANGLE,
    // ark color
    MARK_COLOR
  );


  // Draw marks on arc
  float x, y;
  int16_t x0, x1, y0, y1, innerR, outerR;
  uint16_t c;
  float rad;

  int16_t sHandLen = center * 5 / 6;
  int16_t markLen = sHandLen / 6;
  int16_t innerR1 = center - markLen;
  int16_t outerR1 = center;

  for (int16_t deg = MIN_ANGLE; deg <= MAX_ANGLE; deg++) {
    if (deg % 45 == 0) {
    #ifdef DEBUG
    #if DEBUG_GAUGE
      Serial.println("--------------------");
      Serial.printf("deg = %d\n", deg);
    #endif
    #endif

      innerR = innerR1;
      outerR = outerR1;
      c = MARK_COLOR;
    } else {
      continue;
    }

    rad = degToRad(deg);
    x = cos(rad);
    y = sin(rad);
    x0 = x * outerR + center;
    y0 = y * outerR + center;
    x1 = x * innerR + center;
    y1 = y * innerR + center;

  #ifdef DEBUG
  #if DEBUG_GAUGE
    Serial.printf("rad = %f\n", rad);
    Serial.printf("x = %f\ty = %f\n", x, y);
    Serial.printf("x0 = %d\ty0 = %d\n", x0, y0);
    Serial.printf("x1 = %d\ty1 = %d\n", x1, y1);
  #endif
  #endif

    gfx->drawLine(x0, y0, x1, y1, c);
  }
}

void draw_gauge_needle(int16_t deg, int16_t original_value) {
  // Use start/end write and draw pixels in beetween for faster drawing and to avoid flickering.
  gfx->startWrite();

  // Clear background first.
  gfx->fillEllipseHelper(center, center, needle_length + 7, needle_length + 7, 3, 0, BACKGROUND);

  // Draw center dot
  gfx->fillEllipseHelper(center, center, 10, 10, 3, 0, DARKGREY);

  // Draw actual needle
  float rad = degToRad(deg);
  float x = cos(rad);
  float y = sin(rad);

  int16_t x0 = x * needle_length + center;
  int16_t y0 = y * needle_length + center;

#ifdef DEBUG
#if DEBUG_GAUGE
  Serial.println("--------------------");
  Serial.printf("deg = %d\n", deg);
  Serial.printf("rad = %f\n", rad);
  Serial.printf("x = %f\ty = %f\n", x, y);
  Serial.printf("x0 = %d\ty0 = %d\n", x0, y0);

  // Just draw a dot for debugging
  gfx->fillEllipseHelper(x0, y0, 5, 5, 3, 0, GREEN);
#endif
#endif

  // Draw needle
#if GAUGE_NEEDLE == NEEDLE_LINE
  // Multiply by -1 to draw needle a little bit into the opposite direction
  int16_t x1 = x * needle_length_neg * -1 + center;
  int16_t y1 = y * needle_length_neg * -1 + center;

  #ifdef DEBUG
  #if DEBUG_GAUGE
    Serial.printf("x1 = %d\ty1 = %d\n", x1, y1);
  #endif
  #endif

  gfx->writeLine(x0, y0, x1, y1, NEEDLE_COLOR);

  gfx->endWrite();
#elif GAUGE_NEEDLE == NEEDLE_TRIANGLE
  gfx->endWrite();

  float rad2 = degToRad(deg + 90);
  float rad3 = degToRad(deg + 270);
  int8_t offset = 3;

  int16_t x1 = cos(rad2) * offset + center - x * needle_length_neg;
  int16_t y1 = sin(rad2) * offset + center - y * needle_length_neg;
  int16_t x2 = cos(rad3) * offset + center - x * needle_length_neg;
  int16_t y2 = sin(rad3) * offset + center - y * needle_length_neg;

  #ifdef DEBUG
  #if DEBUG_GAUGE
    Serial.printf("x1 = %d\ty1 = %d\n", x1, y1);
    Serial.printf("x2 = %d\ty2 = %d\n", x2, y2);
  #endif
  #endif

  gfx->fillTriangle(x0, y0, x1, y1, x2, y2, NEEDLE_COLOR);
#endif

  // Print out original value as text
  gfx->setTextColor(color_from_range(original_value));
  gfx->setCursor(center * 0.6, center * 1.5);

  // For numbers less 1000 add a whitespace to align it properly everytime onto the display.
  if (original_value < 1000) {
    gfx->print(" ");
  }

  gfx->printf("%d ppm", original_value);
}

uint16_t color_from_range(int16_t val) {
  uint16_t color = MARK_COLOR;

  for (int8_t i = 0; i < COLOR_RANGE_SIZE; i++) {
    if (val >= color_range[0][i]) {
      color = color_range[1][i];
    }
  }

  return color;
}

void draw_gauge() {
  // TODO: get current value
  // Range: [400, 2000]
  // int16_t raw_val = 845;
  int16_t raw_val = random(RANGE_MIN, RANGE_MAX);
  int16_t adjusted_val = raw_val;

  if (adjusted_val < RANGE_MIN) {
    adjusted_val = RANGE_MIN;
  } else if (adjusted_val > RANGE_MAX) {
    adjusted_val = RANGE_MAX;
  }

  // Convert value into angle between MIN_ANGLE and MAX_ANGLE
  int16_t val = transform_val_to_deg(adjusted_val);
#ifdef DEBUG
#if DEBUG_GAUGE
  Serial.println("--------------------");
  Serial.printf("Raw value: %d\n", adjusted_val);
  Serial.printf("Converted into deg: %d\n", val);
#endif
#endif

  // Draw gauge needle with current value
  draw_gauge_needle(val, raw_val);
}

int16_t transform_val_to_deg(int16_t val) {
  // Notice: static values must be of type float to avoid calculation errors due to int conversion.
  return (val - RANGE_MIN) * (MAX_ANGLE - MIN_ANGLE) / (RANGE_MAX - RANGE_MIN) + MIN_ANGLE;
}

void prepare_gauge() {
  gfx->begin();
  gfx->fillScreen(BACKGROUND);

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

  // init LCD constant
  w = gfx->width();
  h = gfx->height();

  if (w < h) {
    center = w / 2;
  }
  else {
    center = h / 2;
  }

  needle_length = center * 2/3;
  needle_length_neg = center * 1/5;

  // Prepare default font settings
  gfx->setTextColor(MARK_COLOR);
  gfx->setTextSize(2);

  draw_gauge_mark();
}
