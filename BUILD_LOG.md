# Arduino 示例编译记录

> 记录时间: 2026-05-21
> 环境: macOS, arduino-cli 1.5.0, ESP32 Arduino Core 3.3.8
> 板子: `esp32:esp32:esp32s3`

---

## 编译结果汇总

| 示例 | 状态 | 固件大小 | 结果 / 报错 |
|------|------|----------|------------|
| `01_lvgl_demo` | ❌ 失败 | — | `fatal error: demos/lv_demos.h: No such file or directory` |
| `02_lvgl_BQ27220` | ❌ 失败 | — | `fatal error: demos/lv_demos.h: No such file or directory` |
| `03_audio_out_no_tf` | ✅ 成功 | 1,063,480 bytes (81%) | — |
| `04_SDMMC_Test` | ✅ 成功 | 368,047 bytes (28%) | — |
| `05_audio_out_tf` | ❌ 失败 | 1,895,673 bytes (144%) | `Sketch too big; text section exceeds available space` |
| `06_esp_sr` | ❌ 失败 | — | `too many initializers for 'const sr_cmd_t'` |
| `07_I2C_qmi8658` | ✅ 成功 | 299,635 bytes (22%) | — |
| `08_I2C_pcf85063` | ✅ 成功 | 343,471 bytes (26%) | — |

---

## 问题调研

### 1. LVGL demos 头文件路径问题（01、02）

**错误信息：**
```
fatal error: demos/lv_demos.h: No such file or directory
    5 | #include <demos/lv_demos.h>
```

**原因：**
LVGL 在 Arduino 构建系统下有一个已知限制——`demos/` 和 `examples/` 目录位于库根目录，但 Arduino 编译器默认不会把根目录的子目录加入 include path。

**社区方案：**
> 需要手动把 `lvgl/demos` 复制到 `lvgl/src/demos`，同样 `lvgl/examples` 也要复制到 `lvgl/src/examples`。
>
> 参考: [LVGL Arduino 集成文档](https://docs.lvgl.io/master/integration/framework/arduino.html)

```
Arduino/libraries/lvgl/
├── src/
│   ├── core/
│   ├── demos/          <-- 从根目录 demos/ 复制过来
│   ├── examples/       <-- 从根目录 examples/ 复制过来
│   └── ...
├── demos/              <-- 原始位置（Arduino 编译器不搜这里）
└── examples/
```

这是很多厂商示例（Waveshare、Elecrow 等）在 Arduino 下编译 LVGL demo 时的共同步骤。

---

### 2. 固件体积超限（05）

**错误信息：**
```
Sketch uses 1895673 bytes (144%) of program storage space. Maximum is 1310720 bytes.
Sketch too big; see https://support.arduino.cc/...
text section exceeds available space in board
```

**原因：**
默认分区表 `default` 只给 app 分配 **1.3MB**（1,310,720 bytes），而音频库 + SD + WiFi 叠加后接近 1.9MB。
ESP32-S3 实际有 4MB/8MB/16MB Flash，只是分区表限制。

**社区方案：**
- Arduino IDE: **Tools → Partition Scheme → "Huge APP (3MB No OTA/1MB SPIFFS)"**
- arduino-cli: 在 FQBN 里通过 board option 指定分区方案：
  ```bash
  arduino-cli compile --fqbn esp32:esp32:esp32s3:PartitionScheme=huge_app ...
  ```
- 注意：`--build-property build.partitions=xxx` 在 arduino-cli 下经常不生效，正确方式是通过 FQBN option 传递。

> 参考: [StackExchange - How to change partition scheme with arduino-cli](https://arduino.stackexchange.com/questions/93390)

---

### 3. ESP_SR `sr_cmd_t` 结构体不兼容（06）

**错误信息：**
```
error: too many initializers for 'const sr_cmd_t'
   45 | };
      | ^
```

**原因：**
ESP32 Arduino Core 3.x 中 `sr_cmd_t` 结构体的字段定义发生了变化。
旧示例代码用 `{0, "Turn on the light", "TkN nN jc LiT"}`（3 个字段）初始化 `sr_cmd_t`，
但新版 `sr_cmd_t` 可能只有 2 个字段（去掉了 `command_id` 或调整了结构）。

**社区方案：**
- GitHub `espressif/arduino-esp32` [issue #9790](https://github.com/espressif/arduino-esp32/issues/9790) 有人报告了完全相同的代码和报错。
- 需要查看新版 `ESP_SR` 库头文件里 `sr_cmd_t` 的实际定义，去掉多余的初始化字段，或者改用 `ESP_SR` 新版 API 的注册方式。

---

## 额外发现：ESP32 Arduino Core 3.x API 变更

当前安装的 Core 版本为 **3.3.8**，相比 2.x 有大量 API 变更：

| 旧 API (2.x) | 新 API (3.x) |
|-------------|-------------|
| `ledcSetup()` + `ledcAttachPin()` | `ledcAttach()` |
| `ledcWrite(channel, duty)` | `ledcWrite(pin, duty)` |
| `ledcDetachPin()` | `ledcDetach()` |
| I2S 旧驱动 | 完全重构为 `ESP_I2S` 新 API |
| `WiFiEvent_t` 事件名 | 全部改为 `ARDUINO_EVENT_WIFI_*` 前缀 |
| Timer API | 全新设计，`timerBegin(freq)` 等 |

厂商示例里部分代码（如 LEDC 的 `ledcAttach()`）看起来已经适配了 3.x，但 ESP_SR 示例还没完全同步。

---

## 已安装库清单

```
~/Documents/Arduino/libraries/
├── ArduinoJson
├── ESP32-audioI2S-master
├── es7210
├── es8311
├── ESP32MQTTClient
├── kode_bq27220
├── lvgl (v8.4.0)
└── SensorLib
```

---

*本文档由自动编译与网络调研生成。*
