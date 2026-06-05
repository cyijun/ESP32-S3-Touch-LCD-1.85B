# ESP32-S3-Touch-LCD-1.85B Project Notes

## Hardware

- ESP32-S3, 360x360 round LCD (ST77916), CST816 touch
- **Flash: 16MB**, PSRAM: 8MB
- I2C bus shared: BQ27220 (battery), PCF85063 (RTC), CST816 (touch)

## Critical Build Configuration

**When compiling/uploading, the FQBN MUST include both:**

```
FlashSize=16M,PartitionScheme=app3M_fat9M_16MB
```

Only setting `PartitionScheme=app3M_fat9M_16MB` without `FlashSize=16M` causes boot loop:
```
partition 3 invalid - offset 0x310000 size 0x300000 exceeds flash chip size 0x400000
```

Arduino IDE: Tools -> Flash Size -> "16MB (128Mb)"

## Project: VibeMate

Location: `myprojects/vibemate/`

- Kimi Coding Plan usage display on hardware
- 3-page lv_tileview (Usage / Device / Pet)
- Auto-refresh API + manual click refresh
- BQ27220 battery + PCF85063 RTC real-time reading

## Upload Commands

```bash
# Compile
arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB" .

# Upload
arduino-cli upload --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB" -p /dev/cu.usbmodem101 .
```
