# Arduino 示例代码分析报告

> 记录时间: 2026-05-22
> 环境: macOS, arduino-cli 1.5.0, ESP32 Arduino Core 3.3.8

---

## 总览

| 示例 | 类别 | 使用外设 | 核心功能 |
|------|------|---------|---------|
| `01_lvgl_demo` | 显示+触摸 | ST77916 屏幕、CST816 触摸 | LVGL Widgets 演示 |
| `02_lvgl_BQ27220` | 显示+电源 | ST77916、CST816、BQ27220 | 电池状态监控界面 |
| `03_audio_out_no_tf` | 音频输出 | ES8311 DAC、功放 | Flash 内置音频播放 |
| `04_SDMMC_Test` | 存储 | SD 卡 (SD_MMC 4-bit) | SD 卡文件系统测试 |
| `05_audio_out_tf` | 音频输出 | ES8311 DAC、功放、SD 卡 | SD 卡 MP3 播放 |
| `06_esp_sr` | 语音交互 | ES7210 ADC、麦克风 | 离线语音识别 |
| `07_I2C_qmi8658` | 传感器 | QMI8658 六轴 IMU | 加速度+陀螺仪数据读取 |
| `08_I2C_pcf85063` | 时钟 | PCF85063 RTC | 实时时钟读写 |

---

## 01_lvgl_demo — LVGL 官方控件演示

### 使用的外设/硬件
- **屏幕**：ST77916，分辨率 360×360，16-bit 色深，通过 **QSPI** 连接
  - DATA0~DATA3: GPIO 46/45/42/41
  - SCK: GPIO 40, CS: GPIO 21
  - TE: GPIO 18, RST: GPIO 3
- **背光**：GPIO 5，PWM 控制（20kHz，10bit 分辨率）
- **触摸**：CST816 电容触控芯片，I2C 地址 `0x15`
  - SDA: GPIO 11, SCL: GPIO 10, INT: GPIO 4, RST: GPIO 1
- **总线**：QSPI 驱动屏幕 + I2C 驱动触摸

### 实现功能
- 初始化完整显示与触摸系统后，运行 **LVGL 官方 widgets demo**（`lv_demo_widgets()`）
- 在圆形屏幕上展示 LVGL 内置的各种控件（按钮、滑块、列表、图表等），支持手指触摸交互

### 核心代码逻辑
- `setup()` 依次调用：
  1. `I2C_Init()` — 初始化 I2C（为触摸做准备）
  2. `Backlight_Init()` — 初始化背光 PWM 并点亮屏幕
  3. `LCD_Init()` — 复位、读取 LCD 寄存器判断版本，加载对应初始化序列，启动 QSPI 驱动
  4. `Lvgl_Init()` — 注册 LVGL 显示驱动（全屏刷新模式）和输入设备驱动（读取 CST816 触摸坐标）
  5. `lv_demo_widgets()` — 启动官方控件演示
- `loop()` 每 5ms 调用一次 `Lvgl_Loop()`（即 `lv_timer_handler()`），驱动 LVGL 任务与刷新

### 依赖库
- `lvgl`（需启用 demos 组件）
- ESP-IDF 底层 LCD 驱动：`esp_lcd`、`driver/spi_master`、`esp_lcd_panel_io`
- Arduino `Wire` 库

---

## 02_lvgl_BQ27220 — 电池状态监控界面

### 使用的外设/硬件
- **屏幕 & 触摸**：与 01 **完全相同**（ST77916 QSPI 屏 + CST816 I2C 触摸）
- **新增传感器**：**BQ27220 电量计（Fuel Gauge）**，I2C 地址 `0x55`
  - 共享同一 I2C 总线（SDA=GPIO 11, SCL=GPIO 10, 400kHz）
  - 通过 I2C 读取电池 SOC、电压、电流、温度等参数

### 实现功能
- 创建一个**自定义电池信息监控界面**，在屏幕上实时显示：
  - SOC（剩余电量百分比）
  - 电池电压（mV）
  - 电流（mA）
  - 温度（°C）
- 同时通过串口（115200）每秒打印一次电池数据

### 核心代码逻辑
- `setup()`：
  1. `Serial.begin(115200)`
  2. 创建 `wire_mutex`（FreeRTOS 互斥信号量），用于**共享 I2C 总线**（BQ27220 与 CST816 共用）
  3. `gauge.begin(Wire, 0x55, ...)` 初始化 BQ27220；失败则阻塞
  4. 与 01 相同：依次初始化 I2C、背光、LCD、LVGL
  5. `battery_screen_create()` — 构建 LVGL 界面：白色背景 + 圆角面板，内部创建 4 个 `lv_label`
  6. 创建 LVGL 定时器 `bat_timer`（周期 1000ms），回调函数 `battery_timer_cb`
