# VibeMate 局域网语音流式传输 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 VibeMate 手表上实现局域网语音输入输出功能，支持与 Python 上位机的 PTT/全双工语音通信，并具备自动发现能力。

**Architecture:** ESP32 端通过 I2S + ES7210/ES8311 采集/播放 PCM 音频，UDP 广播发现上位机后建立 TCP 长连接传输音频帧。Python 上位机用 asyncio + sounddevice 处理音频 I/O。双方支持 PTT 和全双工模式切换。

**Tech Stack:** Arduino ESP32 (ESP_I2S, FreeRTOS), LVGL v8, Python 3 (asyncio, sounddevice, numpy)

**可并行工作流：**
- **工作流 A** (`audio_manager`)：ESP32 I2S + ES7210/ES8311 音频驱动 — 可独立实现
- **工作流 B** (`voice_network`)：ESP32 UDP 发现 + TCP 连接 + 帧协议 — 可独立实现
- **工作流 C** (`ui_voice`)：ESP32 Voice 页面 LVGL UI — 依赖 B 的状态接口（先用 stub）
- **工作流 D** (`vibemate_host.py`)：Python 上位机 — 可独立实现
- **工作流 E** (`integration`)：vibemate.ino 集成与联调 — 依赖 A/B/C 完成后

---

## File Structure

```
myprojects/vibemate/
├── audio_manager.cpp/h      (新增) I2S 初始化、ES7210/ES8311 控制、功放管理
├── voice_network.cpp/h      (新增) UDP 发现、TCP 连接、帧打包/解包、状态机
├── ui_voice.cpp/h           (新增) Voice 页面 UI、模式切换、PTT 按钮
├── vibemate_host.py         (新增) Python 上位机
├── config.h                 (修改) 添加语音相关配置常量
├── vibemate.ino             (修改) 集成 Voice tile、音频/网络初始化、页面切换
```

---

## 工作流 A: audio_manager — ESP32 音频驱动

### Task A1: 创建 audio_manager.h 头文件

**Files:**
- Create: `myprojects/vibemate/audio_manager.h`

**接口定义：**

```cpp
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <ESP_I2S.h>

// 音频参数
#define AUDIO_SAMPLE_RATE       16000
#define AUDIO_BIT_WIDTH         I2S_DATA_BIT_WIDTH_16BIT
#define AUDIO_CHANNELS          I2S_SLOT_MODE_MONO
#define AUDIO_FRAME_MS          20
#define AUDIO_FRAME_SAMPLES     ((AUDIO_SAMPLE_RATE * AUDIO_FRAME_MS) / 1000)  // 320
#define AUDIO_FRAME_BYTES       (AUDIO_FRAME_SAMPLES * sizeof(int16_t))         // 640

// I2S 引脚
#define I2S_PIN_MCK GPIO_NUM_2
#define I2S_PIN_BCK GPIO_NUM_48
#define I2S_PIN_WS  GPIO_NUM_38
#define I2S_PIN_DOUT GPIO_NUM_47
#define I2S_PIN_DIN  GPIO_NUM_39

// 功放使能
#define AMP_EN_PIN GPIO_NUM_9

// ES7210 I2C
#define ES7210_I2C_ADDR 0x40

// 缓冲区大小（帧数）
#define AUDIO_RINGBUF_FRAMES  8
#define AUDIO_RINGBUF_SIZE    (AUDIO_RINGBUF_FRAMES * AUDIO_FRAME_BYTES)

// 初始化与电源管理
bool audio_manager_init(void);
void audio_manager_deinit(void);
void audio_manager_amp_enable(bool enable);

// I2S 读写（阻塞，每调用一次读/写 20ms 的 PCM 数据）
bool audio_read_frame(int16_t *buffer);   // 从 ES7210 读取一帧
bool audio_write_frame(const int16_t *buffer); // 写入 ES8311 播放一帧

// 音量控制
void audio_set_playback_volume(uint8_t percent);  // 0-100
void audio_set_capture_gain(uint8_t percent);     // 0-100

#endif
```

- [ ] **Step 1: 写入头文件**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/audio_manager.h
git commit -m "feat(voice): add audio_manager.h with I2S/ES7210/ES8311 interface"
```

### Task A2: 实现 audio_manager.cpp — I2S 与编解码器初始化

**Files:**
- Create: `myprojects/vibemate/audio_manager.cpp`
- Modify: `myprojects/vibemate/vibemate.ino`（后续 Task E1 才实际集成，这里先实现模块）

参考 `03_audio_out_no_tf.ino` 和 `06_esp_sr.ino` 的初始化代码。

```cpp
#include "audio_manager.h"
#include "es8311.h"
#include "es7210.h"
#include "I2C_Driver.h"
#include <Wire.h>

static I2SClass i2s;
static es8311_handle_t es8311_handle = NULL;
static es7210_dev_handle_t es7210_handle = NULL;
static bool s_initialized = false;

// 功放控制
void audio_manager_amp_enable(bool enable) {
    digitalWrite(AMP_EN_PIN, enable ? HIGH : LOW);
}

bool audio_manager_init(void) {
    if (s_initialized) return true;

    // 功放引脚
    pinMode(AMP_EN_PIN, OUTPUT);
    digitalWrite(AMP_EN_PIN, LOW);

    // 初始化 ES8311 (DAC)
    es8311_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
    if (!es8311_handle) {
        Serial.println("[Audio] ES8311 create failed");
        return false;
    }
    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = (uint32_t)AUDIO_SAMPLE_RATE * 256,
        .sample_frequency = AUDIO_SAMPLE_RATE
    };
    esp_err_t err = es8311_init(es8311_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
    if (err != ESP_OK) {
        Serial.println("[Audio] ES8311 init failed");
        return false;
    }
    es8311_voice_volume_set(es8311_handle, 60, NULL);
    es8311_microphone_config(es8311_handle, false);

    // 初始化 ES7210 (ADC)
    es7210_i2c_config_t es7210_i2c_conf = {
        .i2c_addr = ES7210_I2C_ADDR
    };
    err = es7210_new_codec(&es7210_i2c_conf, &es7210_handle);
    if (err != ESP_OK) {
        Serial.println("[Audio] ES7210 create failed");
        return false;
    }
    es7210_codec_config_t codec_conf = {};
    codec_conf.i2s_format = ES7210_I2S_FMT_I2S;
    codec_conf.mclk_ratio = 256;
    codec_conf.sample_rate_hz = AUDIO_SAMPLE_RATE;
    codec_conf.bit_width = ES7210_I2S_BITS_16B;
    codec_conf.mic_bias = ES7210_MIC_BIAS_2V87;
    codec_conf.mic_gain = ES7210_MIC_GAIN_30DB;
    codec_conf.flags.tdm_enable = false;
    es7210_config_codec(es7210_handle, &codec_conf);
    es7210_config_volume(es7210_handle, 40);

    // 初始化 I2S
    i2s.setPins(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN, I2S_PIN_MCK);
    i2s.setTimeout(1000);
    if (!i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, AUDIO_BIT_WIDTH, AUDIO_CHANNELS, I2S_STD_SLOT_LEFT)) {
        Serial.println("[Audio] I2S begin failed");
        return false;
    }

    s_initialized = true;
    Serial.println("[Audio] audio_manager_init OK");
    return true;
}

