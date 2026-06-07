# ESP32-S3-Touch-LCD-1.85B 项目指南

> 本文件面向 AI 编码助手，用于快速理解项目结构、构建流程与开发规范。

---

## 项目概述

本项目基于 **ESP32-S3-Touch-LCD-1.85B** 开发板（厂商：Waveshare），是一个硬件嵌入式项目，主要使用 **Arduino 框架** 开发，同时包含少量 **ESP-IDF (V5.5.3)** 示例。

项目核心产物是位于 `myprojects/vibemate/` 的 **VibeMate** 应用：
- 一款桌面宠物 + Kimi Coding Plan 使用量监控器
- 4 页 lv_tileview 界面（Pet Detail / Pet / Usage / Device）
- 支持 WiFi 联网、Kimi API 数据拉取、实时电池/RTC 读取、触摸交互

---

## 硬件平台

| 组件 | 型号/规格 |
|------|----------|
| 主控 | ESP32-S3（Xtensa LX7 双核 240MHz，Wi-Fi 4 + BLE 5.0） |
| Flash | **16MB** |
| PSRAM | **8MB Octal PSRAM** |
| 屏幕 | ST77916，1.85 英寸，360×360 分辨率，16bit RGB565，QSPI 接口 |
| 触摸 | CST816，I2C 地址 `0x15`，单点电容触控 |
| 电量计 | BQ27220，I2C 地址 `0x55` |
| RTC | PCF85063，I2C 地址 `0x51` |
| 六轴 IMU | QMI8658，I2C 地址 `0x6A`/`0x6B` |
| 音频 DAC | ES8311（I2C + I2S） |
| 音频 ADC | ES7210（I2C + I2S，地址 `0x40`） |
| SD 卡 | SD_MMC 4-bit 接口 |
| I2C 总线 | SDA=GPIO 11，SCL=GPIO 10，400kHz，多设备共享 |

### 关键引脚速查

| 功能 | 引脚 |
|------|------|
| 屏幕 QSPI | SCK=40, CS=21, DATA0~3=46/45/42/41, TE=18, RST=3 |
| 背光 PWM | GPIO 5（20kHz，10bit） |
| 触摸 I2C | SDA=11, SCL=10, INT=4, RST=1 |
| I2S 音频 | MCK=2, BCK=48, WS=38, DOUT=47, DIN=39 |
| 功放使能 | GPIO 9（高电平有效） |
| SD_MMC | CLK=15, CMD=14, D0=16, D1=17, D2=12, D3=13 |

---

## 技术栈

- **开发框架**：Arduino（ESP32 Arduino Core 3.3.8）
- **构建工具**：`arduino-cli` 或 Arduino IDE
- **图形库**：LVGL v8.4.0
- **网络**：Arduino `WiFi` + `HTTPClient`
- **JSON 解析**：ArduinoJson
- **RTOS**：FreeRTOS（ESP32 内置）
- **版本控制**：Git

---

## 项目结构