- `battery_timer_cb()`：
  - 获取 `wire_mutex`，调用 BQ27220 API 读取电量、电压、电流、温度
  - 释放信号量后，用 `lv_label_set_text_fmt()` 更新 4 个标签内容
  - 同步 `Serial.printf` 输出数据
- `loop()` 每 10ms 调用一次 `Lvgl_Loop()`

### 依赖库
- 与 01 相同的全部底层库（`lvgl`、ESP LCD 驱动、`Wire`）
- 额外依赖 **BQ27220** 库（`#include <BQ27220.h>`）
- FreeRTOS 信号量 `xSemaphoreCreateMutex`

---

## 03_audio_out_no_tf — 内置音频播放

### 使用的外设/硬件
- **音频编解码器**：ES8311 DAC（I2C 地址 `0x18`）
- **I2C 引脚**：SDA = GPIO 11, SCL = GPIO 10
- **I2S 引脚**：MCLK = GPIO 2, BCK = GPIO 48, LRCK = GPIO 38, DOUT = GPIO 47, DIN = GPIO 39
- **功放使能**：GPIO 9（输出高电平开启板载功放）
- **无 SD 卡**，音频数据直接固化在程序 Flash 中

### 实现功能
- 从**内部存储（Flash）**中循环播放一段预置的音频数据
- 音频参数：采样率 24kHz、16bit 位深、单声道

### 核心代码逻辑
- `setup()`：
  1. 初始化串口（115200）
  2. `Wire.begin()` 初始化 I2C
  3. `es8311_codec_init()` — 创建 ES8311 句柄，配置时钟（MCLK = 24kHz × 256）、分辨率 16bit，设置音量 60，关闭麦克风
  4. `setupI2S()` — 使用 `I2SClass` 初始化 I2S 总线（标准模式、单声道、左对齐槽位）
  5. GPIO 9 置高，使能功放
- `loop()`：
  - 持续调用 `i2s.write()`，将 `music.h` 中的 `audio_data[]` 数组数据推送到 I2S 总线，实现循环播放

### 依赖库
- `Arduino.h`、`ESP_I2S.h`、`Wire.h`、`esp_check.h`
- 同目录自定义驱动：`es8311.h/cpp`、`es8311_reg.h`、`music.h`

---

## 04_SDMMC_Test — SD 卡文件系统测试

### 使用的外设/硬件
- **SD 卡**：通过 `SD_MMC` 接口以 **4-bit 模式**连接
  - CLK = GPIO 15, CMD = GPIO 14
  - D0 = GPIO 16, D1 = GPIO 17, D2 = GPIO 12, D3 = GPIO 13
- **串口**：Serial 用于输出调试信息（波特率 115200）

### 实现功能
- 对 SD 卡进行完整的文件系统操作演示与性能测试：
  - 挂载 SD 卡并识别卡类型（MMC / SDSC / SDHC）
  - 列举根目录文件
  - 创建、删除文件夹
  - 新建、追加、读取、重命名、删除文件
  - 文件 IO 性能测试（连续读写速度）

### 核心代码逻辑
- `setup()`：
  1. `Serial.begin(115200)`
  2. `SD_MMC.setPins(clk, cmd, d0, d1, d2, d3)` 重新映射 4-bit SDMMC 引脚
  3. `SD_MMC.begin("/sdcard")` 挂载 SD 卡
  4. 检测并打印卡片类型与容量
  5. 依次执行：`listDir()` → `createDir()` → `removeDir()` → `writeFile()` → `appendFile()` → `readFile()` → `renameFile()` → `testFileIO()`
- `loop()`：为空，所有测试在 `setup()` 中一次性完成

### 依赖库
- `FS.h`（Arduino 文件系统抽象）
- `SD_MMC.h`（ESP32 内置 SDMMC 驱动）

---

## 05_audio_out_tf — SD 卡 MP3 播放

