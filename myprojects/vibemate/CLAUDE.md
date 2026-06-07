# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Hardware

- ESP32-S3, 360x360 round LCD (ST77916), CST816 touch
- **Flash: 16MB**, PSRAM: 8MB
- I2C bus shared: BQ27220 (battery), PCF85063 (RTC), CST816 (touch)

## Build and Upload

This is an Arduino project for ESP32-S3. Use `arduino-cli` from this directory:

```bash
# Compile
arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc" .

# Upload
arduino-cli upload --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc" -p /dev/cu.usbmodem101 .
```

### Critical FQBN Flags

All four flags are required. Missing any causes hard-to-diagnose failures:

| Flag | What happens if missing |
|------|------------------------|
| `FlashSize=16M` | Boot loop: `partition 3 invalid - offset 0x310000 size 0x300000 exceeds flash chip size 0x400000` |
| `PartitionScheme=app3M_fat9M_16MB` | Boot loop (needs the 16MB partition map) |
| `PSRAM=opi` | Black screen or crash — LVGL double-buffer is allocated in PSRAM; without Octal PSRAM it is unavailable |
| `CDCOnBoot=cdc` | Zero serial output, including panic logs and trace macros — the board's USB port connects to the ESP32-S3 built-in USB-Serial/JTAG peripheral, but Arduino core 3.x maps `Serial` to UART0 by default |

**Arduino IDE equivalents:** Tools → Flash Size → "16MB (128Mb)", PSRAM → "OPI PSRAM".

## Project Architecture

VibeMate is a 5-page LVGL application running on a 360x360 round LCD. The main entry point is `vibemate.ino`, which constructs a horizontal `lv_tileview` with five tiles:

| Index | Tile | File | Purpose |
|-------|------|------|---------|
| 0 | Pet Select | `ui_pet_select.cpp` | Choose/generate a new pet |
| 1 | Pet Detail | `ui_pet_detail.cpp` | Show pet stats and attributes |
| 2 | Pet (main) | `ui_pet.cpp` | Interactive ASCII-art pet with idle animation, touch interactions, feeding |
| 3 | Usage | `ui_usage.cpp` | Display Kimi Coding API usage (quota, window limits) |
| 4 | Device | `ui_device.cpp` | Battery (BQ27220), WiFi, RTC info |

### UI Module Pattern

Each screen is implemented as a pair of `ui_*.cpp/h` files following the same convention:

- `ui_*_create(lv_obj_t *parent_tile)` — builds the page content on the given tile once at startup.
- `ui_*_update(...)` — called from timers to refresh dynamic data.
- `ui_pet_*` additionally has `pause_anim`, `resume_anim`, `enable/disable_interaction` because the Pet page runs idle animations and touch handlers that must be suppressed during tileview scroll.

### Pet System

- **`pet_sprites.cpp/h`** — Defines 18 species (`DUCK` through `CHONK`), each with a 3-frame ASCII template, plus eyes, hats, colors, rarity tiers, and stat labels. The global `g_pet` (`PetData`) holds the current pet state. `pet_generate()` creates a randomized pet with weighted rarity.
- **`pet_storage.cpp/h`** — Persists `g_pet` to ESP32 NVS under namespace `"buddy"` using Arduino `Preferences`. Loads at boot; if none exists, generates a new one automatically.
- **Decay loop** — `decay_timer_cb` in `vibemate.ino` runs every 60s, decrementing `hunger` and `joy` if > 0, then saves and updates the UI.

### Networking and API

- **`network_manager.cpp/h`** — WiFi connection with auto-reconnect, plus NTP-to-RTC sync (`network_sync_ntp_to_rtc`).
- **`kimi_api.cpp/h`** — Fetches Kimi Coding usage from `https://api.kimi.com/coding/v1/usages` via an async FreeRTOS task (`s_api_task`, 12KB stack). Results are written to the global `g_kimi_data` protected by `kimi_mutex`. `g_ui_needs_update` signals the LVGL `ui_timer_cb` to refresh the Usage page.

### Hardware Abstraction Layer

| File | Role |
|------|------|
| `Display_ST77916.cpp/h` | SPI LCD init and low-level drawing |
| `LVGL_Driver.cpp/h` | LVGL display/touch driver registration, PSRAM double-buffer, tick hook |
| `Touch_CST816.cpp/h` | Capacitive touch controller over I2C |
| `I2C_Driver.cpp/h` | Shared I2C bus wrapper (SDA=11, SCL=10) |
| `rtc_bsp.cpp/h` | PCF85063 RTC via I2C |

## Concurrency and Threading Rules

- **I2C bus** is shared by BQ27220, PCF85063, and CST816. Access is protected by `wire_mutex` (created in `vibemate.ino`).
- **Kimi API data** (`g_kimi_data`) is protected by `kimi_mutex`. The HTTP task writes it; the LVGL UI timer reads it.
- **LVGL itself is not thread-safe** and runs on the main task. All LVGL object creation and manipulation must happen on the main task (in `setup`, callbacks, or LVGL timers). Heavy work (HTTP requests) must spawn FreeRTOS tasks and only touch LVGL objects via flag variables read by LVGL timers.

## Timer Lifecycle

All LVGL timers are created once in `setup()` and never deleted. Past crashes were caused by double-freeing timers. If adding or removing timers:

- Create once, keep a static pointer.
- To stop: `lv_timer_pause(timer)` or set a flag to ignore the callback.
- Do **not** call `lv_timer_del` on timers that might be referenced elsewhere or called from an event handler.

## Tileview Event Debouncing

`lv_tileview` can fire `LV_EVENT_VALUE_CHANGED` multiple times during inertial scroll bounces. The handler in `vibemate.ino` uses a static `s_last_reported_tile` pointer to ignore duplicate events for the same tile. Preserve this pattern when modifying navigation logic.

## Debug Trace System

`debug_trace.h` provides compile-time module trace macros (`TRACE_MAIN`, `TRACE_KIMI`, `TRACE_STORAGE`, etc.). Each module has a `#define DEBUG_* 1` switch; setting to `0` optimizes the macros away to zero overhead. Use these for logging instead of raw `Serial.print` to keep output consistent and toggleable.

## Configuration

`config.h.example` contains dummy credentials. Copy it to `config.h` and fill in real WiFi SSID/password and Kimi API key. `config.h` is gitignored-equivalent by convention (do not commit real credentials). The file uses `#ifndef` guards so build flags (`-DWIFI_SSID=...`) override the defaults if injected via the IDE or CLI.
