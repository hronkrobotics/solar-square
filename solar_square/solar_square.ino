// Solar Square v0.1
// 8x8 WS2812B matrix on an ESP32-C3 SuperMini, running from a solar-charged LiPo.
// Short press: next pattern. Long press: next brightness level.
// Phone control: join the SolarSquare-XXXX Wi-Fi network (see config.h).

#include <FastLED.h>
#include "config.h"
#include "matrix.h"
#include "patterns.h"
#include "state.h"
#include "web.h"

uint32_t lastFrame = 0;

bool buttonDown = false;
bool longPressFired = false;
uint32_t pressedAt = 0;
uint32_t lastEdge = 0;

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
    }
  }

  if (buttonDown && !longPressFired && now - pressedAt >= LONG_PRESS_MS) {
    longPressFired = true;
    stepBrightness();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, PIN_MATRIX, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
  random16_add_entropy(esp_random());

  loadSettings();
  FastLED.clear(true);

  startWeb();
}

void loop() {
  uint32_t now = millis();
  pollButton(now);
  handleWeb();
  saveSettingsIfDue(now);

  if (now - lastFrame < FRAME_MS) {
    // The C3 is single-core. A tight spin here starves USB and the port drops.
    delay(2);
    return;
  }
  lastFrame = now;
  MODES[mode].draw(now, !modeStarted);
  modeStarted = true;
  FastLED.show();
}