```
├── AGENTS.md                 # 本文件（面向 AI 助手的项目指南）
├── README.md                 # 项目简介
├── CLAUDE.md                 # 关键构建配置与上传命令速查
├── HARDWARE_SPEC.md          # 完整硬件规格与引脚定义
├── BUILD_LOG.md              # Arduino 示例编译记录与问题调研
├── EXAMPLE_ANALYSIS.md       # 8 个厂商示例的代码分析报告
├── FIXES.md                  # 已知编译问题及修复方案
├── LICENSE                   # Apache License 2.0
│
├── Examples/
│   ├── Arduino-V3.2.0/       # 厂商 Arduino 示例（8 个）
│   │   ├── examples/
│   │   │   ├── 01_lvgl_demo/         # LVGL + 屏幕 + 触摸
│   │   │   ├── 02_lvgl_BQ27220/      # 电池状态监控界面
│   │   │   ├── 03_audio_out_no_tf/   # 内置音频播放
│   │   │   ├── 04_SDMMC_Test/        # SD 卡文件系统测试
│   │   │   ├── 05_audio_out_tf/      # SD 卡 MP3 播放
│   │   │   ├── 06_esp_sr/            # 离线语音识别
│   │   │   ├── 07_I2C_qmi8658/       # 六轴 IMU 读取
│   │   │   └── 08_I2C_pcf85063/      # RTC 实时时钟
│   │   └── libraries/        # 厂商附带库（LVGL、BQ27220、SensorLib 等）
│   └── ESP-IDF-V5.5.3/       # 厂商 ESP-IDF 综合示例（含 BSP 组件）
│
├── Firmware/
│   └── ESP32-S3-Touch-LCD-1.85B-Factory.bin   # 出厂固件
│
├── hardware/
│   └── ESP32-S3-Touch-LCD-1.85B Rev1.1.pdf   # 硬件原理图
│
├── myprojects/
│   └── vibemate/             # ⭐ 主项目：VibeMate 应用
│       ├── vibemate.ino      # 主程序入口（setup/loop）
│       ├── config.h          # WiFi / API Key / 刷新间隔等配置
│       ├── config.h.example  # 配置模板
│       ├── Display_ST77916.* # ST77916 LCD 驱动（QSPI 初始化 + 背光 PWM）
│       ├── Touch_CST816.*    # CST816 触摸驱动（I2C + 手势识别）
│       ├── LVGL_Driver.*     # LVGL 显示/输入设备注册、双缓冲（PSRAM）
│       ├── I2C_Driver.*      # I2C 总线初始化
│       ├── esp_lcd_st77916.* # 底层 ESP-IDF LCD 驱动封装
│       ├── network_manager.* # WiFi 连接管理与 NTP 对时
│       ├── kimi_api.*        # Kimi Coding Plan API 拉取与解析
│       ├── ui_pet.*          # 桌面宠物页（ASCII 动画 + 触摸交互）
│       ├── ui_pet_detail.*   # 宠物详情页
│       ├── ui_usage.*        # Coding Plan 使用量展示页
│       ├── ui_device.*       # 设备状态页（电池、WiFi、RTC）
│       ├── pet_sprites.*     # 宠物精灵图与动画帧
│       ├── rtc_bsp.*         # PCF85063 RTC 封装
│       ├── font_cjk_14.c     # 自定义 CJK 字体（14px）
│       └── font_pet_20.c     # 自定义宠物图标字体（20px）
│
└── docs/
    └── superpowers/            # 设计文档与开发计划
        ├── specs/              # 功能规格说明书
        └── plans/              # 迭代计划
```

---

## 构建与上传

### 环境要求

- **arduino-cli** ≥ 1.5.0（推荐）或 Arduino IDE
- **ESP32 Arduino Core** 3.3.8
- 已安装库清单（位于 `~/Documents/Arduino/libraries/`）：
  - `lvgl` (v8.4.0)
  - `ArduinoJson`
  - `ESP32-audioI2S-master`
  - `es8311`
  - `es7210`
  - `ESP32MQTTClient`
  - `kode_bq27220`
  - `SensorLib`

### 关键：FQBN 必须包含的选项

编译和上传时，FQBN **必须**包含以下选项，缺一不可：

```
FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi
```

- `FlashSize=16M` + `PartitionScheme=app3M_fat9M_16MB`：16MB Flash 分区方案。缺少 `FlashSize=16M` 会导致 boot loop（`partition 3 invalid`）。
- `PSRAM=opi`：启用 8MB Octal PSRAM。缺少此项会导致 LVGL 双缓冲无法分配在 PSRAM 中，进而黑屏或崩溃。

Arduino IDE 对应设置：
- Tools → Flash Size → "16MB (128Mb)"
- Tools → Partition Scheme → "16MB Flash (3MB APP/9.9MB FATFS)"
- Tools → PSRAM → "OPI PSRAM"

### 编译命令

```bash
cd myprojects/vibemate
arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
```

### 上传命令

```bash
cd myprojects/vibemate
arduino-cli upload --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" -p /dev/cu.usbmodem101 .
```

> 串口设备名（`-p` 参数）在不同系统上不同，常见值：`/dev/cu.usbmodem101`（macOS）、`/dev/ttyACM0`（Linux）、`COM3`（Windows）。

---

## 代码组织与模块划分