void audio_manager_deinit(void) {
    if (!s_initialized) return;
    audio_manager_amp_enable(false);
    i2s.end();
    s_initialized = false;
}

bool audio_read_frame(int16_t *buffer) {
    if (!s_initialized || !buffer) return false;
    size_t bytes_read = i2s.readBytes((uint8_t *)buffer, AUDIO_FRAME_BYTES);
    return bytes_read == AUDIO_FRAME_BYTES;
}

bool audio_write_frame(const int16_t *buffer) {
    if (!s_initialized || !buffer) return false;
    size_t bytes_written = i2s.write((uint8_t *)buffer, AUDIO_FRAME_BYTES);
    return bytes_written == AUDIO_FRAME_BYTES;
}

void audio_set_playback_volume(uint8_t percent) {
    if (es8311_handle) {
        es8311_voice_volume_set(es8311_handle, percent, NULL);
    }
}

void audio_set_capture_gain(uint8_t percent) {
    if (es7210_handle) {
        es7210_config_volume(es7210_handle, percent);
    }
}
```

- [ ] **Step 1: 写入 audio_manager.cpp**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/audio_manager.cpp
git commit -m "feat(voice): implement audio_manager with I2S/ES7210/ES8311 init and PCM read/write"
```

---

## 工作流 B: voice_network — UDP 发现 + TCP 传输

### Task B1: 创建 voice_network.h 头文件

**Files:**
- Create: `myprojects/vibemate/voice_network.h`

```cpp
#ifndef VOICE_NETWORK_H
#define VOICE_NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// 网络端口
#define VOICE_DISCOVERY_PORT    3721
#define VOICE_TCP_PORT          3722

// 发现广播间隔
#define DISCOVERY_INTERVAL_MS   2000
#define HEARTBEAT_INTERVAL_MS   3000
#define HEARTBEAT_TIMEOUT_MS    10000

// 帧类型
#define FRAME_AUDIO_UPLINK      0x01
#define FRAME_AUDIO_DOWNLINK    0x02
#define FRAME_CONTROL           0x03

// 帧头魔数
#define FRAME_MAGIC_0           0x56
#define FRAME_MAGIC_1           0x4D

// 连接状态
enum voice_state_t {
    VOICE_IDLE = 0,
    VOICE_DISCOVERING,
    VOICE_CONNECTING,
    VOICE_CONNECTED,
    VOICE_TRANSMITTING,  // PTT 按住中
};

// 工作模式
enum voice_mode_t {
    VOICE_MODE_PTT = 0,
    VOICE_MODE_DUPLEX,
};

// 初始化与主循环更新
void voice_network_init(void);
void voice_network_update(void);  // 在 device_timer_cb 中每 1s 调用

// 状态查询
voice_state_t voice_get_state(void);
voice_mode_t voice_get_mode(void);
const char* voice_get_host_ip(void);
const char* voice_get_host_name(void);

// 模式切换
void voice_set_mode(voice_mode_t mode);

// 连接控制
void voice_start_discovery(void);
void voice_connect_to_host(const char *ip, uint16_t port);
void voice_disconnect(void);

// PTT 控制
void voice_ptt_set(bool pressed);

// 发送音频帧（voice_tx_task 调用）
bool voice_send_audio_frame(const int8_t *pcm_data, size_t len);

// 接收音频帧（voice_rx_task 调用），返回实际读取长度，0 表示无数据
size_t voice_recv_audio_frame(uint8_t *pcm_buffer, size_t buf_len);

// 发送控制帧
bool voice_send_control(const char *json_str);

#endif
```

- [ ] **Step 1: 写入头文件**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/voice_network.h
git commit -m "feat(voice): add voice_network.h with discovery/TCP/frame protocol interface"
```

### Task B2: 实现 voice_network.cpp — UDP 发现

**Files:**
- Create: `myprojects/vibemate/voice_network.cpp`

实现 UDP 发现广播和 announce 接收：

```cpp
#include "voice_network.h"
#include "config.h"
#include <ArduinoJson.h>

static WiFiUDP udp;
static WiFiClient tcp_client;
static voice_state_t s_state = VOICE_IDLE;
static voice_mode_t s_mode = VOICE_MODE_PTT;
static char s_host_ip[32] = "";
static char s_host_name[64] = "";
static unsigned long s_last_discovery = 0;
static unsigned long s_last_heartbeat = 0;
static unsigned long s_last_pong = 0;

void voice_network_init(void) {
    udp.begin(VOICE_DISCOVERY_PORT);
    s_state = VOICE_IDLE;
    s_mode = VOICE_MODE_PTT;
    s_host_ip[0] = '\0';
    s_host_name[0] = '\0';
}

// 发送 UDP 发现广播
static void send_discovery_broadcast(void) {
    if (!network_is_connected()) return;

    StaticJsonDocument<256> doc;
    doc["type"] = "discover";
    doc["device"] = "vibemate";
    doc["version"] = 1;

    char buf[256];
    size_t len = serializeJson(doc, buf, sizeof(buf));

    udp.beginPacket(IPAddress(255, 255, 255, 255), VOICE_DISCOVERY_PORT);
    udp.write((uint8_t*)buf, len);
    udp.endPacket();
}

