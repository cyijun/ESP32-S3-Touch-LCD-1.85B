# ESP32-S3-Touch-LCD-1.85B 硬件规格说明

> 本文档基于厂商提供的 Arduino V3.2.0 示例代码与驱动整理，用于 ESP32-S3-Touch-LCD-1.85B 开发板的软件开发参考。

---

## 1. 主控

| 参数 | 规格 |
|------|------|
| 芯片型号 | **ESP32-S3** |
| CPU | Xtensa LX7 双核，最高 240MHz |
| 无线 | Wi-Fi 4 (802.11 b/g/n) + BLE 5.0 |
| PSRAM | 8MB (片外) |
| Flash | 通常 8MB 或 16MB（具体以实际模组为准） |

---

## 2. 显示屏

| 参数 | 规格 |
|------|------|
| 驱动芯片 | **ST77916** |
| 尺寸 | 1.85 英寸 |
| 分辨率 | **360 × 360** |
| 颜色深度 | 16bit (RGB565) |
| 接口 | **QSPI**（4 线 SPI） |
| 刷新方向 | 默认竖屏，可通过寄存器配置 |

### 2.1 屏幕引脚定义

| 信号名 | GPIO | 说明 |
|--------|------|------|
| `LCD_SCK` | **40** | SPI 时钟 |
| `LCD_CS` | **21** | 片选 |
| `LCD_DATA0` | **46** | QSPI D0 |
| `LCD_DATA1` | **45** | QSPI D1 |
| `LCD_DATA2` | **42** | QSPI D2 |
| `LCD_DATA3` | **41** | QSPI D3 |
| `LCD_TE` | **18** | Tearing Effect 信号 |
| `LCD_RST` | **3** | 复位 |
| `LCD_BL` | **5** | 背光控制（PWM，10bit，默认 20kHz） |

### 2.2 背光控制

- PWM 通道：任意（示例使用 `ledcAttach()`）
- 频率：20kHz
- 分辨率：10bit（范围 0~1023，示例映射为 0~100%）
- 有效范围：`0 ~ 100`

```cpp
#define LCD_Backlight_PIN   5
#define Frequency       20000
#define Resolution      10
#define Backlight_MAX   100

ledcAttach(LCD_Backlight_PIN, Frequency, Resolution);
Set_Backlight(50);  // 50% 亮度
```

---

## 3. 触摸屏

| 参数 | 规格 |
|------|------|
| 驱动芯片 | **CST816** |
| 接口 | I2C |
| I2C 地址 | **0x15** |
| 触摸点数 | 单点 |

### 3.1 触摸引脚定义

| 信号名 | GPIO | 说明 |
|--------|------|------|
| `TP_SDA` | **11** | I2C 数据 |
| `TP_SCL` | **10** | I2C 时钟 |
| `TP_INT` | **4** | 触摸中断（下降沿触发） |
| `TP_RST` | **1** | 触摸复位 |

### 3.2 支持手势

| 手势代码 | 名称 | 说明 |
|----------|------|------|
| `0x00` | NONE | 无手势 |
| `0x01` | SWIPE_UP | 上滑 |
| `0x02` | SWIPE_DOWN | 下滑 |
| `0x03` | SWIPE_LEFT | 左滑 |
| `0x04` | SWIPE_RIGHT | 右滑 |
| `0x05` | SINGLE_CLICK | 单击 |
| `0x0B` | DOUBLE_CLICK | 双击 |
| `0x0C` | LONG_PRESS | 长按 |

---

## 4. I2C 总线

板载所有 I2C 外设共享同一组总线：

| 信号 | GPIO |
|------|------|
| SDA | **11** |
| SCL | **10** |
| 频率 | 400kHz |

### 4.1 I2C 设备地址表

| 设备 | 地址 | 功能描述 |
|------|------|----------|
| CST816 | `0x15` | 电容触摸屏 |
| BQ27220 | `0x55` | 电池电量计（Fuel Gauge） |
| QMI8658 | `0x6A` / `0x6B` | 6 轴 IMU（加速度 + 陀螺仪） |
| PCF85063 | `0x51` | RTC 实时时钟 |
| ES8311 | 依地址引脚 | 音频 DAC（播放） |
| ES7210 | `0x40` | 音频 ADC（录音 / 麦克风） |

> **注意**：多个设备共享 I2C 总线，若同时访问建议使用互斥锁（`xSemaphoreCreateMutex()`）避免总线冲突。

---

## 5. 传感器

### 5.1 六轴 IMU — QMI8658

| 参数 | 规格 |
|------|------|
| 加速度计量程 | ±2g / ±4g / ±8g / ±16g |
| 陀螺仪量程 | ±16dps ~ ±2048dps |
| 输出速率 | 最高 8000Hz |
| 接口 | I2C（地址 0x6A/0x6B） |

可用功能：
- 三轴加速度 + 三轴陀螺仪数据读取
- FIFO 缓冲（可选）
- 计步器（Pedometer）
- 任意运动检测（Any Motion Detection）
- 敲击检测（Tap Detection）

### 5.2 RTC — PCF85063

| 参数 | 规格 |
|------|------|
| 接口 | I2C（地址 0x51） |
| 功能 | 年/月/日/时/分/秒/星期 |
| 闹钟 | 支持（按单位设置） |
| 时钟输出 | 支持 |

---

## 6. 电池与电源管理

### 6.1 电池电量计 — BQ27220

| 参数 | 规格 |
|------|------|
| 芯片 | TI BQ27220 |
| 接口 | I2C（地址 0x55） |
| 监测参数 | SOC%、电压、电流、温度、剩余容量、循环次数 |