### VibeMate 主项目（`myprojects/vibemate/`）

| 文件 | 职责 |
|------|------|
| `vibemate.ino` | 主入口：初始化硬件 → 创建 lv_tileview 四页 → 启动定时器（API 刷新 / 设备状态 / UI 更新） |
| `config.h` | 编译期配置：WiFi 凭证、Kimi API Key、刷新间隔、背光亮度。支持通过编译宏注入覆盖默认值 |
| `Display_ST77916.*` | LCD 硬件抽象：QSPI 初始化、ST77916 寄存器配置、背光 PWM 控制 |
| `Touch_CST816.*` | 触摸硬件抽象：I2C 读取触摸坐标、手势识别（上/下/左/滑/单击/双击/长按） |
| `LVGL_Driver.*` | LVGL 集成：注册显示驱动（全屏刷新 + PSRAM 双缓冲）、注册输入设备驱动、定时器节拍 |
| `I2C_Driver.*` | I2C 总线初始化（SDA=11, SCL=10, 400kHz） |
| `network_manager.*` | WiFi 连接（含断线重连）、NTP 对时并写入 RTC |
| `kimi_api.*` | HTTP GET `api.kimi.com/coding/v1/usages`，解析周限额与 5 小时窗口数据 |
| `ui_pet.*` / `ui_pet_detail.*` | 桌宠界面：ASCII 艺术精灵、空闲动画、触摸反馈 |
| `ui_usage.*` | Kimi 使用量展示：进度条、剩余额度、重置时间 |
| `ui_device.*` | 设备状态：电池 SOC/电压/电流/温度、WiFi RSSI、IP、RTC 时间 |
| `pet_sprites.*` | 宠物精灵图数据与动画帧管理 |
| `rtc_bsp.*` | PCF85063 RTC 读写封装 |
| `font_*.c` | LVGL 自定义字体（CJK 14px、图标 20px） |

### 共享资源与并发

- **I2C 总线**被 BQ27220、PCF85063、CST816 共享，通过 `wire_mutex`（FreeRTOS 互斥信号量）保护。
- **LVGL** 运行在单线程模式，所有 UI 更新通过 `lv_timer` 回调或主 `loop()` 完成。
- **网络任务**在后台运行，`network_check()` 在 `loop()` 中周期性调用以处理断线重连。

---

## 开发规范

### 代码风格

- 语言标准：C++（Arduino）
- 缩进：4 空格
- 函数命名：
  - 底层驱动/硬件抽象：`PascalCase`（如 `LCD_Init`, `Lvgl_Init`, `Backlight_Init`）
  - 业务模块：`snake_case`（如 `kimi_api_refresh_now`, `ui_device_update`）
  - LVGL 事件/定时器回调：`static` + `snake_case`
- 全局共享对象使用 `g_` 前缀（如 `g_bq27220`, `g_kimi_data`, `g_ui_needs_update`）
- 头文件宏守卫使用全大写 + 下划线（如 `CONFIG_H`, `NETWORK_MANAGER_H`）

### 配置管理

- `config.h` 中的敏感信息（WiFi 密码、API Key）使用 `#ifndef` 包裹，允许通过编译宏注入：
  ```bash
  arduino-cli compile --build-property "build.extra_flags=-DWIFI_SSID=\"MySSID\" -DWIFI_PASSWORD=\"MyPass\"" ...
  ```
- `.gitignore` 已排除 `myprojects/vibemate/config.h`，避免敏感信息入仓。
- 仓库中保留 `config.h.example` 作为模板。

### 内存管理

- LVGL 双缓冲分配在 **PSRAM** 中：
  ```cpp
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
  lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
  ```
- 避免在栈上分配大数组；优先使用 PSRAM 或全局静态存储。

---

## 测试策略

本项目为嵌入式硬件项目，无自动化单元测试框架。验证方式如下：

1. **编译验证**：
   - 修改后必须在目标 FQBN 下编译通过。
   - 固件大小应小于 3MB（`app3M_fat9M_16MB` 分区的 app 上限）。

