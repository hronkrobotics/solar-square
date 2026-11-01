#pragma once
#include "matrix.h"

// One string per row, top to bottom. Each character is one pixel; see spriteColor().
typedef const char* const Sprite[MATRIX_H];

const Sprite SMILEY_OPEN = {
  "..YYYY..",
  ".YYYYYY.",
  "YY.YY.YY",
  "YY.YY.YY",
  "YYYYYYYY",
  "Y.YYYY.Y",
  ".Y....Y.",
  "..YYYY..",
};

const Sprite SMILEY_BLINK = {
  "..YYYY..",
  ".YYYYYY.",
  "YYYYYYYY",
  "Y..YY..Y",
  "YYYYYYYY",
  "Y.YYYY.Y",
  ".Y....Y.",
  "..YYYY..",
};

const Sprite HEART_BIG = {
  ".RR..RR.",
  "RPRRRRRR",
  "RRRRRRRR",
  "RRRRRRRR",
  ".RRRRRR.",
  "..RRRR..",
  "...RR...",
  "........",
};

const Sprite HEART_SMALL = {
  "........",
  "........",
  "..R..R..",
  ".RRRRRR.",
  ".RRRRRR.",
  "..RRRR..",
  "...RR...",
  "........",
};

const Sprite INVADER_A = {
  "...GG...",
  "..GGGG..",
  ".GGGGGG.",
  "GG.GG.GG",
  "GGGGGGGG",
  "..G..G..",
  ".G.GG.G.",
  "G.G..G.G",
};

const Sprite INVADER_B = {
  "...GG...",
  "..GGGG..",
  ".GGGGGG.",
  "GG.GG.GG",
  "GGGGGGGG",
  ".G.GG.G.",
  "G......G",
  ".G....G.",
};

inline CRGB spriteColor(char c) {
  switch (c) {
    case 'Y': return CRGB(255, 160, 0);
    case 'R': return CRGB(255, 0, 24);
    case 'P': return CRGB(255, 130, 150);
    case 'G': return CRGB(40, 255, 0);
    case 'B': return CRGB(0, 90, 255);
    case 'W': return CRGB(255, 255, 255);
    default:  return CRGB::Black;
  }
}

inline void drawSprite(const Sprite& rows) {
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      leds[XY(x, y)] = spriteColor(rows[y][x]);
    }
  }
}
