#pragma once
#include "matrix.h"
#include "sprites.h"

// Each pattern draws one frame into leds. `start` is true on the first frame
// after switching to it, so patterns with state can reset.
typedef void (*PatternFn)(uint32_t now, bool start);

void patternSmiley(uint32_t now, bool start) {
  static uint32_t nextBlink;
  if (start) nextBlink = now + 1500;

  bool blinking = now >= nextBlink;
  if (blinking && now >= nextBlink + 140) nextBlink = now + random16(2000, 5000);

  drawSprite(blinking ? SMILEY_BLINK : SMILEY_OPEN);
}

void patternHeart(uint32_t now, bool) {
  uint16_t t = now % 1200;
  bool beat = t < 140 || (t >= 260 && t < 400);

  drawSprite(beat ? HEART_BIG : HEART_SMALL);
  if (!beat) nscale8(leds, NUM_LEDS, 170);
}

void patternInvader(uint32_t now, bool) {
  drawSprite((now / 500) % 2 ? INVADER_B : INVADER_A);
}

void patternRainbow(uint32_t now, bool) {
  uint8_t hue = now / 12;
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      leds[XY(x, y)] = CHSV(hue + (x + y) * 12, 255, 255);
    }
  }
}

void patternPlasma(uint32_t now, bool) {
  uint16_t t = now / 4;
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      uint8_t v = sin8((uint8_t)(x * 28 + t)) / 3
                + sin8((uint8_t)(y * 36 + t * 3 / 2)) / 3
                + sin8((uint8_t)((x + y) * 20 + t / 2)) / 3;
      leds[XY(x, y)] = ColorFromPalette(PartyColors_p, v + (t >> 3));
    }
  }
}

// Fire2012 by Mark Kriegsman, run once per column with the base on the bottom row.
void patternFire(uint32_t now, bool start) {
  static uint8_t heat[MATRIX_W][MATRIX_H];  // [column][0 = bottom]
  static uint32_t last;
  if (start) memset(heat, 0, sizeof(heat));
  if (!start && now - last < 30) return;
  last = now;

  for (uint8_t x = 0; x < MATRIX_W; x++) {
    for (uint8_t i = 0; i < MATRIX_H; i++) {
      heat[x][i] = qsub8(heat[x][i], random8(0, 60));
    }
    for (uint8_t i = MATRIX_H - 1; i >= 2; i--) {
      heat[x][i] = (heat[x][i - 1] + heat[x][i - 2] + heat[x][i - 2]) / 3;
    }
    if (random8() < 140) {
      heat[x][0] = qadd8(heat[x][0], random8(160, 255));
    }
    for (uint8_t i = 0; i < MATRIX_H; i++) {
      leds[XY(x, MATRIX_H - 1 - i)] = HeatColor(heat[x][i]);
    }
  }
}

// Falling green code, one drop per column at its own speed.
void patternRain(uint32_t now, bool start) {
  static int8_t head[MATRIX_W];
  static uint8_t speed[MATRIX_W];
  static uint32_t last;
  static uint16_t tick;
  if (start) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      head[x] = -(int8_t)random8(0, 10);
      speed[x] = random8(1, 4);
    }
  }
  if (!start && now - last < 50) return;
  last = now;
  tick++;

  fadeToBlackBy(leds, NUM_LEDS, 45);
  for (uint8_t x = 0; x < MATRIX_W; x++) {
    if (tick % speed[x]) continue;

    int8_t y = head[x];
    if (y >= 0 && y < MATRIX_H) leds[XY(x, y)] = CRGB(0, 200, 40);
    y++;
    if (y >= MATRIX_H + 2) {
      y = -(int8_t)random8(0, 10);
      speed[x] = random8(1, 4);
    }
    if (y >= 0 && y < MATRIX_H) leds[XY(x, y)] = CRGB(170, 255, 170);
    head[x] = y;
  }
}

void patternConfetti(uint32_t now, bool start) {
  static uint32_t last;
  if (!start && now - last < 30) return;
  last = now;

  fadeToBlackBy(leds, NUM_LEDS, 25);
  leds[random8(NUM_LEDS)] += CHSV(now / 20 + random8(64), 200, 255);
}

// Conway's Game of Life on a board that wraps around at the edges. One bit per
// cell, so the whole 8x8 board fits in a uint64_t.
static uint64_t randomBoard() {
  auto r64 = []() { return ((uint64_t)esp_random() << 32) | esp_random(); };
  return r64() & (r64() | r64());  // about 37% of cells alive
}

static bool lifeCell(uint64_t board, int8_t x, int8_t y) {
  x = (x + MATRIX_W) % MATRIX_W;
  y = (y + MATRIX_H) % MATRIX_H;
  return (board >> (y * MATRIX_W + x)) & 1;
}

static uint64_t lifeStep(uint64_t board) {
  uint64_t next = 0;
  for (int8_t y = 0; y < MATRIX_H; y++) {
    for (int8_t x = 0; x < MATRIX_W; x++) {
      uint8_t n = 0;
      for (int8_t dy = -1; dy <= 1; dy++) {
        for (int8_t dx = -1; dx <= 1; dx++) {
          if ((dx || dy) && lifeCell(board, x + dx, y + dy)) n++;
        }
      }
      bool alive = lifeCell(board, x, y);
      if (n == 3 || (alive && n == 2)) next |= 1ULL << (y * MATRIX_W + x);
    }
  }
  return next;
}

void patternLife(uint32_t now, bool start) {
  static uint64_t board, previous;
  static uint32_t lastStep, stuckSince;
  static uint16_t generation;
  static uint8_t hue;

  if (start) {
    board = randomBoard();
    previous = 0;
    generation = 0;
    stuckSince = 0;
    hue = random8();
  }

  if (now - lastStep >= 250) {
    lastStep = now;
    uint64_t next = lifeStep(board);
    // Still lifes, blinkers and long runs restart with a fresh board.
    bool stuck = next == board || next == previous || generation > 150;
    if (stuck && !stuckSince) stuckSince = now;
    if (stuckSince && now - stuckSince > 1500) {
      next = randomBoard();
      generation = 0;
      stuckSince = 0;
      hue += 40;
    }
    previous = board;
    board = next;
    generation++;
  }

  fadeToBlackBy(leds, NUM_LEDS, 40);
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      if (lifeCell(board, x, y)) leds[XY(x, y)] = CHSV(hue + y * 6, 230, 255);
    }
  }
}

// Pixels painted on the web page, row by row from the top left.
static CRGB canvas[NUM_LEDS];

void patternDrawing(uint32_t, bool) {
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      leds[XY(x, y)] = canvas[y * MATRIX_W + x];
    }
  }
}

struct Mode {
  const char* name;
  PatternFn draw;
};

// The button steps through these in order. The web page lists them the same way.
const Mode MODES[] = {
  {"smiley", patternSmiley},
  {"heart", patternHeart},
  {"invader", patternInvader},
  {"rainbow", patternRainbow},
  {"plasma", patternPlasma},
  {"fire", patternFire},
  {"rain", patternRain},
  {"confetti", patternConfetti},
  {"life", patternLife},
  {"drawing", patternDrawing},
};
const uint8_t MODE_COUNT = sizeof(MODES) / sizeof(MODES[0]);
const uint8_t DRAWING_MODE = MODE_COUNT - 1;
