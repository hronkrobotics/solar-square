#pragma once
#include <FastLED.h>
#include "config.h"

static CRGB leds[NUM_LEDS];

// x is the column from the left and y is the row from the top, seen from the front.
inline uint16_t XY(uint8_t x, uint8_t y) {
  if (MATRIX_TRANSPOSE) {
    uint8_t t = x;
    x = y;
    y = t;
  }
  if (MATRIX_FLIP_X) x = MATRIX_W - 1 - x;
  if (MATRIX_FLIP_Y) y = MATRIX_H - 1 - y;
  if (MATRIX_SERPENTINE && (y & 1)) x = MATRIX_W - 1 - x;
  return y * MATRIX_W + x;
}