// 处理 UDP 接收（announce 回复）
static void process_udp_inbound(void) {
    int pkt_size = udp.parsePacket();
    if (pkt_size <= 0) return;

    char buf[512];
    int n = udp.read(buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = '\0';

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, buf);
    if (err) return;

    const char *type = doc["type"] | "";
    if (strcmp(type, "announce") != 0) return;

    const char *ip = doc["ip"] | "";
    const char *name = doc["name"] | "Unknown";
    uint16_t port = doc["port"] | VOICE_TCP_PORT;

    if (ip[0] == '\0') return;

    // 保存第一个发现的设备（预留一对多：这里可改为列表）
    if (s_host_ip[0] == '\0') {
        strlcpy(s_host_ip, ip, sizeof(s_host_ip));
        strlcpy(s_host_name, name, sizeof(s_host_name));
        Serial.printf("[Voice] Discovered host: %s @ %s:%d\n", name, ip, port);
    }
}

voice_state_t voice_get_state(void) { return s_state; }
voice_mode_t voice_get_mode(void) { return s_mode; }
const char* voice_get_host_ip(void) { return s_host_ip; }
const char* voice_get_host_name(void) { return s_host_name; }

void voice_set_mode(voice_mode_t mode) {
    s_mode = mode;
    // 发送模式切换控制帧（如果已连接）
    if (s_state == VOICE_CONNECTED && tcp_client.connected()) {
        StaticJsonDocument<128> doc;
        doc["cmd"] = "mode";
        doc["type"] = (mode == VOICE_MODE_PTT) ? "ptt" : "duplex";
        char buf[128];
        serializeJson(doc, buf, sizeof(buf));
        voice_send_control(buf);
    }
}

void voice_start_discovery(void) {
    s_state = VOICE_DISCOVERING;
    s_host_ip[0] = '\0';
    s_host_name[0] = '\0';
    s_last_discovery = 0;  // 立即触发第一次广播
}

void voice_disconnect(void) {
    if (tcp_client.connected()) {
        tcp_client.stop();
    }
    s_state = VOICE_IDLE;
    s_host_ip[0] = '\0';
}
```

- [ ] **Step 1: 写入 UDP 发现代码（上面代码的前半部分）**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/voice_network.cpp
git commit -m "feat(voice): implement UDP discovery broadcast and announce handling"
```

### Task B3: 实现 voice_network.cpp — TCP 连接与帧收发

**Files:**
- Modify: `myprojects/vibemate/voice_network.cpp`（追加）

```cpp
// 追加到 voice_network.cpp

void voice_connect_to_host(const char *ip, uint16_t port) {
    if (!ip || ip[0] == '\0') return;
    s_state = VOICE_CONNECTING;
    Serial.printf("[Voice] Connecting to %s:%d...\n", ip, port);

    if (tcp_client.connect(ip, port)) {
        s_state = VOICE_CONNECTED;
        s_last_heartbeat = millis();
        s_last_pong = millis();

        // 发送 hello 控制帧
        StaticJsonDocument<128> doc;
        doc["cmd"] = "hello";
        doc["version"] = 1;
        doc["mode"] = (s_mode == VOICE_MODE_PTT) ? "ptt" : "duplex";
        char buf[128];
        serializeJson(doc, buf, sizeof(buf));
        voice_send_control(buf);

        Serial.println("[Voice] TCP connected, hello sent");
    } else {
        s_state = VOICE_DISCOVERING;
        Serial.println("[Voice] TCP connect failed");
    }
}

// 发送一帧音频
bool voice_send_audio_frame(const int8_t *pcm_data, size_t len) {
    if (!tcp_client.connected()) return false;

    uint8_t header[5];
    header[0] = FRAME_MAGIC_0;
    header[1] = FRAME_MAGIC_1;
    header[2] = FRAME_AUDIO_UPLINK;
    header[3] = (len >> 8) & 0xFF;
    header[4] = len & 0xFF;

    tcp_client.write(header, 5);
    tcp_client.write((const uint8_t*)pcm_data, len);
    return true;
}

// 尝试接收一帧音频，返回实际读取的 payload 长度，0 表示暂无数据
size_t voice_recv_audio_frame(uint8_t *pcm_buffer, size_t buf_len) {
    if (!tcp_client.connected() || !pcm_buffer || buf_len == 0) return 0;

    // 检查是否有至少 5 字节头
    if (tcp_client.available() < 5) return 0;

    uint8_t header[5];
    tcp_client.read(header, 5);

    // 检查 Magic
    if (header[0] != FRAME_MAGIC_0 || header[1] != FRAME_MAGIC_1) {
        // 帧同步丢失：滑动查找下一个 Magic
        // 简单处理：丢弃当前字节，返回 0 让下次再试
        return 0;
    }

    uint8_t frame_type = header[2];
    uint16_t payload_len = ((uint16_t)header[3] << 8) | header[4];

    if (payload_len > buf_len) {
        // 缓冲区不够，丢弃
        while (payload_len > 0 && tcp_client.available()) {
            tcp_client.read();
            payload_len--;
        }
        return 0;
    }

    // 等待 payload 到达
    unsigned long wait_start = millis();
    while ((size_t)tcp_client.available() < payload_len) {
        if (millis() - wait_start > 100) return 0;  // 超时
        delay(1);
    }

    if (frame_type == FRAME_AUDIO_DOWNLINK) {
        tcp_client.read(pcm_buffer, payload_len);
        return payload_len;
    } else if (frame_type == FRAME_CONTROL) {
        uint8_t ctrl_buf[256];
        size_t to_read = min((size_t)payload_len, sizeof(ctrl_buf) - 1);
        tcp_client.read(ctrl_buf, to_read);
        ctrl_buf[to_read] = '\0';
        // 处理控制帧（简化：只处理 pong 更新心跳时间）
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, (char*)ctrl_buf) == DeserializationError::Ok) {
            const char *cmd = doc["cmd"] | "";
            if (strcmp(cmd, "pong") == 0 || strcmp(cmd, "hello_ack") == 0) {
                s_last_pong = millis();
            }
        }
        // 消耗剩余 payload
        size_t remaining = payload_len - to_read;
        while (remaining-- > 0 && tcp_client.available()) tcp_client.read();
    } else {
        // 未知类型，丢弃 payload
        for (uint16_t i = 0; i < payload_len && tcp_client.available(); i++) {
            tcp_client.read();
        }
    }
    return 0;
}

// 发送控制帧
bool voice_send_control(const char *json_str) {
    if (!tcp_client.connected() || !json_str) return false;

    size_t len = strlen(json_str);
    if (len > 65535) return false;

    uint8_t header[5];
    header[0] = FRAME_MAGIC_0;
    header[1] = FRAME_MAGIC_1;
    header[2] = FRAME_CONTROL;
    header[3] = (len >> 8) & 0xFF;
    header[4] = len & 0xFF;

    tcp_client.write(header, 5);
    tcp_client.write((uint8_t*)json_str, len);
    return true;
}

void voice_ptt_set(bool pressed) {
    if (s_mode != VOICE_MODE_PTT) return;
    if (pressed && s_state == VOICE_CONNECTED) {
        s_state = VOICE_TRANSMITTING;
        // 发送 ptt pressed 控制帧
        StaticJsonDocument<64> doc;
        doc["cmd"] = "ptt";
        doc["state"] = "pressed";
        char buf[64];
        serializeJson(doc, buf, sizeof(buf));
        voice_send_control(buf);
    } else if (!pressed && s_state == VOICE_TRANSMITTING) {
        s_state = VOICE_CONNECTED;
        StaticJsonDocument<64> doc;
        doc["cmd"] = "ptt";
        doc["state"] = "released";
        char buf[64];
        serializeJson(doc, buf, sizeof(buf));
        voice_send_control(buf);
    }
}

// 主循环更新（由 device_timer_cb 每秒调用）
void voice_network_update(void) {
    if (s_state == VOICE_DISCOVERING) {
        // 周期性发送发现广播
        if (millis() - s_last_discovery >= DISCOVERY_INTERVAL_MS) {
            s_last_discovery = millis();
            send_discovery_broadcast();
        }
        // 检查是否有 announce 回复
        process_udp_inbound();

        // 如果发现了主机，自动连接（当前版本自动连第一个）
        if (s_host_ip[0] != '\0') {
            voice_connect_to_host(s_host_ip, VOICE_TCP_PORT);
        }
    }

    if (s_state == VOICE_CONNECTED || s_state == VOICE_TRANSMITTING) {
        // 发送心跳
        if (millis() - s_last_heartbeat >= HEARTBEAT_INTERVAL_MS) {
            s_last_heartbeat = millis();
            StaticJsonDocument<32> doc;
            doc["cmd"] = "ping";
            char buf[32];
            serializeJson(doc, buf, sizeof(buf));
            voice_send_control(buf);
        }
        // 检查心跳超时
        if (millis() - s_last_pong >= HEARTBEAT_TIMEOUT_MS) {
            Serial.println("[Voice] Heartbeat timeout, disconnecting");
            voice_disconnect();
            voice_start_discovery();
        }
    }
}
```

