# VibeMate 项目指南

> 本文档面向 AI 编码助手，用于快速理解 `myprojects/vibemate/` 目录下的主项目结构、构建流程与开发规范。

---

## 项目概述

**VibeMate** 是一款运行在 **ESP32-S3-Touch-LCD-1.85B** 开发板上的桌面宠物 + Kimi Coding Plan 使用量监控器。

- **5 页 lv_tileview 界面**：宠物选择 (Pet Select) → 宠物详情 (Pet Detail) → 宠物主界面 (Pet) → Kimi 用量 (Usage) → 设备状态 (Device)
- **桌面宠物系统**：基于 ASCII 艺术的精灵图，支持 18 种物种、多种眼睛/帽子/颜色组合、稀有度、 shiny 变异；具有饥饿度和心情值，会随时间衰减
- **Kimi API 集成**：自动轮询 `api.kimi.com/coding/v1/usages`，展示周限额与 5 小时窗口用量
- **硬件传感**：实时读取 BQ27220 电量计（SOC/电压/电流/温度）、PCF85063 RTC、WiFi RSSI
- **触摸交互**：单击/长按/滑动切换页面，宠物页支持喂食、对话、玩耍、长按换帽子

---

## 技术栈

| 层级 | 技术 |
|------|------|
| 开发框架 | Arduino（ESP32 Arduino Core 3.3.8） |
| 图形库 | LVGL v8.4.0 |
| RTOS | FreeRTOS（ESP32 内置） |
| 网络 | Arduino `WiFi` + `HTTPClient`（HTTPS） |
| JSON | ArduinoJson |
| 字体 | 自定义 CJK 字体（`font_cjk_14.c`、`font_mono_14.c`、`font_mono_16.c`） |

---

## 项目文件组织

| 文件 | 职责 |
|------|------|
| `vibemate.ino` | **主入口**。`setup()` 按序初始化硬件 → 创建 5 页 tileview → 加载或生成宠物 → 启动 4 个 lv_timer（API 刷新 / 设备状态 / UI 更新 / 属性衰减）。`loop()` 调用 `Lvgl_Loop()` + `network_check()` + 周期性堆内存诊断。 |
| `config.h` | 编译期配置：WiFi 凭证、Kimi API Key、刷新间隔、背光亮度。支持通过编译宏注入覆盖默认值。 **该文件已被 `.gitignore` 排除。** |
| `config.h.example` | 配置模板，供新克隆者复制使用。 |
| `debug_trace.h` | 模块级调试追踪宏。定义了 `TRACE_MAIN`/`TRACE_KIMI`/`TRACE_DETAIL`/`TRACE_SELECT`/`TRACE_INTERACT`/`TRACE_STORAGE`/`TRACE_NETWORK` 等宏，可通过 `#define DEBUG_XXX 0/1` 在编译时关闭特定模块日志，零运行时开销。 |
| `Display_ST77916.*` | LCD 硬件抽象：QSPI 初始化、ST77916 寄存器配置、背光 PWM 控制。 |
| `Touch_CST816.*` | 触摸硬件抽象：I2C 读取坐标、手势识别（上/下/左/右滑、单击/双击/长按）。通过 `wire_mutex` 保护 I2C 总线。 |
| `LVGL_Driver.*` | LVGL 集成：注册显示驱动（局部刷新 + PSRAM 双缓冲）、注册输入设备驱动、`lv_tick_inc` 定时器、`lv_timer_handler` 主循环封装。 |
| `I2C_Driver.*` | I2C 总线初始化（SDA=GPIO 11, SCL=GPIO 10）。提供裸 `I2C_Read`/`I2C_Write`，**不**加互斥锁（由调用方负责）。 |
| `network_manager.*` | WiFi 连接管理（含断线重连）、NTP 对时并写入 RTC。 |
| `kimi_api.*` | HTTP GET 拉取 Kimi Coding Plan 用量，在独立 FreeRTOS task 中执行（栈大小 12288）。通过 `kimi_mutex` 保护 `g_kimi_data`，设置 `g_ui_needs_update` 标志通知 UI 线程。 |
| `ui_pet_select.*` | 宠物选择页：物种轮盘、眼睛/颜色选择、稀有度预览、生成新宠物。 |
| `ui_pet_detail.*` | 宠物详情页：ASCII 头像、属性五维雷达图/柱状图、稀有度徽章、元信息。 |
| `ui_pet.*` | 宠物主界面：ASCII 精灵动画（3 帧呼吸/漂浮）、状态条（饥饿/心情）、气泡对话、光环装饰、触摸交互（喂食/对话/玩耍/长按换帽子）。 |
| `ui_usage.*` | Kimi 用量页：双弧进度条（周限额 + 5 小时窗口）、百分比、剩余额度、重置时间、状态指示点。 |
| `ui_device.*` | 设备状态页：电池 SOC 进度条 + 详细数值、WiFi RSSI / IP、RTC 时间。 |
| `pet_sprites.*` | 宠物精灵图数据：18 个物种各 3 帧 ASCII 模板、眼睛/帽子/颜色/稀有度/属性标签表、名字池、`pet_generate()` / `pet_reset_stats()` / RNG 工具。 |
| `pet_storage.*` | 宠物持久化：使用 ESP32 `Preferences`（NVS）读写宠物属性，namespace 为 `"buddy"`。 |
| `rtc_bsp.*` | PCF85063 RTC 读写封装，基于 `SensorPCF85063`（SensorLib）。 |
| `esp_lcd_st77916.*` | 底层 ESP-IDF LCD 驱动封装（QSPI + ST77916 专用命令序列）。 |
| `font_*.c` | LVGL 自定义字体源文件（CJK 14px、等宽 14px/16px）。 |

