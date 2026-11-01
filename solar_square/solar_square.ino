// Solar Square v0.1
// 8x8 WS2812B matrix on an ESP32-C3 SuperMini, running from a solar-charged LiPo.
// Short press: next pattern. Long press: next brightness level.

#include <FastLED.h>
#include <Preferences.h>
#include "config.h"
#include "matrix.h"
#include "patterns.h"

struct Mode {
  const char* name;
  PatternFn draw;
};

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
};
const uint8_t MODE_COUNT = sizeof(MODES) / sizeof(MODES[0]);

Preferences prefs;
uint8_t mode = 0;
uint8_t brightnessLevel = 1;
bool modeStarted = false;
uint32_t lastFrame = 0;
uint32_t saveAt = 0;

bool buttonDown = false;
bool longPressFired = false;
uint32_t pressedAt = 0;
uint32_t lastEdge = 0;
uint32_t flashUntil = 0;

void setMode(uint8_t m) {
  mode = m % MODE_COUNT;
  modeStarted = false;
  FastLED.clear();
  saveAt = millis() + SAVE_DELAY_MS;
  Serial.printf("pattern: %s\n", MODES[mode].name);
}

void setBrightnessLevel(uint8_t level) {
  brightnessLevel = level % BRIGHTNESS_COUNT;
  FastLED.setBrightness(BRIGHTNESS_LEVELS[brightnessLevel]);
  saveAt = millis() + SAVE_DELAY_MS;
  Serial.printf("brightness: %u\n", BRIGHTNESS_LEVELS[brightnessLevel]);
}

void pollButton(uint32_t now) {
  bool down = digitalRead(PIN_BUTTON) == LOW;
  if (down != buttonDown && now - lastEdge > DEBOUNCE_MS) {
    lastEdge = now;
    buttonDown = down;
    if (down) {
      pressedAt = now;
      longPressFired = false;
    } else if (!longPressFired) {
      setMode(mode + 1);
      flashUntil = now + 120;
    }
  }

  if (buttonDown && !longPressFired && now - pressedAt >= LONG_PRESS_MS) {
    longPressFired = true;
    setBrightnessLevel(brightnessLevel + 1);
  }
}

void updateButtonLed(uint32_t now) {
  if (buttonDown || now < flashUntil) {
    analogWrite(PIN_BUTTON_LED, 255);
  } else {
    analogWrite(PIN_BUTTON_LED, beatsin8(12, 6, 80));
  }
}

void saveSettingsIfDue(uint32_t now) {
  if (!saveAt || now < saveAt) return;
  saveAt = 0;
  prefs.putUChar("mode", mode);
  prefs.putUChar("bright", brightnessLevel);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BUTTON_LED, OUTPUT);

  FastLED.addLeds<WS2812B, PIN_MATRIX, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
  random16_add_entropy(esp_random());

  prefs.begin("solarsquare", false);
  mode = prefs.getUChar("mode", 0) % MODE_COUNT;
  brightnessLevel = prefs.getUChar("bright", 1) % BRIGHTNESS_COUNT;
  FastLED.setBrightness(BRIGHTNESS_LEVELS[brightnessLevel]);
  FastLED.clear(true);
}

void loop() {
  uint32_t now = millis();
  pollButton(now);
  updateButtonLed(now);
  saveSettingsIfDue(now);

  if (now - lastFrame >= FRAME_MS) {
    lastFrame = now;
    MODES[mode].draw(now, !modeStarted);
    modeStarted = true;
    FastLED.show();
  }
}