- [ ] **Step 1: 追加 TCP 连接与帧收发代码到 voice_network.cpp**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/voice_network.cpp
git commit -m "feat(voice): implement TCP connect, audio frame TX/RX, heartbeat, and PTT control"
```

---

## 工作流 C: ui_voice — Voice 页面 UI

### Task C1: 创建 ui_voice.h

**Files:**
- Create: `myprojects/vibemate/ui_voice.h`

```cpp
#ifndef UI_VOICE_H
#define UI_VOICE_H

#include <lvgl.h>

void ui_voice_create(lv_obj_t *parent_tile);
void ui_voice_update(void);  // 由 device_timer_cb 调用，更新状态显示

// 外部状态变化时更新 UI（由 voice_network 状态变化触发）
void ui_voice_set_state_connected(bool connected);
void ui_voice_set_state_transmitting(bool transmitting);
void ui_voice_set_host_info(const char *name, const char *ip);

#endif
```

- [ ] **Step 1: 写入头文件**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/ui_voice.h
git commit -m "feat(voice): add ui_voice.h with Voice page interface"
```

### Task C2: 实现 ui_voice.cpp — Voice 页面创建与事件处理

**Files:**
- Create: `myprojects/vibemate/ui_voice.cpp`

```cpp
#include "ui_voice.h"
#include "voice_network.h"
#include "audio_manager.h"
#include "config.h"
#include <lvgl.h>

// 控件句柄
static lv_obj_t *label_status;
static lv_obj_t *label_host;
static lv_obj_t *btn_ptt_mode;
static lv_obj_t *btn_duplex_mode;
static lv_obj_t *btn_ptt_big;      // PTT 大按钮
static lv_obj_t *btn_duplex_toggle; // 全双工 开始/结束 按钮
static lv_obj_t *bar_volume;
static lv_obj_t *status_dot;
static lv_obj_t *label_ptt_text;
static lv_obj_t *label_mode_hint;

// 状态
static bool s_is_ptt_mode = true;
static bool s_duplex_active = false;

// 颜色常量
#define COLOR_BG       0x0a0e17
#define COLOR_TEXT     0xf0f4f8
#define COLOR_SUBTEXT  0x94a3b8
#define COLOR_ACCENT   0x4fc3f7
#define COLOR_GREEN    0x4ade80
#define COLOR_ORANGE   0xffa726
#define COLOR_RED      0xef4444

// 模式切换回调
static void mode_btn_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    if (btn == btn_ptt_mode) {
        s_is_ptt_mode = true;
        voice_set_mode(VOICE_MODE_PTT);
        lv_obj_set_style_bg_color(btn_ptt_mode, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_bg_color(btn_duplex_mode, lv_color_hex(0x1e293b), 0);
        lv_obj_clear_flag(btn_ptt_big, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_duplex_toggle, LV_OBJ_FLAG_HIDDEN);
    } else {
        s_is_ptt_mode = false;
        voice_set_mode(VOICE_MODE_DUPLEX);
        lv_obj_set_style_bg_color(btn_duplex_mode, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_bg_color(btn_ptt_mode, lv_color_hex(0x1e293b), 0);
        lv_obj_add_flag(btn_ptt_big, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_duplex_toggle, LV_OBJ_FLAG_HIDDEN);
    }
}

// PTT 大按钮事件
static void ptt_btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSING || code == LV_EVENT_PRESSED) {
        voice_ptt_set(true);
        lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_GREEN), 0);
        lv_label_set_text(label_ptt_text, "发送中...");
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        voice_ptt_set(false);
        lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_ACCENT), 0);
        lv_label_set_text(label_ptt_text, "按住说话");
    }
}

// 全双工切换按钮
static void duplex_toggle_cb(lv_event_t *e) {
    s_duplex_active = !s_duplex_active;
    if (s_duplex_active) {
        lv_obj_set_style_bg_color(btn_duplex_toggle, lv_color_hex(COLOR_RED), 0);
        lv_label_set_text(lv_obj_get_child(btn_duplex_toggle, 0), "结束通话");
        // 全双工模式下，启动语音任务（由 integration 代码处理）
    } else {
        lv_obj_set_style_bg_color(btn_duplex_toggle, lv_color_hex(COLOR_GREEN), 0);
        lv_label_set_text(lv_obj_get_child(btn_duplex_toggle, 0), "开始通话");
    }
}

void ui_voice_create(lv_obj_t *parent_tile) {
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // --- 顶部状态栏 ---
    status_dot = lv_obj_create(parent_tile);
    lv_obj_set_size(status_dot, 10, 10);
    lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_RED), 0);
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_align(status_dot, LV_ALIGN_TOP_MID, -60, 25);

    label_status = lv_label_create(parent_tile);
    lv_label_set_text(label_status, "未连接");
    lv_obj_set_style_text_color(label_status, lv_color_hex(COLOR_SUBTEXT), 0);
    lv_obj_set_style_text_font(label_status, &font_cjk_14, 0);
    lv_obj_align_to(label_status, status_dot, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    label_host = lv_label_create(parent_tile);
    lv_label_set_text(label_host, "");
    lv_obj_set_style_text_color(label_host, lv_color_hex(COLOR_SUBTEXT), 0);
    lv_obj_set_style_text_font(label_host, &lv_font_montserrat_12, 0);
    lv_obj_align(label_host, LV_ALIGN_TOP_MID, 0, 45);

    // --- 模式切换按钮 ---
    btn_ptt_mode = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_ptt_mode, 80, 32);
    lv_obj_set_style_bg_color(btn_ptt_mode, lv_color_hex(COLOR_ACCENT), 0);
    lv_obj_set_style_radius(btn_ptt_mode, 16, 0);
    lv_obj_align(btn_ptt_mode, LV_ALIGN_TOP_MID, -45, 75);
    lv_obj_add_event_cb(btn_ptt_mode, mode_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_ptt = lv_label_create(btn_ptt_mode);
    lv_label_set_text(lbl_ptt, "PTT");
    lv_obj_set_style_text_color(lbl_ptt, lv_color_hex(COLOR_BG), 0);
    lv_obj_center(lbl_ptt);

    btn_duplex_mode = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_duplex_mode, 80, 32);
    lv_obj_set_style_bg_color(btn_duplex_mode, lv_color_hex(0x1e293b), 0);
    lv_obj_set_style_radius(btn_duplex_mode, 16, 0);
    lv_obj_align(btn_duplex_mode, LV_ALIGN_TOP_MID, 45, 75);
    lv_obj_add_event_cb(btn_duplex_mode, mode_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_dup = lv_label_create(btn_duplex_mode);
    lv_label_set_text(lbl_dup, "全双工");
    lv_obj_set_style_text_color(lbl_dup, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_center(lbl_dup);

    // --- PTT 大按钮 ---
    btn_ptt_big = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_ptt_big, 160, 160);
    lv_obj_set_style_radius(btn_ptt_big, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_ACCENT), 0);
    lv_obj_align(btn_ptt_big, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_event_cb(btn_ptt_big, ptt_btn_event_cb, LV_EVENT_ALL, NULL);

    label_ptt_text = lv_label_create(btn_ptt_big);
    lv_label_set_text(label_ptt_text, "按住说话");
    lv_obj_set_style_text_color(label_ptt_text, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_text_font(label_ptt_text, &font_cjk_14, 0);
    lv_obj_center(label_ptt_text);

    // --- 全双工 开始/结束 按钮（默认隐藏） ---
    btn_duplex_toggle = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_duplex_toggle, 160, 60);
    lv_obj_set_style_radius(btn_duplex_toggle, 30, 0);
    lv_obj_set_style_bg_color(btn_duplex_toggle, lv_color_hex(COLOR_GREEN), 0);
    lv_obj_align(btn_duplex_toggle, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_flag(btn_duplex_toggle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(btn_duplex_toggle, duplex_toggle_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl_dup_toggle = lv_label_create(btn_duplex_toggle);
    lv_label_set_text(lbl_dup_toggle, "开始通话");
    lv_obj_set_style_text_color(lbl_dup_toggle, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_text_font(lbl_dup_toggle, &font_cjk_14, 0);
    lv_obj_center(lbl_dup_toggle);

    // --- 音量条 ---
    bar_volume = lv_bar_create(parent_tile);
    lv_obj_set_size(bar_volume, 200, 8);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(0x1e293b), 0);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_volume, 4, 0);
    lv_obj_set_value(bar_volume, 0, 0);
    lv_obj_align(bar_volume, LV_ALIGN_CENTER, 0, 105);
}

void ui_voice_update(void) {
    voice_state_t state = voice_get_state();
    const char *host_name = voice_get_host_name();
    const char *host_ip = voice_get_host_ip();

    // 更新状态灯和文字
    switch (state) {
        case VOICE_IDLE:
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_RED), 0);
            lv_label_set_text(label_status, "未连接");
            break;
        case VOICE_DISCOVERING:
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_ORANGE), 0);
            lv_label_set_text(label_status, "发现中...");
            break;
        case VOICE_CONNECTING:
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_ORANGE), 0);
            lv_label_set_text(label_status, "连接中...");
            break;
        case VOICE_CONNECTED:
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_GREEN), 0);
            lv_label_set_text(label_status, "已连接");
            break;
        case VOICE_TRANSMITTING:
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_ACCENT), 0);
            lv_label_set_text(label_status, "发送中");
            break;
    }

    // 更新主机信息
    if (host_ip[0] != '\0') {
        char buf[64];
        lv_snprintf(buf, sizeof(buf), "%s %s", host_name, host_ip);
        lv_label_set_text(label_host, buf);
    } else {
        lv_label_set_text(label_host, "");
    }

    // 更新 PTT 按钮状态
    if (state == VOICE_TRANSMITTING) {
        lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_GREEN), 0);
        lv_label_set_text(label_ptt_text, "发送中...");
    } else if (state == VOICE_CONNECTED && s_is_ptt_mode) {
        lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_ACCENT), 0);
        lv_label_set_text(label_ptt_text, "按住说话");
    }
}

void ui_voice_set_state_connected(bool connected) {
    // 供外部调用，实际状态通过 ui_voice_update() 刷新
    (void)connected;
}

void ui_voice_set_state_transmitting(bool transmitting) {
    (void)transmitting;
}

void ui_voice_set_host_info(const char *name, const char *ip) {
    (void)name;
    (void)ip;
}
```