### 使用的外设/硬件
- **音频编解码器**：ES8311 DAC（I2C 地址 `0x18`）
- **I2C 引脚**：SDA = GPIO 11, SCL = GPIO 10
- **I2S 引脚**：MCLK = GPIO 2, BCK = GPIO 48, LRCK = GPIO 38, DOUT = GPIO 47, DIN = GPIO 39
- **功放使能**：GPIO 9（输出高电平）
- **SD 卡（TF 卡）**：通过 `SD_MMC` 接口以 **4-bit 模式**连接
  - CLK = GPIO 15, CMD = GPIO 14, D0 = GPIO 16, D1 = GPIO 17, D2 = GPIO 12, D3 = GPIO 13

### 实现功能
- 从 **SD 卡** 中读取并播放 MP3 音频文件（文件名为 `ff-16b-1c-44100hz.mp3`）
- 音频参数：采样率 16kHz，MCLK 倍数 256，音量 65

### 核心代码逻辑
- `setup()`：
  1. 初始化串口（115200）
  2. `Wire.begin()` 初始化 I2C
  3. `sdmmc_init()` — 配置 SD_MMC 引脚并挂载 SD 卡，检测卡类型和容量
  4. `es8311_codec_init()` — 初始化 ES8311（采样率 16kHz，音量 65）
  5. `audio.setPinout()` — 配置 I2S 引脚（含 MCLK）
  6. `audio.connecttoFS(SD_MMC, "ff-16b-1c-44100hz.mp3")` — 指定播放 SD 卡中的 MP3 文件
  7. GPIO 9 置高，使能功放
- `loop()`：
  - 每隔 1ms 延时后调用 `audio.loop()`，由 `Audio` 库内部处理 MP3 解码和 I2S 数据推送

### 依赖库
- `Arduino.h`、`esp_check.h`、`Wire.h`
- `FS.h`、`SD_MMC.h`（ESP32 SD 卡文件系统）
- `Audio.h`（第三方音频播放库，用于 MP3 解码与 I2S 输出管理）
- 同目录自定义驱动：`es8311.h/cpp`、`es8311_reg.h`

---

## 06_esp_sr — 离线语音识别

### 使用的外设/硬件
- **音频 ADC 编解码器**：ES7210（4 通道麦克风 ADC）
  - 通过 **I2C** 配置，设备地址 `0x40`
  - I2C 引脚：SDA = GPIO 11, SCL = GPIO 10
- **I2S 数字音频接口**（连接 ES7210 获取麦克风数据）：
  - MCK = GPIO 2, BCK = GPIO 48
  - WS (LRCK) = GPIO 38, DIN = GPIO 39, DOUT = GPIO 47
- **串口**：Serial 用于输出识别结果（波特率 115200）

### 实现功能
- 基于乐鑫 **ESP-SR** 框架的本地离线语音识别，采用 **两级检测模式**：
  1. **唤醒词检测 (WakeWord)**：检测默认唤醒词（如 "Hi Lexin"）
  2. **命令词检测 (Command)**：唤醒后识别具体指令
- 预定义命令词（支持多说法映射到同一命令 ID）：
  - 开灯：`"Turn on the light"` / `"Switch on the light"`
  - 关灯：`"Turn off the light"` / `"Switch off the light"` / `"Go dark"`
  - 开风扇：`"Start fan"`
  - 关风扇：`"Stop fan"`

### 核心代码逻辑
- `es7210_init()`：
  1. 使用 `Wire` (I2C) 创建 ES7210 设备句柄
  2. 配置编解码参数：采样率 16kHz、16-bit 位宽、I2S 标准格式、MCLK 倍频 256、MIC 偏置 2.87V、MIC 增益 30dB、ADC 音量 40dB
  3. 通过写大量寄存器完成芯片初始化
- `setup()`：
  1. 初始化串口、I2C (`Wire.begin`)
  2. 调用 `es7210_init()` 初始化音频 ADC
  3. 配置 `I2SClass` 引脚与参数：`i2s.setPins()` → `i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)`
  4. 注册事件回调 `ESP_SR.onEvent(onSrEvent)`
  5. 启动语音识别引擎：`ESP_SR.begin(i2s, sr_commands, ..., SR_CHANNELS_STEREO, SR_MODE_WAKEWORD)`
- `onSrEvent()` 回调（事件驱动，无需 `loop()` 处理）：
  - `SR_EVENT_WAKEWORD`：检测到唤醒词
  - `SR_EVENT_WAKEWORD_CHANNEL`：唤醒词通道验证通过，自动切换到 `SR_MODE_COMMAND`
  - `SR_EVENT_COMMAND`：识别到具体命令，打印 `command_id` 与对应短语
  - `SR_EVENT_TIMEOUT`：命令模式超时，自动切回 `SR_MODE_WAKEWORD`
