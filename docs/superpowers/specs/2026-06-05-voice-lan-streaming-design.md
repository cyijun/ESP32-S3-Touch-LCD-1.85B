# VibeMate 局域网语音流式设计规格

## 1. 概述

为 VibeMate 手表添加局域网语音输入输出功能，通过板载 **ES8311（DAC 播放）+ ES7210（ADC 录音）** 音频编解码芯片，与局域网内的 Python 上位机建立流式音频通信。支持 **PTT（按住说话）** 和 **全双工通话** 两种模式切换，并具备局域网自动发现能力。

## 2. 硬件信息

| 组件 | 芯片 | 接口 | 地址 | 说明 |
|------|------|------|------|------|
| 音频 DAC | ES8311 | I2C + I2S | I2C 动态地址 | 播放输出 |
| 音频 ADC | ES7210 | I2C + I2S | `0x40` | 麦克风录音 |
| 功放使能 | — | GPIO | GPIO 9 | 高电平有效 |

**I2S 引脚定义**：

| 信号 | GPIO |
|------|------|
| I2S_MCK | 2 |
| I2S_BCK | 48 |
| I2S_LRCK (WS) | 38 |
| I2S_DOUT | 47 |
| I2S_DIN | 39 |

**音频参数**：16kHz 采样率 / 16bit 位宽 / 单声道（与现有 ES7210 示例一致）

## 3. 整体架构

```
┌─────────────────┐      UDP 广播发现       ┌─────────────────────┐
│   ESP32 手表     │  ───────────────────►  │  Python 上位机       │
│                 │     (端口 3721)         │                     │
│  ES7210(录音)   │                         │  asyncio TCP Server │
│  ES8311(播放)   │  ◄────────────────────► │  sounddevice I/O    │
│  Voice UI       │     TCP 长连接           │                     │
│                 │     (端口 3722)         │                     │
└─────────────────┘                         └─────────────────────┘
```

**通信协议栈**：
- **发现层**：UDP 广播/应答，JSON 格式
- **传输层**：TCP 长连接，自定义帧协议
- **音频层**：PCM 透传（16kHz/16bit/mono），20ms 一帧

**方案选择**：PCM 透传（方案 A）。局域网内 WiFi 带宽充足（双向约 512kbps），ESP32 端无需引入 Opus 编码器，开发简单稳定。下行预留了 `opus_decoder`（ESP32-audioI2S-master 自带），后续可升级为混合 Opus 方案。

## 4. 局域网发现协议

### 4.1 发现流程

1. Python 上位机启动后，监听 UDP **3721** 端口
2. 手表进入 Voice 页面时，每 2 秒广播一次 UDP 发现包
3. 上位机收到后回复 announce 包
4. 手表显示可连接的上位机列表（当前版本只显示第一个，预留一对多扩展）
5. 用户点击连接，或自动连接第一个发现的设备

### 4.2 UDP 报文格式

**发现请求**（手表广播）：
```json
{"type":"discover","device":"vibemate","version":1}
```

**Announce 回复**（上位机单播）：
```json
{"type":"announce","name":"MacBook-Pro","ip":"192.168.1.x","port":3722,"version":1}
```

## 5. 音频传输协议

### 5.1 帧结构

TCP 为流式协议，需帧边界区分音频块和控制消息：

```
┌─────────┬─────────┬─────────────┬─────────┐
│  Magic  │  Type   │ Payload Len │ Payload │
│ 2 bytes │ 1 byte  │  2 bytes    │ N bytes │
│0x56 0x4D│         │  大端uint16 │         │
│  ("VM") │         │             │         │
└─────────┴─────────┴─────────────┴─────────┘
```

### 5.2 帧类型

| 值 | 常量名 | 说明 |
|---|---|---|
| `0x01` | `AUDIO_UPLINK` | 手表→电脑的 PCM 音频数据 |
| `0x02` | `AUDIO_DOWNLINK` | 电脑→手表的 PCM 音频数据 |
| `0x03` | `CONTROL` | JSON 控制/状态消息 |