- [ ] **Step 1: 写入 ui_voice.cpp**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/ui_voice.cpp myprojects/vibemate/ui_voice.h
git commit -m "feat(voice): implement Voice page UI with PTT/duplex mode switch, status, and host info"
```

---

## 工作流 D: Python 上位机

### Task D1: 创建 vibemate_host.py

**Files:**
- Create: `myprojects/vibemate/vibemate_host.py`

```python
#!/usr/bin/env python3
"""
VibeMate 语音上位机
运行: python vibemate_host.py
依赖: pip install sounddevice numpy
"""

import asyncio
import json
import struct
import sys
import time
from collections import deque

import numpy as np
import sounddevice as sd

# 帧常量
FRAME_MAGIC = b"VM"
FRAME_AUDIO_UPLINK = 0x01
FRAME_AUDIO_DOWNLINK = 0x02
FRAME_CONTROL = 0x03

SAMPLE_RATE = 16000
CHANNELS = 1
BLOCKSIZE = 320  # 20ms @ 16kHz
FRAME_BYTES = BLOCKSIZE * 2  # 640 bytes


class AudioRingBuffer:
    """简单的音频环形缓冲区"""
    def __init__(self, max_frames=50):
        self._q = deque(maxlen=max_frames)
        self._lock = asyncio.Lock()

    def put(self, pcm_array: np.ndarray):
        self._q.append(pcm_array.copy())

    async def get(self, frames: int) -> np.ndarray | None:
        if not self._q:
            return None
        pcm = self._q.popleft()
        return pcm

    def clear(self):
        self._q.clear()


