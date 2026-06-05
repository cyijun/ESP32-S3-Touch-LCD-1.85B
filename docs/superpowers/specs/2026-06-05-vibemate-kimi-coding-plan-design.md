# VibeMate — Kimi Coding Plan 硬件显示器 设计文档

> 为 ESP32-S3-Touch-LCD-1.85B 开发板编写的 Arduino 程序，通过 WiFi 获取 Kimi Coding API 用量，在 360×360 圆形屏幕上以 LVGL 图形界面展示，支持左右滑动切换多页面。

---

## 1. 项目概述

### 1.1 目标

在 ESP32-S3-Touch-LCD-1.85B 圆形屏幕上实现一个 AI 编程搭子（Kimi for Coding）的 plan 套餐用量显示器。

### 1.2 核心功能

- 通过 WiFi 请求 Kimi Coding API 获取用量数据
- 用量页：双圆弧仪表展示周限额 + 5小时窗口用量
- 设备页：电池、WiFi、RTC 时间等硬件状态
- 桌宠页：装饰性占位页面，预留动画扩展
- 左右滑动切换 3 个页面（`lv_tileview`）
- 自动定时刷新（5 分钟）+ 手动点击刷新

### 1.3 参考文件

| 文件 | 作用 |
|------|------|
| `myprojects/vibemate/kimi_usage.py` | Kimi API 请求与解析逻辑参考 |
| `myprojects/vibemate/lvgl-gui-kimi-coding-plan.html` | UI 视觉设计参考（圆形仪表） |
| `Examples/Arduino-V3.2.0/examples/02_lvgl_BQ27220/` | LVGL 初始化 + BQ27220 电池读取参考 |
| `Examples/Arduino-V3.2.0/examples/08_I2C_pcf85063/` | PCF85063 RTC 读取参考 |

---

## 2. 硬件规格

| 组件 | 型号 | 参数 |
|------|------|------|
| 主控 | ESP32-S3 | 240MHz 双核，8MB PSRAM |
| 屏幕 | ST77916 | 1.85 英寸，360×360，QSPI，圆形 |
| 触摸屏 | CST816 | I2C 0x15，支持左右滑动手势 |
| 电池计 | BQ27220 | I2C 0x55，SOC/电压/电流/温度 |
| RTC | PCF85063 | I2C 0x51，年月日时分秒 |
| 联网 | 内置 WiFi | 802.11 b/g/n |

屏幕分辨率 360×360，颜色深度 RGB565（16bit）。

---

## 3. 文件结构

```
myprojects/vibemate/
├── vibemate.ino          # 主程序：setup/loop、硬件初始化、tileview 创建
├── config.h              # WiFi + Kimi API Key 配置（用户手动编辑）
├── network_manager.cpp   # WiFi 连接、状态检测、自动重连
├── network_manager.h
├── kimi_api.cpp          # HTTP 请求、JSON 解析、数据模型
├── kimi_api.h
├── ui_usage.cpp          # 用量页：双圆弧仪表 + 数字详情
├── ui_usage.h
├── ui_device.cpp         # 设备页：电池 + WiFi + RTC 时间
├── ui_device.h
├── ui_pet.cpp            # 桌宠页：装饰动画占位
├── ui_pet.h
└── assets/               # 图片资源（如有需要）
```

---

## 4. 架构设计

### 4.1 初始化流程

```
Serial.begin(115200)
    ↓
I2C_Init()          # 初始化 I2C 总线（SDA=GPIO11, SCL=GPIO10, 400kHz）
    ↓
Backlight_Init()    # 背光 PWM 初始化（GPIO5, 20kHz, 10bit）
    ↓
LCD_Init()          # ST77916 QSPI 屏幕初始化
    ↓
Lvgl_Init()         # LVGL v8.4 初始化（双缓冲）
    ↓
WiFi.begin()        # 使用 config.h 中的凭证连接 WiFi
    ↓
等待 WiFi 连接（超时 30 秒）
    ↓
创建 lv_tileview + 3 个 tile
    ↓
分别调用 ui_usage_create() / ui_device_create() / ui_pet_create()
    ↓
创建 lv_timer：api_refresh_timer（300000ms = 5 分钟）
    ↓
创建 lv_timer：device_refresh_timer（1000ms = 1 秒）
    ↓
创建 lv_timer：ui_update_timer（500ms，检查更新标志）
```

### 4.2 主循环

```cpp
void loop() {
    Lvgl_Loop();          // lv_timer_handler()
    network_check();      // 非阻塞 WiFi 状态检测与自动重连
    vTaskDelay(pdMS_TO_TICKS(10));
}
```

---

## 5. 页面设计

使用 `lv_tileview` 作为根容器，3 个 tile 水平排列。

```cpp
lv_obj_t *tv = lv_tileview_create(lv_scr_act());
lv_obj_t *tile_usage  = lv_tileview_add_tile(tv, 0, 0, LV_DIR_HOR);
lv_obj_t *tile_device = lv_tileview_add_tile(tv, 1, 0, LV_DIR_HOR);
lv_obj_t *tile_pet    = lv_tileview_add_tile(tv, 2, 0, LV_DIR_HOR);
lv_obj_set_tile(tv, tile_usage, LV_ANIM_OFF);  // 默认显示用量页
```

