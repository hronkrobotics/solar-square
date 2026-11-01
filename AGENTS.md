# Agent notes

Solar Square is the follow-up to [Solar Eyes](https://github.com/hronkrobotics/solareyes): an ESP32-C3 SuperMini driving an 8x8 WS2812B matrix, powered by a solar-charged 1500 mAh LiPo. See `README.md` for the hardware, wiring, and pin map.

## Git and GitHub

- Commit as `Haroon Khalid <haroon@hronk.ca>` (set in this repo's local git config). The global identity is a different account, and Vercel/GitHub attribute commits by email.
- The repo lives at `hronkrobotics/solar-square`. Push with the `hronkrobotics` gh account.

## Building

- PlatformIO lives at `%USERPROFILE%\.platformio\penv\Scripts\pio.exe` (not on PATH).
- `pio run` builds; `pio run -t upload` flashes. The SuperMini has been on COM6 before.
- The shell is Windows PowerShell 5: no `&&`, and `gh --jq` quoting breaks, so pipe to `ConvertFrom-Json`.
- `platformio.ini` pins `espressif32@6.12.0` (Arduino core 2.0.17) with FastLED 3.7.8. FastLED 3.10.x fails to compile on this core.

## Hardware decisions

- Pins: matrix DIN on GPIO10, momentary button on GPIO1, button LED on GPIO20.
- Kept free on purpose: GPIO3 (accelerometer interrupt, deep-sleep wake capable) and GPIO6/7 (I2C).
- Avoid GPIO2, 8, 9 (strapping) and GPIO21 (bootloader output).
- LED current is capped at 450 mA because the 5 V converter is rated for 600 mA.

## Writing style

The owner dislikes text that reads as AI-written. In README and posts: plain sentences, no emojis, no em dashes.

## Next steps

1. Flash and confirm the matrix layout (`MATRIX_SERPENTINE` and the flip options in `config.h`).
2. Measure real current draw and battery life, then update the README's estimates.
3. Add an accelerometer (LIS3DH or MPU-6050 GY-521) for tilt-driven sand/water and shake input.
4. Battery gauge on an ADC pin, deep sleep with button wake.