---

## 构建与上传

### 环境要求

- **arduino-cli** ≥ 1.5.0 或 Arduino IDE
- **ESP32 Arduino Core** 3.3.8
- 已安装库（位于 `~/Documents/Arduino/libraries/`）：
  - `lvgl` (v8.4.0) — **必须**将 `lvgl/demos` 和 `lvgl/examples` 复制到 `lvgl/src/` 下
  - `ArduinoJson`
  - `BQ27220` (`kode_bq27220`)
  - `SensorLib`（PCF85063、QMI8658、CST816 等）

### 关键 FQBN 选项

编译和上传时，FQBN **必须**包含以下选项：

```
FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc
```

| 选项 | 说明 |
|------|------|
| `FlashSize=16M` | 16MB Flash。缺少会导致 `partition 3 invalid` boot loop。 |
| `PartitionScheme=app3M_fat9M_16MB` | 3MB APP / 9.9MB FATFS 分区。 |
| `PSRAM=opi` | 启用 8MB Octal PSRAM。缺少会导致 LVGL 双缓冲无法分配，黑屏或崩溃。 |
| `CDCOnBoot=cdc` | 将 `Serial` 映射到 USB-Serial/JTAG 外设。缺少会导致串口无任何输出。 |

### 命令

```bash
cd myprojects/vibemate

# 编译
arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc" .

# 上传（串口设备名因系统而异）
arduino-cli upload --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc" -p /dev/cu.usbmodem101 .
```

---

## 运行时架构

### 任务与定时器

| 任务/定时器 | 周期 | 职责 |
|-------------|------|------|
| `api_timer` | 45 秒 | 触发 Kimi API 刷新（在独立 FreeRTOS task 中执行 HTTP） |
| `device_timer` | 1 秒 | 更新设备状态页 + 宠物页状态条 |
| `ui_timer` | 500 毫秒 | 检查 `g_ui_needs_update`，将 `g_kimi_data` 同步到用量页 UI |
| `decay_timer` | 60 秒 | 宠物饥饿度/心情值自然衰减，触发保存 |
| `loop()` | 10 毫秒 delay | 调用 `Lvgl_Loop()`（即 `lv_timer_handler`）+ `network_check()` + 堆内存诊断 |

### 并发与同步

- **I2C 总线共享**：BQ27220、PCF85063、CST816 共享同一 I2C 总线。`wire_mutex`（FreeRTOS 互斥量）保护所有 I2C 访问。`Touch_CST816.cpp` 使用 `xSemaphoreTake(wire_mutex, ...)` 封装读写。
- **Kimi 数据同步**：`kimi_api.cpp` 在后台 task `s_api_task` 中执行 HTTP，结果写入 `g_kimi_data` 前获取 `kimi_mutex`；`vibemate.ino` 的 `ui_timer_cb` 读取前同样获取该互斥量。
- **LVGL 线程模型**：单线程模式。所有 UI 创建/更新均在 `setup()` 或 `lv_timer` 回调中完成，不跨 task 操作 LVGL 对象。

### 内存布局

- LVGL 双缓冲分配在 **PSRAM** 中：
  ```cpp
  buf1 = (lv_color_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
  buf2 = (lv_color_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
  ```
- 避免在栈上分配大数组；优先使用 PSRAM 或全局静态存储。

---

## 代码风格规范

- **语言标准**：C++（Arduino）
- **缩进**：4 空格
- **函数命名**：
  - 底层驱动/硬件抽象：`PascalCase`（如 `LCD_Init`, `Lvgl_Init`, `Backlight_Init`）
  - 业务模块：`snake_case`（如 `kimi_api_refresh_now`, `ui_device_update`）
  - LVGL 事件/定时器回调：`static` + `snake_case`
- **全局共享对象**：使用 `g_` 前缀（如 `g_bq27220`, `g_kimi_data`, `g_pet`, `g_ui_needs_update`）
- **头文件宏守卫**：全大写 + 下划线（如 `CONFIG_H`, `NETWORK_MANAGER_H`）
- **注释**：以中文为主，关键逻辑可辅以英文。