2. **硬件在环测试**：
   - 上传后在串口监视器（115200 baud）观察启动日志。
   - 关键初始化节点均打印 `[INIT] Xxx...` 日志。
   - 检查 LVGL 是否正常渲染（无黑屏、无撕裂）。

3. **厂商示例作为参考**：
   - `Examples/Arduino-V3.2.0/examples/` 中的 8 个示例是各外设的最小可用演示。
   - 当新增外设功能时，优先复用示例中的驱动逻辑。

---

## 已知问题与修复（重要）

### 1. LVGL demos 头文件找不到

**影响**：`01_lvgl_demo`、`02_lvgl_BQ27220`
**原因**：Arduino 编译器不会自动包含库根目录下的子目录。LVGL 的 `demos/` 和 `examples/` 不在 `src/` 内。
**修复**：将 `lvgl/demos` 和 `lvgl/examples` 复制到 `lvgl/src/` 下：
```bash
cp -r ~/Documents/Arduino/libraries/lvgl/demos ~/Documents/Arduino/libraries/lvgl/src/demos
cp -r ~/Documents/Arduino/libraries/lvgl/examples ~/Documents/Arduino/libraries/lvgl/src/examples
```

### 2. 固件体积超限

**影响**：音频/语音识别示例（`05_audio_out_tf`、`06_esp_sr`）
**原因**：默认 `default` 分区表仅分配 1.3MB app 空间。
**修复**：FQBN 中使用 `PartitionScheme=huge_app`（3MB app）。VibeMate 主项目已使用 `app3M_fat9M_16MB`，无需额外处理。

### 3. ESP_SR `sr_cmd_t` 结构体不兼容

**影响**：`06_esp_sr`
**原因**：ESP32 Arduino Core 3.3.8 中 `sr_cmd_t` 从 3 个字段（含 `phonetic`）变为 2 个字段。
**修复**：删除初始化器中的第三个字段（`phonetic` 字符串）。

> 详细记录见 `FIXES.md`。

---

## 安全注意事项

1. **API Key 与 WiFi 凭证**
   - `config.h` 包含硬编码的 WiFi SSID/密码和 Kimi API Key。
   - 该文件已被 `.gitignore` 排除，但历史提交中可能已泄露。
   - 建议定期轮换 API Key，并通过编译宏注入凭证。

2. **HTTP 明文风险**
   - `kimi_api.cpp` 使用 `HTTPClient` 通过 **HTTPS** 访问 `api.kimi.com`，传输层已加密。
   - 不要降级为 HTTP。

3. **串口日志**
   - 调试日志通过 `Serial` 输出，包含 WiFi IP、API 错误详情等。生产环境可考虑关闭或混淆敏感字段。

---

## 依赖库版本兼容性

| 库 | 版本/备注 | 兼容性注意 |
|---|----------|-----------|
| ESP32 Arduino Core | 3.3.8 | 2.x API 大量不兼容（LEDC、I2S、WiFiEvent 等） |
| LVGL | 8.4.0 | Arduino 下需手动复制 demos/examples 到 src/ |
| ArduinoJson | 最新版 | 用于解析 Kimi API 响应 |
| BQ27220 (kode_bq27220) | 厂商附带 | 电量计专用 |
| SensorLib | 厂商附带 | QMI8658、PCF85063、CST816 等 |

> 厂商示例代码部分已适配 Core 3.x（如 LEDC 使用 `ledcAttach()`），但 `06_esp_sr` 的 `sr_cmd_t` 仍需手动修复。

---

## 文档索引

| 文件 | 内容 |
|------|------|
| `HARDWARE_SPEC.md` | 完整硬件规格、引脚定义、外设参数、I2C 地址表 |
| `CLAUDE.md` | 构建配置速查、FQBN 参数、上传命令 |
| `EXAMPLE_ANALYSIS.md` | 8 个厂商示例的逐一代码分析 |
| `BUILD_LOG.md` | 示例编译记录、问题调研、已安装库清单 |
| `FIXES.md` | 编译问题修复记录与验证结果 |
| `docs/superpowers/specs/` | VibeMate 功能规格说明书 |
| `docs/superpowers/plans/` | 迭代开发计划 |

---

*本文档基于项目实际内容生成，最后更新：2026-06-06*