### 5.3 音频帧载荷

- 每 **20ms** 一帧
- 16kHz × 0.02s × 2 bytes = **640 bytes PCM payload**
- 帧头 5 bytes + 640 bytes = 每帧 645 bytes
- 对应码率：645 × 50 × 8 = **258 kbps**

### 5.4 控制帧 JSON 载荷

```json
// 连接握手
{"cmd":"hello","version":1,"mode":"ptt"}
{"cmd":"hello_ack","status":"ready","sample_rate":16000}

// 模式切换
{"cmd":"mode","type":"ptt"}
{"cmd":"mode","type":"duplex"}

// PTT 状态
{"cmd":"ptt","state":"pressed"}
{"cmd":"ptt","state":"released"}

// 心跳
{"cmd":"ping"}
{"cmd":"pong"}

// 状态报告
{"cmd":"status","rssi":-42,"buffer_level":3}
```

### 5.5 连接生命周期

1. `DISCOVERING` → UDP 广播发现上位机
2. `CONNECTING` → TCP `connect()` 到上位机 3722 端口
3. `HANDSHAKE` → 互发 `hello` 控制帧，协商版本和初始模式
4. `CONNECTED` → 开始音频流传输
5. 心跳：每 3 秒 `ping`，10 秒未收到 `pong` 判定断开
6. 断开时自动回到 `DISCOVERING` 状态

## 6. ESP32 端设计

### 6.1 新增模块

| 文件 | 职责 |
|---|---|
| `audio_manager.cpp/h` | I2S 初始化（ESP_I2S）、ES7210/ES8311 编解码器配置、功放 GPIO 9 控制、音频缓冲区管理 |
| `voice_network.cpp/h` | UDP 发现广播、TCP 连接管理、帧打包/解包、心跳维护、连接状态机 |
| `ui_voice.cpp/h` | Voice 页面 UI（第 4 页）、模式切换控件、PTT 按钮、连接状态显示 |

### 6.2 任务架构

ESP32-S3 双核分配：
- **Core 0**：WiFi/BT 协议栈（Arduino 默认）
- **Core 1**：应用程序 + LVGL + 音频任务

**三个 FreeRTOS 任务**：

| 任务名 | 核心 | 优先级 | 职责 |
|---|---|---|---|
| `voice_tx_task` | Core 1 | 5 | 从 I2S 读取 PCM → 打包 → TCP 发送 |
| `voice_rx_task` | Core 1 | 5 | TCP 接收 → 解包 → 写入 I2S 播放 |
| `voice_discovery_task` | Core 1 | 2 | 周期性 UDP 广播发现 / 心跳维护 |

**音频缓冲区**：
- `tx_ringbuf`：环形缓冲区，约 8 帧（≈ 5KB），`voice_tx_task` 消费
- `rx_ringbuf`：环形缓冲区，约 8 帧（≈ 5KB），`voice_rx_task` 生产

### 6.3 音频任务状态机

| 状态 | PTT 模式行为 | 全双工模式行为 |
|---|---|---|
| `IDLE` | 未连接 | 未连接 |
| `DISCOVERING` | UDP 广播发现中 | 同左 |
| `CONNECTING` | TCP 握手 | 同左 |
| `CONNECTED` | 等待用户按住 PTT 按钮 | `voice_tx_task` + `voice_rx_task` 同时持续运行 |
| `TRANSMITTING` | 按住中：`voice_tx_task` 活跃发送 | — |
| `RECEIVING` | 电脑端发送音频时：`voice_rx_task` 写入 I2S | `voice_rx_task` 持续接收播放 |

### 6.4 关键设计决策

- **I2S 始终运行**：进入 Voice 页面即启动 I2S 读写，避免频繁开关导致的 pop 噪声
- **PTT 控制发送而非 I2S**：按住按钮只是让 `voice_tx_task` 发送数据，I2S 持续读取（未发送时丢弃），松手无延迟
- **功放 GPIO 9**：进入 Voice 页面时 `HIGH`，离开时 `LOW`，与现有 `Backlight_Init` 模式一致
- **任务优先级 5**：高于 LVGL (2)，确保音频不卡顿