`LV_DIR_HOR` 限制只允许水平滑动。

### 5.1 Tile 0 — 用量仪表页（主页面）

深色科技风背景（`#0a0e17` → LVGL `lv_color_hex(0x0a0e17)`）。

| 区域 | 元素 | 实现 |
|------|------|------|
| 顶部 | "KIMI CODING"（小字）+ 套餐名如 "Allegro" | `lv_label` ×2，居中对齐 |
| 右上角 | 状态指示点（6px 圆） | `lv_obj`，颜色根据状态变化 |
| 中心 | 双 `lv_arc` 仪表 | `lv_arc` ×2，旋转 -90° 使起点在顶部 |
| | 外圈（r=118px）：周用量 | 蓝色 `#4fc3f7` |
| | 内圈（r=94px）：5小时窗口 | 橙色 `#ffa726` |
| 中心文字 | 大百分比 + 标签 + 数值 | `lv_label` ×3，覆盖在 arc 中心 |
| 底部 | 图例：● 周配额 / ● 5h窗口 | `lv_obj` 小圆点 + `lv_label` |

**交互：** 点击中心区域触发手动 API 刷新（`LV_EVENT_CLICKED`）。

**Arc 计算：**
- `lv_arc_set_range(arc, 0, 100)` — 百分比范围
- `lv_arc_set_value(arc, utilization_percent)` — 设置当前值
- 背景轨道用 `lv_arc_set_bg_angles()` 设置半圆或全圆

### 5.2 Tile 1 — 设备状态页

竖排信息卡片样式。

| 信息项 | 数据来源 | 更新频率 |
|--------|----------|----------|
| 电池 SOC% | BQ27220 `readStateOfChargePercent()` | 1 秒 |
| 电池电压 mV | BQ27220 `readVoltageMillivolts()` | 1 秒 |
| 电池电流 mA | BQ27220 `readCurrentMilliamps()` | 1 秒 |
| 电池温度 °C | BQ27220 `readTemperatureCelsius()` | 1 秒 |
| WiFi RSSI | `WiFi.RSSI()` | 5 秒 |
| IP 地址 | `WiFi.localIP()` | 连接时更新 |
| RTC 时间 | PCF85063 读取 | 1 秒 |

I2C 总线共享，使用互斥锁 `xSemaphoreCreateMutex()` 避免冲突。

### 5.3 Tile 2 — 桌宠页

第一版为极简占位：

- 顶部标题："VIBEMATE"
- 中心：眨眼的简单表情（如 `^_^`）或跳动圆点
- 底部：随机状态文案（"Coding hard..." / "Take a break~"）

后续可扩展为帧动画或更复杂的 Lottie 动画。

---

## 6. 数据模型

### 6.1 Kimi API 数据结构

```cpp
struct kimi_usage_t {
    // 周限额
    float week_limit;
    float week_remaining;
    float week_used;
    float week_pct;          // 0~100
    String week_reset_time;  // ISO 8601 格式

    // 5小时窗口
    float window_limit;
    float window_remaining;
    float window_used;
    float window_pct;        // 0~100
    String window_reset_time;

    // 元信息
    String tier_name;        // 套餐等级，如 "Allegro"
    bool api_ok;             // 上次请求是否成功
    String last_error;       // 错误信息
    unsigned long last_update_ms;  // millis() 时间戳
};
```

### 6.2 全局实例

```cpp
extern kimi_usage_t g_kimi_data;    // 所有页面可读
extern bool g_ui_needs_update;       // UI 更新标志
```

---

## 7. 数据流

### 7.1 API 请求流程

```
api_refresh_timer (每 5 分钟触发)
    ↓
检查 WiFi 已连接？
    ↓ 否
跳过，保留上次数据
    ↓ 是
HTTPClient GET https://api.kimi.com/coding/v1/usages
    ↓
响应 200？
    ↓ 是
ArduinoJson 解析 JSON
    ↓
提取 limits[].detail（5小时窗口）和 usage（周限额）
    ↓
计算 used = limit - remaining, pct = used/limit*100
    ↓
填充 g_kimi_data，api_ok = true
    ↓
设置 g_ui_needs_update = true
    ↓ 否（401/403/5xx/网络错误）
api_ok = false，记录错误码到 last_error
设置 g_ui_needs_update = true（更新错误状态）
```

### 7.2 UI 更新流程

```
ui_update_timer (每 500ms)
    ↓
g_ui_needs_update == true？
    ↓ 否
跳过
    ↓ 是
g_ui_needs_update = false
    ↓
调用 ui_usage_update(&g_kimi_data)  # 更新用量页 arc/label
调用 ui_device_update()              # 更新设备页（从 BQ27220/WiFi/RTC）
```

**为什么不在 HTTP 回调里直接更新 UI？**

