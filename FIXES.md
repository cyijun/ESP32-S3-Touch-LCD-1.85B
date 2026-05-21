# 编译问题修复记录

> 记录时间: 2026-05-22
> 环境: macOS, arduino-cli 1.5.0, ESP32 Arduino Core 3.3.8

---

## 修复汇总

| 示例 | 问题 | 修复方案 | 状态 |
|------|------|---------|------|
| `01_lvgl_demo` | `demos/lv_demos.h: No such file` | 复制 LVGL demos 目录 | ✅ 已修复 |
| `02_lvgl_BQ27220` | `demos/lv_demos.h: No such file` | 复制 LVGL demos 目录 | ✅ 已修复 |
| `05_audio_out_tf` | `Sketch too big` (144%) | 使用 `huge_app` 分区表 | ✅ 已修复 |
| `06_esp_sr` | `too many initializers for sr_cmd_t` | 去掉 phonetic 字段 | ✅ 已修复 |

---

## 1. LVGL demos 头文件找不到

**影响示例**: `01_lvgl_demo`, `02_lvgl_BQ27220`

**错误信息:**
```
fatal error: demos/lv_demos.h: No such file or directory
    5 | #include <demos/lv_demos.h>
```

**原因**: Arduino 编译器不会自动包含库根目录下的子目录。LVGL 的 `demos/` 和 `examples/` 位于库根目录，不在 `src/` 内，因此 `#include <demos/lv_demos.h>` 找不到文件。

**修复步骤**:

将 LVGL 库中的 demos 和 examples 复制到 `src/` 目录下：

```bash
cp -r ~/Documents/Arduino/libraries/lvgl/demos \
      ~/Documents/Arduino/libraries/lvgl/src/demos

cp -r ~/Documents/Arduino/libraries/lvgl/examples \
      ~/Documents/Arduino/libraries/lvgl/src/examples
```

修复后的目录结构：
```
libraries/lvgl/
├── src/
│   ├── core/
│   ├── demos/          <-- 复制到这里
│   ├── examples/       <-- 复制到这里
│   └── ...
├── demos/              <-- 原始位置
└── examples/           <-- 原始位置
```

> 这是 LVGL 官方文档中针对 Arduino 构建系统的已知限制和推荐做法。

---

## 2. 固件体积超限

**影响示例**: `05_audio_out_tf`, `06_esp_sr`

**错误信息:**
```
Sketch too big; see https://support.arduino.cc/...
text section exceeds available space in board
```

**原因**: 默认 `default` 分区表只给 App 分配 **1.3MB** (1,310,720 bytes)。音频和语音识别库体积很大，叠加后超过上限。

**修复步骤**:

在 FQBN 中指定 `PartitionScheme=huge_app`，将 App 分区扩大到 **3MB**。

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3:PartitionScheme=huge_app ...
```

修复前后对比：

| 示例 | 修复前 | 修复后 |
|------|--------|--------|
| `05_audio_out_tf` | 1,896KB (144% of 1.3MB) ❌ | 1,896KB (60% of 3MB) ✅ |
| `06_esp_sr` | 1,435KB (109% of 1.3MB) ❌ | 1,435KB (45% of 3MB) ✅ |

---

## 3. ESP_SR `sr_cmd_t` 结构体不兼容

**影响示例**: `06_esp_sr`

**错误信息:**
```
error: too many initializers for 'const sr_cmd_t'
   45 | };
      | ^
```

**原因**: ESP32 Arduino Core 3.3.8 中 `sr_cmd_t` 从 3 个字段变为 2 个字段：

```c
// 旧版 (示例代码假设的)
typedef struct {
  int command_id;
  char str[256];
  char phonetic[256];   // <-- 新版已移除
} sr_cmd_t;

// 新版 (Core 3.3.8 实际定义)
typedef struct sr_cmd_t {
  int command_id;
  char str[SR_CMD_STR_LEN_MAX];
} sr_cmd_t;
```

**修复步骤**:

删除 `06_esp_sr.ino` 中所有 `sr_cmd_t` 初始化器的第三个字段（phonetic 字符串）。

修改前：
```cpp
static const sr_cmd_t sr_commands[] = {
  { 0, "Turn on the light", "TkN nN jc LiT" },
  { 0, "Switch on the light", "SWgp nN jc LiT" },
  ...
};
```

修改后：
```cpp
static const sr_cmd_t sr_commands[] = {
  { 0, "Turn on the light" },
  { 0, "Switch on the light" },
  ...
};
```

> 已提交到仓库: `06_esp_sr.ino`

---

## 编译验证结果

修复后全部 8 个示例编译通过：

| 示例 | 状态 | 固件大小 |
|------|------|---------|
| `01_lvgl_demo` | ✅ | 687KB (52%) |
| `02_lvgl_BQ27220` | ✅ | 584KB (44%) |
| `03_audio_out_no_tf` | ✅ | 1,063KB (81%) |
| `04_SDMMC_Test` | ✅ | 368KB (28%) |
| `05_audio_out_tf` | ✅ | 1,896KB (60% of 3MB) |
| `06_esp_sr` | ✅ | 1,435KB (45% of 3MB) |
| `07_I2C_qmi8658` | ✅ | 300KB (22%) |
| `08_I2C_pcf85063` | ✅ | 343KB (26%) |

---

*本文档记录了从厂商示例到当前开发环境的适配过程。*