- `loop()`：为空，识别完全由 ESP-SR 库后台运行并通过回调通知

### 依赖库
- `ESP_I2S.h`（乐鑫 Arduino I2S 驱动）
- `ESP_SR.h`（乐鑫离线语音识别库）
- `Wire.h`（Arduino I2C 驱动）
- `esp_log.h`、`esp_check.h`（ESP-IDF 日志与错误检查宏）
- 本地驱动文件：`es7210.h/cpp`、`es7210_reg.h`

---

## 07_I2C_qmi8658 — 六轴 IMU 数据读取

### 使用的外设/硬件
- **六轴 IMU**：QMI8658C（加速度 + 陀螺仪）
  - 接口：I2C（地址 `0x6A`/`0x6B`）
  - I2C 引脚：SDA = GPIO 11, SCL = GPIO 10
- **串口**：Serial 用于输出传感器数据（波特率 115200）

### 实现功能
- 初始化 QMI8658 后，以 FreeRTOS 任务方式持续读取六轴数据
- 输出：三轴加速度 (g) + 三轴陀螺仪 (dps)，每秒多次采样

### 核心代码逻辑
- `setup()`：
  1. `Serial.begin(115200)`
  2. `I2C_master_Init()` 初始化 I2C 总线
  3. 创建 FreeRTOS 任务 `qmi8658c_example`，优先级 2，栈大小 3000 bytes
- `qmi8658c_example` 任务：
  - 调用 `qmi8658_init()` 初始化传感器
  - 循环读取加速度 `acc[3]` 和陀螺仪 `gyro[3]`
  - 通过 `Serial.printf` 输出原始数据
- `loop()`：为空，所有工作由后台任务完成

### 依赖库
- `Arduino.h`、`Wire.h`
- `freertos/FreeRTOS.h`、`freertos/task.h`
- 本地驱动文件：`qmi8658c.h/cpp`、`i2c_bsp.h/cpp`

---

## 08_I2C_pcf85063 — RTC 实时时钟

### 使用的外设/硬件
- **RTC 芯片**：PCF85063
  - 接口：I2C（地址 `0x51`）
  - I2C 引脚：SDA = GPIO 11, SCL = GPIO 10
- **串口**：Serial 用于输出时间数据（波特率 115200）

### 实现功能
- 初始化 PCF85063 RTC
- 向 RTC 写入一个预设日期时间（2025 年 9 月 9 日 14:51:30）
- 每秒读取并打印当前 RTC 时间到串口

### 核心代码逻辑
- `setup()`：
  1. `Serial.begin(115200)` 开启串口
  2. `rtc_init()` 调用 `rtc.begin(Wire, SDA, SCL)` 初始化传感器；若检测失败则阻塞循环
  3. `i2c_rtc_setTime(2025, 9, 9, 14, 51, 30)` 设置初始时间
  4. 创建 FreeRTOS 任务 `i2c_rtc_loop_task`，固定绑定到 **Core 0**
- `i2c_rtc_loop_task`：
  - 调用 `rtc.getDateTime()` 获取 `RTC_DateTime` 对象
  - 通过 `Serial.printf` 按 `年/月/日 时:分:秒` 格式输出
  - `vTaskDelay(pdMS_TO_TICKS(1000))` 每秒执行一次
- `loop()`：为空，所有工作由后台任务完成

### 依赖库
- `Arduino.h`、`Wire.h`、`Serial`
- `SensorPCF85063.hpp`（PCF85063 专用库，提供 `begin` / `setDateTime` / `getDateTime` 等 API）
- `freertos/FreeRTOS.h`、`freertos/task.h`
- 本地封装文件：`rtc_bsp.h/cpp`

---

## 外设使用频率统计

| 外设/资源 | 涉及的示例 |
|-----------|-----------|
| ST77916 屏幕 (QSPI) | 01, 02 |
| CST816 触摸 (I2C) | 01, 02 |
| BQ27220 电量计 (I2C) | 02 |
| ES8311 音频 DAC (I2C+I2S) | 03, 05 |
| ES7210 音频 ADC (I2C+I2S) | 06 |
| SD 卡 (SD_MMC 4-bit) | 04, 05 |
| QMI8658 六轴 IMU (I2C) | 07 |
| PCF85063 RTC (I2C) | 08 |
| I2C 总线 (GPIO 11/10) | 02, 03, 05, 06, 07, 08 |
| I2S 音频总线 | 03, 05, 06 |

---

*本文档由多 Agent 并行代码分析生成。*