### 6.5 与现有代码集成

```cpp
// vibemate.ino
// 1. 新增 tile
lv_obj_t *tile_voice = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR);
lv_obj_set_style_bg_color(tile_voice, lv_color_hex(0x0a0e17), 0);
ui_voice_create(tile_voice);

// 2. 在 setup() 中初始化
audio_manager_init();
voice_network_init();

// 3. 在 device_timer_cb 中增加
voice_network_update();  // 处理状态机、心跳超时

// 4. 在 lv_tileview 滑动事件中检测是否进入/离开 Voice 页
//    进入时开启功放和 I2S，离开时关闭
```

## 7. Python 上位机设计

### 7.1 架构

```python
# vibemate_host.py
import asyncio
import sounddevice as sd
import numpy as np

class VibeMateHost:
    def __init__(self):
        self.udp_transport = None    # UDP 发现服务 (:3721)
        self.tcp_server = None       # TCP 音频服务 (:3722)
        self.client_writer = None    # 当前连接的客户端
        self.mode = "ptt"            # "ptt" or "duplex"
        self.rx_buffer = AudioRingBuffer()  # 接收自手表的音频
        self.tx_buffer = AudioRingBuffer()  # 待发送给手表的音频

    async def run(self):
        # 启动 UDP 发现
        self.udp_transport, _ = await loop.create_datagram_endpoint(
            lambda: UDPDiscoveryProtocol(self), local_addr=("0.0.0.0", 3721)
        )
        # 启动 TCP 服务
        self.tcp_server = await asyncio.start_server(
            self.handle_client, "0.0.0.0", 3722
        )
        # 启动音频流
        self.stream = sd.RawStream(
            samplerate=16000, channels=1, dtype="int16",
            blocksize=320,  # 20ms
            callback=self.audio_callback
        )
        self.stream.start()
```

### 7.2 音频回调

```python
def audio_callback(self, indata, outdata, frames, time, status):
    # 播放来自手表的音频
    pcm_rx = self.rx_buffer.get(frames)
    outdata[:] = pcm_rx if pcm_rx is not None else np.zeros((frames, 1), dtype="int16")

    # 全双工时：把麦克风采集的数据发送给手表
    if self.mode == "duplex" and self.client_writer:
        self.tx_buffer.put(indata.copy())

    # PTT 时：不采集本地麦克风（或根据需求采集）
```

### 7.3 帧解析循环

```python
async def handle_client(self, reader, writer):
    self.client_writer = writer
    try:
        while True:
            header = await reader.readexactly(5)
            magic = header[0:2]
            if magic != b"VM":
                continue  # 帧同步丢失，跳过直到找到 Magic
            frame_type = header[2]
            payload_len = int.from_bytes(header[3:5], "big")
            payload = await reader.readexactly(payload_len)

            if frame_type == 0x01:    # AUDIO_UPLINK
                self.rx_buffer.put(np.frombuffer(payload, dtype="int16"))
            elif frame_type == 0x02:  # AUDIO_DOWNLINK
                pass  # 不应收到
            elif frame_type == 0x03:  # CONTROL
                await self.handle_control(json.loads(payload.decode()))
    except asyncio.IncompleteReadError:
        pass  # 客户端断开
    finally:
        self.client_writer = None
```

### 7.4 依赖与启动

```bash
pip install sounddevice numpy
python vibemate_host.py
# 输出: [UDP] Discovery listening on :3721
#        [TCP] Audio server listening on :3722
```

## 8. Voice 页面 UI 设计

### 8.1 布局（360×360 圆屏）