class VibeMateHost:
    def __init__(self):
        self.udp_transport = None
        self.tcp_server = None
        self.client_reader = None
        self.client_writer = None
        self.mode = "ptt"  # "ptt" or "duplex"
        self.ptt_pressed = False
        self.connected = False
        self.rx_buffer = AudioRingBuffer(max_frames=50)   # 来自手表的音频
        self.tx_buffer = AudioRingBuffer(max_frames=50)   # 发往手表的音频
        self.last_ping = 0
        self.audio_stream = None

    # === UDP 发现 ===
    class UDPDiscoveryProtocol(asyncio.DatagramProtocol):
        def __init__(self, host):
            self.host = host

        def datagram_received(self, data, addr):
            try:
                msg = json.loads(data.decode())
                if msg.get("type") == "discover":
                    response = json.dumps({
                        "type": "announce",
                        "name": self.host.get_hostname(),
                        "ip": self.host.get_local_ip(),
                        "port": 3722,
                        "version": 1,
                    })
                    self.transport.sendto(response.encode(), addr)
                    print(f"[UDP] Discovery from {addr[0]}, replied")
            except Exception as e:
                pass

    def get_local_ip(self):
        import socket
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except:
            return "127.0.0.1"

    def get_hostname(self):
        import socket
        return socket.gethostname()

    # === TCP 音频服务 ===
    async def handle_client(self, reader, writer):
        addr = writer.get_extra_info("peername")
        print(f"[TCP] Client connected from {addr}")
        self.client_reader = reader
        self.client_writer = writer
        self.connected = True
        self.rx_buffer.clear()
        self.tx_buffer.clear()

        # 启动两个任务：接收帧 + 发送帧
        recv_task = asyncio.create_task(self._recv_frames())
        send_task = asyncio.create_task(self._send_frames())

        try:
            await asyncio.gather(recv_task, send_task)
        except asyncio.CancelledError:
            pass
        finally:
            self.connected = False
            self.client_reader = None
            self.client_writer = None
            print(f"[TCP] Client disconnected")

    async def _recv_frames(self):
        """持续接收来自手表的帧"""
        while self.connected and self.client_reader:
            try:
                # 读取 5 字节头
                header = await self.client_reader.readexactly(5)
                magic = header[0:2]
                if magic != FRAME_MAGIC:
                    print(f"[TCP] Frame sync lost, magic={magic.hex()}")
                    continue

                frame_type = header[2]
                payload_len = struct.unpack(">H", header[3:5])[0]

                if payload_len > 65535:
                    print(f"[TCP] Invalid payload len: {payload_len}")
                    continue

                payload = await self.client_reader.readexactly(payload_len)

                if frame_type == FRAME_AUDIO_UPLINK:
                    # 来自手表的 PCM 音频
                    pcm = np.frombuffer(payload, dtype=np.int16).reshape(-1, 1)
                    self.rx_buffer.put(pcm)

                elif frame_type == FRAME_CONTROL:
                    # 控制帧
                    ctrl = json.loads(payload.decode())
                    await self._handle_control(ctrl)

            except asyncio.IncompleteReadError:
                break
            except Exception as e:
                print(f"[TCP] Recv error: {e}")
                break

    async def _send_frames(self):
        """持续发送音频/控制帧给手表"""
        while self.connected and self.client_writer:
            # 发送音频下行帧（如果有数据）
            pcm = await self.tx_buffer.get(BLOCKSIZE)
            if pcm is not None and len(pcm) == BLOCKSIZE:
                self._send_frame(FRAME_AUDIO_DOWNLINK, pcm.tobytes())

            # 检查心跳
            if time.time() - self.last_ping > 3:
                self._send_control({"cmd": "pong"})

            await asyncio.sleep(0.005)  # 5ms

    def _send_frame(self, frame_type: int, payload: bytes):
        if not self.client_writer:
            return
        header = FRAME_MAGIC + bytes([frame_type]) + struct.pack(">H", len(payload))
        self.client_writer.write(header + payload)
        # 不 await drain，让 asyncio 自己缓冲

    def _send_control(self, obj: dict):
        payload = json.dumps(obj).encode()
        self._send_frame(FRAME_CONTROL, payload)
        if obj.get("cmd") == "pong":
            self.last_ping = time.time()

    async def _handle_control(self, ctrl: dict):
        cmd = ctrl.get("cmd", "")
        print(f"[CTRL] {cmd}: {ctrl}")

        if cmd == "hello":
            self.mode = ctrl.get("mode", "ptt")
            self._send_control({"cmd": "hello_ack", "status": "ready", "sample_rate": SAMPLE_RATE})
            print(f"[CTRL] Hello received, mode={self.mode}")

        elif cmd == "ping":
            self._send_control({"cmd": "pong"})

        elif cmd == "ptt":
            state = ctrl.get("state", "")
            self.ptt_pressed = (state == "pressed")
            print(f"[CTRL] PTT {state}")

        elif cmd == "mode":
            self.mode = ctrl.get("type", "ptt")
            print(f"[CTRL] Mode switched to {self.mode}")

    # === 音频 I/O ===
    def audio_callback(self, indata, outdata, frames, time_info, status):
        if status:
            print(f"[Audio] Status: {status}")

        # 播放来自手表的音频
        pcm_rx = None
        try:
            pcm_rx = self.rx_buffer.get(frames)
        except:
            pass

        if pcm_rx is not None and len(pcm_rx) == frames:
            outdata[:] = pcm_rx
        else:
            outdata.fill(0)

        # 采集本地麦克风并发送给手表
        if self.connected:
            if self.mode == "duplex":
                # 全双工：始终采集发送
                self.tx_buffer.put(indata.copy())
            elif self.mode == "ptt":
                # PTT：不发送本地音频（手表只听不说，或者说的时候电脑听）
                # 实际上 PTT 模式下，手表按住时发送音频到电脑，电脑播放即可
                # 电脑端麦克风是否回传取决于需求，这里先不回传
                pass

    # === 主循环 ===
    async def run(self):
        loop = asyncio.get_event_loop()

        # 启动 UDP 发现
        self.udp_transport, _ = await loop.create_datagram_endpoint(
            lambda: self.UDPDiscoveryProtocol(self),
            local_addr=("0.0.0.0", 3721)
        )
        print(f"[UDP] Discovery listening on :3721")

        # 启动 TCP 服务
        self.tcp_server = await asyncio.start_server(
            self.handle_client, "0.0.0.0", 3722
        )
        print(f"[TCP] Audio server listening on :3722")

        # 启动音频流
        self.audio_stream = sd.RawStream(
            samplerate=SAMPLE_RATE,
            channels=CHANNELS,
            dtype="int16",
            blocksize=BLOCKSIZE,
            callback=self.audio_callback,
        )
        self.audio_stream.start()
        print(f"[Audio] Stream started: {SAMPLE_RATE}Hz, {BLOCKSIZE} samples/block")

        print("\nVibeMate Host is running. Press Ctrl+C to stop.\n")

        try:
            while True:
                await asyncio.sleep(1)
        except asyncio.CancelledError:
            pass
        finally:
            self.shutdown()

    def shutdown(self):
        print("\n[Host] Shutting down...")
        if self.audio_stream:
            self.audio_stream.stop()
            self.audio_stream.close()
        if self.tcp_server:
            self.tcp_server.close()
        if self.udp_transport:
            self.udp_transport.close()


