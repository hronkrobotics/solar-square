# Agent notes

Solar Square is the follow-up to [Solar Eyes](https://github.com/hronkrobotics/solareyes): an ESP32-C3 SuperMini driving an 8x8 WS2812B matrix in a 3D-printed case, powered by a solar-charged 1500 mAh LiPo, with a Wi-Fi control page. See `README.md` for the hardware, wiring, assembly, and pin map. The README follows the same structure as the Solar Eyes README; keep it that way.

## Git and GitHub

- Commit as `Haroon Khalid <haroon@hronk.ca>` (set in this repo's local git config). The global identity is a different account, and Vercel/GitHub attribute commits by email.
- The repo lives at `hronkrobotics/solar-square`. Push with the `hronkrobotics` gh account.

## Building

- PlatformIO lives at `%USERPROFILE%\.platformio\penv\Scripts\pio.exe` (not on PATH).
- `pio run` builds; `pio run -t upload` flashes. The SuperMini shows up as VID 303A, usually on COM8. Its port drops and comes back on every reset, so retry uploads.
- The loop must yield (`delay(2)` when no frame is due). A tight loop starves the C3's native USB and the port disconnects. Uploads run at 115200.
- The shell is Windows PowerShell 5: no `&&`, and `gh --jq` quoting breaks, so pipe to `ConvertFrom-Json`.
- `platformio.ini` pins `espressif32@6.12.0` (Arduino core 2.0.17) with FastLED 3.7.8. FastLED 3.10.x fails to compile on this core.

## Firmware layout

- `state.h` holds the mode, brightness, and drawing, and saves them to NVS (`Preferences`, namespace `solarsquare`).
- `web.h` runs a SoftAP (`SolarSquare-XXXX`, password `solarsquare`) with a captive-portal DNS and the control page at 192.168.4.1. TX power is lowered to 8.5 dBm for the SuperMini's weak antenna.
- `3d/solar-square-case.step` is the Shapr3D export of the case and the matrix retaining bracket, in mm.

## Hardware decisions

- Pins: matrix DIN on GPIO10, momentary button on GPIO1. The button's built-in LED is not wired.
- Kept free on purpose: GPIO3 (accelerometer interrupt, deep-sleep wake capable) and GPIO6/7 (I2C).
- Avoid GPIO2, 8, 9 (strapping) and GPIO21 (bootloader output).
- LED current is capped at 350 mA: the 5 V converter is rated for 600 mA and Wi-Fi takes up to about 200 mA.

## Writing style

The owner dislikes text that reads as AI-written. In README and posts: plain sentences, no emojis, no em dashes.

## Next steps

1. Flash the Wi-Fi version and test the control page on a phone.
2. Confirm the matrix layout with the Drawing pattern (`MATRIX_SERPENTINE` and the flip options in `config.h`).
3. Measure real current draw and battery life, then update the README's estimates.
4. Add build photos to the README.
5. Add an accelerometer (LIS3DH or MPU-6050 GY-521) for tilt-driven sand/water and shake input.
6. Battery gauge on an ADC pin, deep sleep with button wake.
