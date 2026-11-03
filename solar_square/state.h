#pragma once
#include <Preferences.h>
#include "patterns.h"

// Current pattern, brightness, and drawing, shared by the button and the web page.
// Changes are written to flash a few seconds after the last one.

Preferences prefs;
uint8_t mode = 0;
uint8_t brightness = DEFAULT_BRIGHTNESS;
bool modeStarted = false;
bool canvasDirty = false;
uint32_t saveAt = 0;

void scheduleSave() {
  saveAt = millis() + SAVE_DELAY_MS;
  if (!saveAt) saveAt = 1;
}

void setMode(uint8_t m) {
  mode = m % MODE_COUNT;
  modeStarted = false;
  FastLED.clear();
  scheduleSave();
  Serial.printf("pattern: %s\n", MODES[mode].name);
}

void setBrightness(uint8_t value) {
  brightness = value ? value : 1;
  FastLED.setBrightness(brightness);
  scheduleSave();
}

// Jumps to the next preset above the current brightness, wrapping to the lowest.
void stepBrightness() {
  for (uint8_t i = 0; i < BRIGHTNESS_COUNT; i++) {
    if (BRIGHTNESS_LEVELS[i] > brightness) {
      setBrightness(BRIGHTNESS_LEVELS[i]);
      return;
    }
  }
  setBrightness(BRIGHTNESS_LEVELS[0]);
}

void canvasChanged() {
  canvasDirty = true;
  if (mode != DRAWING_MODE) {
    setMode(DRAWING_MODE);
  } else {
    scheduleSave();
  }
}

void loadSettings() {
  prefs.begin("solarsquare", false);
  mode = prefs.getUChar("mode", 0) % MODE_COUNT;
  brightness = prefs.getUChar("brightness", DEFAULT_BRIGHTNESS);
  prefs.getBytes("canvas", canvas, sizeof(canvas));
  FastLED.setBrightness(brightness);
}

void saveSettingsIfDue(uint32_t now) {
  if (!saveAt || (int32_t)(now - saveAt) < 0) return;
  saveAt = 0;
  prefs.putUChar("mode", mode);
  prefs.putUChar("brightness", brightness);
  if (canvasDirty) {
    prefs.putBytes("canvas", canvas, sizeof(canvas));
    canvasDirty = false;
  }
}