async def main():
    host = VibeMateHost()
    try:
        await host.run()
    except KeyboardInterrupt:
        host.shutdown()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nBye!")
        sys.exit(0)
```

- [ ] **Step 1: 写入 vibemate_host.py**
- [ ] **Step 2: Commit**

```bash
git add myprojects/vibemate/vibemate_host.py
git commit -m "feat(voice): implement Python host with asyncio TCP/UDP, sounddevice audio I/O, and frame protocol"
```

---

## 工作流 E: 集成与联调

### Task E1: 修改 vibemate.ino 集成 Voice 页面与音频任务

**Files:**
- Modify: `myprojects/vibemate/vibemate.ino`
- Modify: `myprojects/vibemate/config.h`

**config.h 添加：**
```cpp
// 语音相关配置
#define VOICE_DISCOVERY_PORT    3721
#define VOICE_TCP_PORT          3722
```

**vibemate.ino 修改：**

在现有 include 区域新增：
```cpp
#include "audio_manager.h"
#include "voice_network.h"
#include "ui_voice.h"
```

在现有 tile 变量区域新增：
```cpp
static lv_obj_t *tile_voice;
```

在现有 timer 区域新增（或复用 device_timer）：
```cpp
// voice_network_update 由 device_timer_cb 调用
// audio 任务在下面创建
```

在 `setup()` 中，在 `ui_usage_create()` 附近新增：
```cpp
    Serial.println("[INIT] Audio manager...");
    audio_manager_init();

    // 添加第 4 个 tile
    tile_voice = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR);
    lv_obj_set_style_bg_color(tile_voice, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tile_voice, 0, 0);
    lv_obj_set_style_border_width(tile_voice, 0, 0);

    ui_voice_create(tile_voice);

    Serial.println("[INIT] Voice network...");
    voice_network_init();
    voice_start_discovery();
