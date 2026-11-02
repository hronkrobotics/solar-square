#pragma once
#include <Arduino.h>

// Pins on the ESP32-C3 SuperMini. GPIO2, 8 and 9 are strapping pins, so they are
// left alone. GPIO3 and GPIO6/7 are kept free for a future accelerometer.
constexpr uint8_t PIN_MATRIX = 10;     // WS2812B DIN, through a 330 ohm resistor
constexpr uint8_t PIN_BUTTON = 1;      // momentary button to GND

constexpr uint8_t MATRIX_W = 8;
constexpr uint8_t MATRIX_H = 8;
constexpr uint16_t NUM_LEDS = MATRIX_W * MATRIX_H;

// Rigid 8x8 boards usually wire every row left to right. Flexible panels usually
// zigzag. If the smiley looks scrambled, set MATRIX_SERPENTINE to true. If it is
// mirrored or rotated, use the flip and transpose options.
constexpr bool MATRIX_SERPENTINE = false;
constexpr bool MATRIX_FLIP_X = false;
constexpr bool MATRIX_FLIP_Y = false;
constexpr bool MATRIX_TRANSPOSE = false;

// The 5 V converter is rated for 600 mA, and the SuperMini needs some of that.
// FastLED dims the whole frame whenever a frame would draw more than this.
constexpr uint32_t MAX_MILLIAMPS = 450;

// A long press steps through these. WS2812Bs are very bright up close.
constexpr uint8_t BRIGHTNESS_LEVELS[] = {8, 24, 64, 128};
constexpr uint8_t BRIGHTNESS_COUNT = sizeof(BRIGHTNESS_LEVELS) / sizeof(BRIGHTNESS_LEVELS[0]);

constexpr uint16_t FRAME_MS = 16;
constexpr uint16_t DEBOUNCE_MS = 25;
constexpr uint16_t LONG_PRESS_MS = 700;
constexpr uint16_t SAVE_DELAY_MS = 3000;
