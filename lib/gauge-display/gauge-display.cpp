#include "gauge-display.h"

// ESP32 hardware spi
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, VSPI);

// GC9A01 IPS LCD 240x240
Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);


static int16_t w, h, center;
// The needle's length to show the current value and second value is the needle's extension into the negative direction to extend it backwards over the center dot.
static int16_t needle_length, needle_length_neg;


void draw_gauge_mark() {
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
  float mdeg;

  int16_t sHandLen = center * 5 / 6;
  int16_t markLen = sHandLen / 6;
  int16_t innerR1 = center - markLen;
  int16_t outerR1 = center;

  for (int16_t i = MIN_ANGLE; i < MAX_ANGLE; i++) {
    if (i % 45 == 0) {
      innerR = innerR1;
      outerR = outerR1;
      c = MARK_COLOR;
    } else {
      continue;
    }

    mdeg = (SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN;
    x = cos(mdeg);
    y = sin(mdeg);
    x0 = x * outerR + center;
    y0 = y * outerR + center;
    x1 = x * innerR + center;
    y1 = y * innerR + center;

    gfx->drawLine(x0, y0, x1, y1, c);
  }
}

void draw_gauge_needle(int16_t val) {
  gfx->startWrite();

  // Draw center dot
  gfx->drawCircle(center, center, 5, DARKGREY);

  // Draw actual needle
  float x = cos(val);
  float y = sin(val);
  int16_t x0 = x * needle_length + center;
  int16_t y0 = y * needle_length + center;
  int16_t x1 = x * needle_length_neg + center;
  int16_t y1 = y * needle_length_neg + center;

  gfx->drawLine(x0, y0, x1, y1, ORANGE);

  gfx->endWrite();
}

void draw_gauge() {
  // TODO: get current value
  // Range: [400, 2000]
  int16_t raw_val = 845;

  // Convert value into angle between MIN_ANGLE and MAX_ANGLE
  int16_t val = transform_val_to_deg(raw_val);
#ifdef DEBUG
  Serial.print("Raw value: ");
  Serial.println(raw_val);

  Serial.print("Converted into deg: ");
  Serial.println(val);
#endif

  // Draw gauge needle with current value
  draw_gauge_needle(val);
}

int16_t transform_val_to_deg(int16_t val) {
  return (val - RANGE_MIN) * (MAX_ANGLE / RANGE_MAX) + MIN_ANGLE;
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

  draw_gauge_mark();
}