---

## 配置管理

`config.h` 中的敏感信息使用 `#ifndef` 包裹，允许通过编译宏注入：

```bash
arduino-cli compile --build-property "build.extra_flags=-DWIFI_SSID=\"MySSID\" -DWIFI_PASSWORD=\"MyPass\" -DKIMI_API_KEY=\"sk-xxx\"" ...
```

`.gitignore` 已排除 `myprojects/vibemate/config.h`，避免敏感信息入仓。仓库中保留 `config.h.example` 作为模板。

---

## 调试策略

### 串口日志

- 波特率：**115200**
- 所有关键初始化节点均打印 `[INIT] Xxx...` 日志（通过 `TRACE_MAIN`）。
- `debug_trace.h` 提供按模块开关的追踪宏：
  - `TRACE_MAIN` / `TRACE_MAIN_ENTER` / `TRACE_MAIN_EXIT` / `TRACE_MAIN_HEAP`
  - `TRACE_KIMI`、`TRACE_NETWORK`、`TRACE_DETAIL`、`TRACE_SELECT`、`TRACE_INTERACT`、`TRACE_STORAGE`
  - 将对应模块的 `#define DEBUG_XXX 1` 改为 `0` 即可关闭，编译器会优化掉，零开销。

### 硬件在环测试

1. **编译验证**：修改后必须在目标 FQBN 下编译通过。固件大小应小于 3MB。
2. **上传观察**：上传后在串口监视器观察启动日志，确认：
   - `[INIT] I2C...` → `[INIT] LCD...` → `[INIT] LVGL...` → `[INIT] BQ27220...` → `[INIT] Network...` → `[INIT] Kimi API...` → `VibeMate ready!`
   - 无 `partition 3 invalid`（FlashSize 错误）
   - 无黑屏（PSRAM 错误）
   - 无串口静默（CDCOnBoot 缺失）
3. **功能验证**：滑动切换 5 页，检查宠物动画、Kimi 数据刷新、电池/RTC 更新是否正常。

> 本项目为嵌入式硬件项目，无自动化单元测试框架。

---

## 安全注意事项

1. **API Key 与 WiFi 凭证**
   - `config.h` 包含硬编码的 WiFi SSID/密码和 Kimi API Key。
   - 该文件已被 `.gitignore` 排除，但历史提交中可能已泄露。
   - 建议定期轮换 API Key，并通过编译宏注入凭证。

2. **HTTP 传输**
   - `kimi_api.cpp` 使用 `HTTPClient` 通过 **HTTPS** 访问 `api.kimi.com`，传输层已加密。不要降级为 HTTP。

3. **串口日志**
   - 调试日志通过 `Serial` 输出，包含 WiFi IP、API 错误详情等。生产环境可考虑关闭敏感字段日志。

---

## 依赖库版本兼容性

| 库 | 版本/备注 | 兼容性注意 |
|---|----------|-----------|
| ESP32 Arduino Core | 3.3.8 | 2.x API 不兼容（LEDC、I2S、WiFiEvent 等） |
| LVGL | 8.4.0 | Arduino 下需手动复制 demos/examples 到 src/ |
| ArduinoJson | 最新版 | 用于解析 Kimi API 响应 |
| BQ27220 (kode_bq27220) | 厂商附带 | 电量计专用 |
| SensorLib | 厂商附带 | PCF85063、CST816、QMI8658 等 |

---

## 常见问题速查

| 现象 | 根因 | 修复 |
|------|------|------|
| 编译提示 `demos/lv_demos.h` 找不到 | LVGL demos 目录不在 `src/` 内 | `cp -r ~/Documents/Arduino/libraries/lvgl/demos ~/Documents/Arduino/libraries/lvgl/src/demos` |
| 上电后 `partition 3 invalid` boot loop | FQBN 缺少 `FlashSize=16M` | 补全 FQBN：`FlashSize=16M,PartitionScheme=app3M_fat9M_16MB` |
| 上电黑屏/崩溃 | FQBN 缺少 `PSRAM=opi` | 补全 FQBN：`PSRAM=opi` |
| 串口无任何输出 | FQBN 缺少 `CDCOnBoot=cdc` | 补全 FQBN：`CDCOnBoot=cdc` |
| I2C 读取偶尔超时/乱码 | 多设备竞争 I2C 总线 | 确保所有 I2C 访问通过 `wire_mutex` 保护 |

---

## 相关文档

- 上级目录 `AGENTS.md`：整个仓库（含厂商示例、硬件规格、构建指南）的完整说明
- `../../CLAUDE.md`：构建配置速查、FQBN 参数、上传命令
- `../../HARDWARE_SPEC.md`：完整硬件规格、引脚定义、外设参数、I2C 地址表
- `../../FIXES.md`：已知编译问题及修复方案

---

*本文档基于项目实际内容生成，最后更新：2026-06-07*