HTTP 请求在 Arduino 的 loop 任务中同步执行，而 LVGL 的渲染也在同一上下文。虽然此处不会跨线程，但使用标志位解耦可以：
- 避免 HTTP 超时阻塞 UI 渲染
- 统一更新入口，减少代码重复
- 手动刷新和自动刷新共用同一更新逻辑

### 7.3 手动刷新

用户点击用量页中心区域 → 触发 `LV_EVENT_CLICKED` → 调用 `kimi_api_refresh_now()` → 立即执行一次 HTTP 请求 → 设置 `g_ui_needs_update = true`。

---

## 8. 错误处理

| 场景 | 行为 | UI 表现 |
|------|------|---------|
| WiFi 连接失败（开机 30s 超时） | 进入离线模式，只显示设备页和桌宠页 | 屏幕显示 "WiFi Failed"，用量页显示 "--" |
| WiFi 运行中断开 | 自动重连（`network_check()` 每 loop 检测） | 状态点变红，设备页 RSSI 显示断开 |
| API Key 无效（401/403） | 停止请求，等待配置修改 | 状态点变红，中心显示 "Invalid Key" |
| Kimi 服务端错误（5xx） | 5 分钟后自动重试 | 状态点变黄，显示 "Server Error" |
| JSON 解析失败 | 记录到 Serial，保留上次数据 | 状态点变黄 |
| HTTP 网络超时 | 超时 10 秒，保留上次数据 | 状态点变黄 |
| 首次开机无数据 | 等待第一次请求成功 | 用量页显示 "--%" |
| 滑动到用量页时数据过期 | 如果超过 5 分钟未更新，自动触发一次刷新 | 静默刷新 |

### 8.1 状态指示点颜色规则

| 颜色 | 含义 |
|------|------|
| 绿色 `#4ade80` | 数据正常，请求成功 |
| 橙色 `#ffa726` | 请求失败但数据可用（使用缓存） |
| 红色 `#ef4444` | API Key 无效或无数据可用 |

---

## 9. 依赖库

### 9.1 项目自带库（位于 `Examples/Arduino-V3.2.0/libraries/`）

| 库 | 用途 |
|----|------|
| `lvgl` | LVGL v8.4 图形库 |
| `kode_bq27220` | BQ27220 电池电量计驱动 |
| `SensorLib` | PCF85063 RTC 驱动 |

### 9.2 Arduino IDE 内置/需额外安装的库

| 库 | 来源 | 用途 |
|----|------|------|
| `WiFi` | ESP32 Arduino Core 内置 | WiFi 连接 |
| `HTTPClient` | ESP32 Arduino Core 内置 | HTTP 请求 |
| `ArduinoJson` | Arduino Library Manager | JSON 解析 |
| `Wire` | Arduino 内置 | I2C 通信 |

---

## 10. 配置说明（`config.h`）

用户首次使用前必须编辑此文件：

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// WiFi 配置
#define WIFI_SSID     "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// Kimi API 配置
#define KIMI_API_KEY  "your-kimi-api-key"

// 刷新间隔（毫秒）
#define API_REFRESH_INTERVAL_MS  300000  // 5 分钟

// 背光亮度（0~100）
#define BACKLIGHT_BRIGHTNESS     80

#endif
```

---

## 11. 性能与资源预估

| 资源 | 预估用量 | 备注 |
|------|----------|------|
| Flash | ~500KB | LVGL + WiFi + HTTP + JSON |
| RAM | ~150KB | LVGL 双缓冲（360×360/10 × 2 × 2 bytes ≈ 52KB）+ 运行时 |
| PSRAM | 可选 | 可将 LVGL 缓冲移至 PSRAM |
| HTTP 请求频率 | 每 5 分钟 | 极低频率，不影响 API 限额 |

---

## 12. 后续扩展

### 12.1 桌宠页增强
- 帧动画（眨眼、跳跃、摇尾巴）
- 根据用量状态改变表情（用量高时紧张，用量低时开心）
- 语音反馈（ES8311 播放简短音效）

### 12.2 新增页面
- 用量历史趋势页（需本地存储到 SD 卡）
- 系统设置页（背光调节、刷新间隔配置）
- 通知中心（用量告警、低电量提醒）

### 12.3 交互增强
- 上下滑动呼出快捷菜单
- 双击唤醒/睡眠
- 摇晃刷新（QMI8658 检测）

---

## 13. 设计决策记录

| 决策 | 选择 | 原因 |
|------|------|------|
| 页面切换方式 | `lv_tileview` | 原生支持滑动动画和手势，代码最简洁 |
| 圆形屏幕适配 | 不预设安全区 | 先直接占满 360×360，根据实际效果再调整 |
| 配置存储 | `config.h` 头文件 | 第一版最简单，后续可迁移到 SD 卡/Flash |
| 刷新频率 | 5 分钟自动 + 手动 | 平衡实时性和 API 负载 |
| I2C 访问 | 互斥锁保护 | BQ27220、PCF85063、CST816 共享总线 |
| UI 更新 | 标志位解耦 | HTTP 请求不直接操作 LVGL 对象，统一更新入口 |

---

*文档生成时间: 2026-06-05*
*对应设计讨论: brainstorming session #1*
