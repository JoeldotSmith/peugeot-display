# Peugeot Display ESP32 Firmware

Custom dashboard display firmware for a Waveshare ESP32-S3-LCD-5B. The firmware uses LVGL to render a 1024x340 dash UI and reads Peugeot CAN bus data through the ESP32 TWAI controller.

The current display shows:

- boost or vacuum pressure
- coolant temperature
- spoiler position
- clock value derived from the vehicle clock counter
- CAN bus receive rate
- automatic screen dimming when the vehicle lights are on

## Hardware

- Board: Waveshare ESP32-S3-LCD-5B
- PlatformIO board target: `esp32-s3-devkitc1-n16r8`
- Framework: Arduino
- Display/UI: LVGL 9.5
- CAN controller: ESP32 TWAI
- CAN bitrate: 500 kbit/s
- CAN TX pin: GPIO 15
- CAN RX pin: GPIO 16

The Waveshare board has onboard flash, so the clock offset is stored in ESP32 NVS flash storage. A TF/SD card is not required for the clock setting feature.

## Clock Setup

The Peugeot clock signal used here behaves like a counter. When the vehicle battery is disconnected, the fixed offset can become wrong. To avoid reflashing firmware just to correct the displayed time, the ESP32 accepts a USB serial clock command and stores the selected offset in NVS.

To set the clock:

1. Power the display while the vehicle CAN bus is active.
2. Connect to the ESP32 USB serial console at `1000000` baud.
3. Wait until at least one `0x552` clock frame has been received.
4. Send the current 24-hour local time:

```text
clock HH:MM[:SS]
```

Example:

```text
clock 18:42:00
```

Useful serial commands:

```text
clock status
clock reset
clock help
```

The firmware compares the entered local time-of-day to the latest CAN clock counter and stores the difference as `clock/offset` in NVS using Arduino `Preferences`.

On every boot:

1. The firmware reads the saved offset from NVS.
2. CAN frame `0x552` supplies the raw clock counter.
3. The display shows `(counter + saved_offset) mod 86400`.

USB serial is only needed when changing the stored offset.

The first boot default offset is `-42690` seconds, matching the older hardcoded adjustment:

```cpp
-43200 + 510
```

Once the serial command is used, the saved NVS value replaces that default.

The clock command refuses to save until at least one `0x552` clock frame has been received. This prevents saving a bogus offset before the counter is available.

## CAN Bus Signals

Only standard 11-bit CAN frames are decoded. Extended and RTR frames are ignored.

| CAN ID | Direction | DLC used | Purpose | Decode |
| --- | --- | ---: | --- | --- |
| `0x488` | RX | `>= 1` | Coolant temperature | `coolant_c = D0 - 40` |
| `0x552` | RX | `>= 3` | Clock counter | `raw = (D0 << 12) \| (D1 << 4) \| (D2 >> 4)`, then `raw % 86400` |
| `0x517` | RX | `>= 3` | Light state / dimming | lights on when `D0 D1 D2 = EB 7A 80`; lights off when `69 5A 80` |
| `0x612` | RX | `>= 5` | Spoiler position | `D4` is mapped to display percent |
| `0x7DF` | TX | `8` | OBD-II MAP request | sends `02 01 0B 00 00 00 00 00` every 250 ms |
| `0x7E8` | RX | `>= 4` | OBD-II MAP response | accepts `03 41 0B XX`; manifold pressure kPa is `D3` |

### Boost/Vacuum

Manifold absolute pressure comes from OBD-II PID `0x0B`:

```cpp
manifoldPressureKpa = D3
boostPressureKpa = manifoldPressureKpa - 101.0
```

The display converts positive pressure to PSI:

```cpp
boostPsi = boostPressureKpa * 0.1450377
```

Negative pressure is shown as vacuum in inHg:

```cpp
vacuumInHg = -boostPressureKpa * 0.295300
```

### Spoiler Position

Spoiler display percent is derived from byte `D4` in frame `0x612`:

| Raw value | Display |
| --- | ---: |
| `<= 0x10` | `0%` |
| `<= 0x20` | `30%` |
| `<= 0x40` | `80%` |
| `>= 0x50` | `100%` |
| other values | `100%` |

### Known But Disabled Signals

These were useful during reverse engineering, but are not currently shown:

| CAN ID | Purpose | Decode |
| --- | --- | --- |
| `0x208` | RPM | `rpm = ((D0 << 8) \| D1) * 0.125` |

## Display Behavior

The UI starts with a short boot screen, then switches to the main dash after board/display initialization. CAN is started after the display startup screen has been created.


## Build

Install PlatformIO, then build from this directory:

```sh
pio run
```

Flash with:

```sh
pio run -t upload
```

Serial monitor:

```sh
pio device monitor
```

The serial monitor is configured for `1000000` baud and uses the ESP32 exception decoder filter.

## Useful Files

- `src/main.cpp` - firmware entry point, CAN decoding, LVGL UI, USB serial clock setup
- `src/lvgl_v8_port.cpp` - LVGL/display port glue
- `include/lv_conf.h` - LVGL configuration used by the PlatformIO build
- `conf/esp_panel_board_custom_conf.h` - Waveshare/display panel configuration
- `platformio.ini` - board, dependencies, build flags, and monitor settings