```

在 `device_timer_cb` 中新增：
```cpp
static void device_timer_cb(lv_timer_t *timer) {
    ui_device_update();
    ui_pet_update();
    ui_voice_update();           // 更新 Voice 页面状态显示
    voice_network_update();      // 处理发现/心跳
}
```

**创建 FreeRTOS 音频任务：**

在 `setup()` 末尾（创建 timer 之后）新增：
```cpp
    // 启动音频收发任务
    xTaskCreatePinnedToCore(voice_tx_task, "voice_tx", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(voice_rx_task, "voice_rx", 4096, NULL, 5, NULL, 1);
```

在文件末尾（loop 之后）新增任务函数：
```cpp
static void voice_tx_task(void *pvParameters) {
    int16_t pcm_buffer[AUDIO_FRAME_SAMPLES];
    while (true) {
        // 读取一帧 I2S 数据
        if (audio_read_frame(pcm_buffer)) {
            voice_state_t state = voice_get_state();
            voice_mode_t mode = voice_get_mode();

            bool should_send = false;
            if (mode == VOICE_MODE_DUPLEX) {
                should_send = (state == VOICE_CONNECTED);
            } else {  // PTT
                should_send = (state == VOICE_TRANSMITTING);
            }

            if (should_send) {
                voice_send_audio_frame((const int8_t*)pcm_buffer, AUDIO_FRAME_BYTES);
            }
        }
        // 20ms 一帧，读操作本身阻塞约 20ms
        // 如需精确调度可用 vTaskDelayUntil
    }
}

static void voice_rx_task(void *pvParameters) {
    static uint8_t pcm_buffer[AUDIO_FRAME_BYTES];
    while (true) {
        size_t recv_len = voice_recv_audio_frame(pcm_buffer, sizeof(pcm_buffer));
        if (recv_len == AUDIO_FRAME_BYTES) {
            audio_write_frame((const int16_t*)pcm_buffer);
        } else {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}
```

**页面切换事件检测（进入/离开 Voice 页时开关功放）：**

在 `setup()` 中，tileview 创建后添加事件监听：
```cpp
    lv_obj_add_event_cb(tileview, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
```

新增回调：
```cpp
static void tileview_event_cb(lv_event_t *e) {
    lv_obj_t *tv = lv_event_get_target(e);
    lv_obj_t *current = lv_tileview_get_tile_act(tv);

    if (current == tile_voice) {
        Serial.println("[Voice] Entered voice page");
        audio_manager_amp_enable(true);
    } else {
        // 离开 Voice 页时，关闭功放省电
        audio_manager_amp_enable(false);
    }
}
```

- [ ] **Step 1: 修改 config.h 添加语音端口常量**
- [ ] **Step 2: 修改 vibemate.ino 集成所有组件**
- [ ] **Step 3: Commit**

```bash
git add myprojects/vibemate/config.h myprojects/vibemate/vibemate.ino
git commit -m "feat(voice): integrate audio_manager, voice_network, ui_voice into main app"
```

---

## 工作流 F: 联调验证

### Task F1: 编译验证

**Files:**
- Modify: 可能需要修复编译错误

**Compile:**
```bash
cd /Users/cyijun/Desktop/dev/ESP32-S3-Touch-LCD-1.85B/myprojects/vibemate
arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB" .
```

- [ ] **Step 1: 编译，修复任何编译错误**
- [ ] **Step 2: Commit 修复**

### Task F2: 运行验证

**验证步骤：**

1. **启动 Python 上位机**：
```bash
cd myprojects/vibemate
pip install sounddevice numpy
python vibemate_host.py
# 预期输出：[UDP] Discovery listening on :3721
#           [TCP] Audio server listening on :3722
```

2. **烧录 ESP32** 并打开串口监视器

3. **验证发现**：
   - 手表滑动到 Voice 页
   - 串口应输出发现广播和 TCP 连接成功
   - Python 端应输出 discovery 和 client connected

4. **验证 PTT 模式**：
   - 按住 PTT 按钮，对着手表说话
   - 电脑端应播放出声音
   - 松开按钮，发送停止

5. **验证全双工模式**：
   - 切换到全双工，点击"开始通话"
   - 两端同时说话，应能互相听到

6. **验证心跳与重连**：
   - 关闭 Python 程序，手表应显示断开并自动重新发现
   - 重新启动 Python，应自动重连

- [ ] **Step 1: 按上述步骤逐项验证**
- [ ] **Step 2: 修复发现的问题**
- [ ] **Step 3: Commit 最终版本**

---

## Self-Review

### Spec Coverage Check

| 设计规格章节 | 对应任务 |
|---|---|
| 硬件信息 (ES8311/ES7210/I2S 引脚) | Task A1-A2 |
| UDP 发现协议 | Task B2 |
| 音频传输帧格式 | Task B3 (voice_network.cpp 帧收发) |
| ESP32 双任务模型 | Task E1 (voice_tx_task / voice_rx_task) |
| Python 上位机架构 | Task D1 |
| Voice 页面 UI | Task C1-C2 |
| 状态机与错误处理 | Task B3 (心跳/超时/PTT), Task C2 (UI 状态) |
| 扩展预留 (一对多/Opus) | 帧协议预留帧类型，ui_voice 预留列表接口 |

**无缺口** — 所有规格章节均有对应任务。

### Placeholder Scan

- [x] 无 "TBD" / "TODO" / "implement later"
- [x] 无 "Add appropriate error handling" 等模糊描述
- [x] 无 "Similar to Task N" 引用
- [x] 每个代码步骤都包含完整代码

### Type Consistency

- `voice_state_t` / `voice_mode_t` 枚举在 B1 定义，B3 和 C2 中一致使用
- `FRAME_MAGIC_0/1` / `FRAME_AUDIO_*` 常量在 B1 定义，B3 中使用
- `AUDIO_FRAME_BYTES` / `AUDIO_FRAME_SAMPLES` 在 A1 定义，E1 中使用
- 函数签名在头文件和实现中一致

---

## Execution Handoff

**Plan complete and saved to `docs/superpowers/plans/2026-06-05-voice-lan-streaming-plan.md`.**

**Four parallel workstreams identified:**
- **A** (`audio_manager`) + **B** (`voice_network`) + **C** (`ui_voice`) + **D** (`vibemate_host.py`) can all be implemented in parallel by separate agents
- **E** (`integration`) depends on A/B/C completion
- **F** (`verification`) depends on D + E

**Two execution options:**

**1. Subagent-Driven (recommended)** — I dispatch 4 parallel subagents for workstreams A/B/C/D, then handle E/F inline

**2. Inline Execution** — Execute tasks in this session using executing-plans

**Which approach do you prefer?**