可读取数据（示例库 `kode_bq27220`）：
- `readStateOfChargePercent()` — 剩余电量 %
- `readVoltageMillivolts()` — 电池电压 mV
- `readCurrentMilliamps()` — 电流 mA（正为充电，负为放电）
- `readTemperatureCelsius()` — 温度 °C
- `readRemainingCapacitymAh()` — 剩余容量 mAh
- `readCycleCount()` — 循环次数

### 6.2 供电说明

- 板载锂电池接口（带充电管理）
- 可通过 USB 直接供电/充电
- 背光、音频功放等可独立控制以降低功耗

---

## 7. 音频

### 7.1 音频播放 — ES8311 (DAC)

| 参数 | 规格 |
|------|------|
| 芯片 | ES8311 低功耗音频 DAC |
| 接口 | I2C + I2S |
| 采样率 | 支持 8kHz ~ 96kHz（示例使用 24kHz） |
| 分辨率 | 16bit |

### 7.2 音频录音 — ES7210 (ADC)

| 参数 | 规格 |
|------|------|
| 芯片 | ES7210 音频 ADC |
| I2C 地址 | `0x40` |
| 接口 | I2C + I2S |
| 采样率 | 示例使用 16kHz |
| 麦克风 | 板载麦克风输入 |

### 7.3 I2S 引脚定义

| 信号 | GPIO | 说明 |
|------|------|------|
| `I2S_MCK` | **2** | 主时钟 |
| `I2S_BCK` | **48** | 位时钟 |
| `I2S_LRCK` (WS) | **38** | 左右声道时钟 |
| `I2S_DOUT` | **47** | 数据输出（播放） |
| `I2S_DIN` | **39** | 数据输入（录音） |
| `AMP_EN` | **9** | 音频功放使能（高电平有效） |

### 7.4 音频初始化示例

```cpp
// I2S 初始化
i2s.setPins(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCK_PIN);
i2s.begin(I2S_MODE_STD, 24000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);

// 功放使能
pinMode(9, OUTPUT);
digitalWrite(9, HIGH);
```

---

## 8. SD 卡 (SD_MMC)

| 参数 | 规格 |
|------|------|
| 接口 | **SD_MMC 4-bit** |
| 文件系统 | FAT32 |

### 8.1 SD 卡引脚定义

| 信号 | GPIO |
|------|------|
| CLK | **15** |
| CMD | **14** |
| D0 | **16** |
| D1 | **17** |
| D2 | **12** |
| D3 | **13** |

### 8.2 SD 卡初始化示例

```cpp
#include "SD_MMC.h"

int clk = 15, cmd = 14, d0 = 16, d1 = 17, d2 = 12, d3 = 13;
SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);
SD_MMC.begin("/sdcard");
```

---

## 9. Arduino 示例与库

### 9.1 示例代码目录

```
Examples/Arduino-V3.2.0/examples/
├── 01_lvgl_demo/           # LVGL + 屏幕 + 触摸基础框架
├── 02_lvgl_BQ27220/        # 电量计 + LVGL 界面
├── 03_audio_out_no_tf/     # 音频播放（ES8311，内置测试音频）
├── 04_SDMMC_Test/          # SD 卡读写测试
├── 05_audio_out_tf/        # 从 SD 卡播放音频
├── 06_esp_sr/              # 语音识别（ES7210 + ESP-SR）
├── 07_I2C_qmi8658/         # QMI8658 六轴传感器读取
└── 08_I2C_pcf85063/        # PCF85063 RTC 时钟读写
```

### 9.2 自带库目录

```
Examples/Arduino-V3.2.0/libraries/
├── lvgl/                   # LVGL 图形库（含 demo）
├── kode_bq27220/           # BQ27220 电量计驱动
├── SensorLib/              # 传感器集合（QMI8658、PCF85063、CST816 等）
├── es8311/                 # ES8311 音频 DAC 驱动
├── es7210/                 # ES7210 音频 ADC 驱动
└── ESP32-audioI2S-master/  # I2S 音频播放库
```

---

## 10. 快速参考：核心引脚总览

| 功能 | 引脚列表 |
|------|----------|
| **屏幕 QSPI** | 40(SCK), 21(CS), 46/45/42/41(DATA0~3), 18(TE), 3(RST) |
| **背光** | 5 (PWM) |
| **触摸 I2C** | 11(SDA), 10(SCL), 4(INT), 1(RST) |
| **I2S 音频** | 2(MCK), 48(BCK), 38(WS), 47(DOUT), 39(DIN) |
| **功放使能** | 9 |
| **SD_MMC** | 15(CLK), 14(CMD), 16(D0), 17(D1), 12(D2), 13(D3) |

---

## 11. 项目开发建议

基于本硬件的 **桌宠 + Coding Plan 显示器** 项目，各模块可利用资源如下：

| 需求 | 可用硬件资源 |
|------|-------------|
| 圆形 GUI / 动画 | 360×360 ST77916 + LVGL |
| 触摸交互 | CST816（点击/滑动/长按/双击） |
| 时间显示 / 定时刷新 | PCF85063 RTC |
| 摇晃/姿态感应 | QMI8658 六轴 IMU |
| 音效/语音 | ES8311 DAC + ES7210 ADC |
| 存储图片/音频/数据 | SD_MMC (4-bit) |
| 电池状态显示 | BQ27220 电量计 |
| 联网获取数据 | ESP32-S3 WiFi |
| 低功耗待机 | 背光 PWM + 触摸唤醒 |

---

*文档生成时间: 2026-05-21*