```
┌──────────────────────────────┐
│  ● 连接状态    192.168.1.12  │  ← 顶部：状态灯(绿/黄/红) + IP/名称
│                              │
│    [ PTT ]  [ 全双工 ]       │  ← 模式切换按钮组（二选一高亮）
│                              │
│         ┌────────┐           │
│         │  🎤    │           │  ← PTT 模式：大圆按钮"按住说话"
│         │ 按住   │           │    全双工模式：波形动画 + "通话中"
│         │ 说话   │           │
│         └────────┘           │
│                              │
│    ▓▓▓▓░░░░░░░░░░           │  ← 实时音量/电平指示条
│                              │
│    [ 🔇 ]    [ ❌ 断开 ]     │  ← 底部：静音开关 + 断开连接
└──────────────────────────────┘
```

### 8.2 控件说明

| 控件 | 说明 |
|---|---|
| 状态灯 | 绿色=已连接，黄色=发现中/可连接，红色=断开 |
| 模式按钮 | PTT / 全双工 二选一，高亮当前模式 |
| PTT 大按钮 | 直径约 160px 圆形按钮，`PRESSING` 时变亮并显示"发送中..."，`RELEASED` 恢复 |
| 全双工显示 | "开始通话"按钮 → 点击后变"结束通话"，同时显示音频波形动画 |
| 音量条 | 显示近 20 帧的音频电平历史 |
| 静音 | 本地静音开关（控制 I2S 播放） |
| 断开 | 主动断开 TCP 连接 |

### 8.3 颜色与样式

与现有 UI 保持一致：
- 背景：`0x0a0e17`
- 主按钮按下：`0x4fc3f7`（青色）
- 全双工激活：`0x4ade80`（绿色）
- 状态灯：绿 `0x4ade80` / 黄 `0xffa726` / 红 `0xef4444`
- 文字：`0xf0f4f8`（主文字）/ `0x94a3b8`（次要文字）
- 字体：`lv_font_montserrat_14/16` + `font_cjk_14`

## 9. 错误处理

| 场景 | 处理策略 |
|---|---|
| TCP 断开 | 自动回到 `DISCOVERING` 状态，重新 UDP 广播发现 |
| 心跳超时 | 10 秒未收到 `pong`，强制断开 TCP，重新发现 |
| I2S 读写错误 | 打印日志，尝试重新初始化 I2S，3 次失败后放弃 |
| WiFi 断开 | 暂停所有音频任务，等待 `network_check()` 重连成功后恢复 |
| 无上位机响应 | UDP 广播持续发送，最多显示"未发现设备"提示 |
| 帧同步丢失（Magic 不匹配） | 逐字节滑动查找下一个 `0x56 0x4D`，最多丢弃 1KB 数据 |

## 10. 扩展预留

### 10.1 一对多上位机

当前设计只连接第一个发现的设备，但协议和 UI 已预留：
- UDP 发现可收集多个 announce 回复
- `ui_voice` 可将 IP 显示改为下拉列表
- TCP 连接逻辑只需改为选择指定 IP

### 10.2 升级到 Opus 编解码

当前为 PCM 透传，后续可在不改动架构的前提下升级：
- **下行**：电脑端 Opus 编码 → ESP32 `opus_decoder` 解码 → ES8311 播放（解码器已有）
- **上行**：引入 ESP32 Opus 编码器 → 传输 Opus 帧 → 电脑端解码
- 帧协议无需改动，只需新增 `AUDIO_OPUS_UPLINK` / `AUDIO_OPUS_DOWNLINK` 帧类型

### 10.3 与 AI 语音助手集成

Python 上位机可扩展：
- 接收手表语音 → ASR 转文字 → LLM 处理 → TTS 生成语音 → 发送回手表
- 保持现有 TCP 帧协议不变，上位机内部处理 ASR/TTS/LLM 链路

## 11. 文件变更清单

### 新增文件
- `myprojects/vibemate/audio_manager.cpp/h`
- `myprojects/vibemate/voice_network.cpp/h`
- `myprojects/vibemate/ui_voice.cpp/h`
- `myprojects/vibemate/vibemate_host.py`

### 修改文件
- `myprojects/vibemate/vibemate.ino`：添加 Voice tile、音频/网络初始化、页面切换事件
- `myprojects/vibemate/config.h`：添加语音相关配置（端口、采样率等）
