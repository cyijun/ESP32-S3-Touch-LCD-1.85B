# Buddy 桌宠全面 redesign 设计文档

## 概述

将 `myprojects/vibemate/` 现有桌宠实现对齐 `buddy_proto_new/` HTML 原型的新设计。全面移植新 UI、交互机制、养成系统和状态持久化，充分利用 ESP32-S3 的 16MB Flash 与 8MB PSRAM。

---

## 页面结构与导航

现有的 4 页 `lv_tileview` 扩展为 5 页，横向滑动：

| 坐标 | 页面 | 说明 |
|------|------|------|
| `(0, 0)` | **Pet Select** | 新增 — 伙伴选择与自定义 |
| `(1, 0)` | **Pet Detail** | 重写 — 属性面板（弧形进度条 + 元数据） |
| `(2, 0)` | **Pet** | 重写 — 交互主屏（精灵 + 动作按钮 + 状态条） |
| `(3, 0)` | **Usage** | 保留现有 Kimi Coding Plan 用量页 |
| `(4, 0)` | **Device** | 保留现有电池/RTC 设备页 |

- 启动时默认进入 **Pet 页**（交互主屏）
- **无页面点状指示器**

---

## 数据模型

`pet_sprites.h` 中的 `PetData` 扩展：

```c
typedef enum {
    PET_DUCK = 0, PET_GOOSE, PET_BLOB, PET_CAT, PET_DRAGON,
    PET_OCTOPUS, PET_OWL, PET_PENGUIN, PET_TURTLE, PET_SNAIL,
    PET_GHOST, PET_AXOLOTL, PET_CAPYBARA, PET_CACTUS, PET_ROBOT,
    PET_RABBIT, PET_MUSHROOM, PET_CHONK,
    PET_SPECIES_COUNT
} PetSpecies;

typedef enum {
    EYE_DOT = 0,    // '·'
    EYE_STAR,       // '✦'
    EYE_X,          // '×'
    EYE_CIRCLE,     // '◉'
    EYE_AT,         // '@'
    EYE_DEGREE,     // '°'
    EYE_COUNT
} PetEye;

typedef enum {
    HAT_NONE = 0, HAT_CROWN, HAT_TOPHAT, HAT_PROPELLER,
    HAT_HALO, HAT_WIZARD, HAT_BEANIE, HAT_TINYDUCK,
    HAT_COUNT
} PetHat;

typedef enum {
    COLOR_CYAN = 0, COLOR_PINK, COLOR_GOLD, COLOR_PURPLE,
    COLOR_GREEN, COLOR_BLUE, COLOR_ORANGE, COLOR_WHITE,
    COLOR_COUNT
} PetColor;

typedef struct {
    PetSpecies species;
    PetEye eye;
    PetHat hat;
    PetColor color;
    PetRarity rarity;
    bool shiny;
    uint8_t stats[PET_STAT_COUNT];  // DEBUG/PATIENCE/CHAOS/WISDOM/SNARK
    uint8_t hunger;                 // 0-100
    uint8_t joy;                    // 0-100
    char name[16];
} PetData;
```

### NVS 持久化（`pet_storage.cpp/h`）

- 使用 `Preferences` 库，key 前缀 `buddy_`
- 启动时 `pet_load()` 读取，首次运行则生成默认宠物
- 任何状态变更（喂食、玩耍、换帽、选择伙伴）后立即 `pet_save()`
- `hunger`/`joy` 每 60 秒衰减后也保存

---

## 交互主屏（Pet 页）

从上到下布局：

### 顶部信息区
- 宠物名称：15px 加粗白色，`LV_ALIGN_TOP_MID, 0, 28`
- 稀有度星星：9px，根据稀有度着色（Common 灰 / Uncommon 青 / Rare 蓝 / Epic 紫 / Legendary 金）

### Mini 状态条
- 两个水平进度条（`lv_bar`）：◐ 饱食度 / ♥ 开心值
- 宽度 220px，高度 4px，圆角，强调色 `#3DD9D0` 填充
- 数值显示在条右侧（9px 等宽字体）

### 宠物舞台（居中）
- 装饰圆环：`lv_arc` 或自定义绘制，220px 细圆环，带脉冲动画
- 宠物精灵：`lv_label`，等宽字体 11px，4 行紧凑 ASCII
- 眼睛字符用强调色 `#3DD9D0`；`shiny` 时加发光效果
- 帽子：若不为 `none`，在精灵顶部叠加一行帽子 ASCII
- **浮空动画**：`lv_timer` 每 40ms 更新精灵 y 偏移，正弦波 ±4px，周期 2.5s

### 气泡对话框
- 圆角矩形容器（`lv_obj` + `lv_label`），默认隐藏
- 对话/喂食/玩耍各有独立消息池（中文）
- 触发时弹性弹出，3 秒后自动隐藏

### 底部动作栏
3 个圆形按钮（48px，圆角 50%，深色背景 + 细边框）：
- **左 — 喂食**：点击 → wiggle 动画 + 漂浮食物特效 + hunger +8 + 饱食反馈消息
- **中 — 对话**（强调色高亮）：点击 → 显示随机气泡消息（本地预置，不走 API）
- **右 — 玩耍**：点击 → jump 动画 + 漂浮球特效 + joy +10 + 圆环加速脉冲 0.9s + 开心反馈消息

### 长按交互
- 长按宠物精灵 600ms → 弹出帽子选择菜单（3×3 网格 + 取消按钮）
- 短按宠物精灵 → 触发对话

### 漂浮特效
- 食物 `●`、球 `◯`、数值 `+Hunger / +Joy`
- 临时 `lv_label`，`lv_timer` 驱动上移淡出，1s 后销毁

---

## 属性面板（Pet Detail 页）

### 顶部头像区
- 40px 圆形边框容器，内部显示迷你脸部表情（如 `=✦ω✦=`），10px 等宽强调色
- 下方名称：11px 加粗白色
- 稀有度徽章：小胶囊标签，强调色背景 + 文字

### 属性列表
5 个属性（DEBUGGING / PATIENCE / CHAOS / WISDOM / SNARK），每行：
- 左侧：`lv_arc`（直径 22px，线宽 2.5px），背景弧灰色，前景弧强调色带圆角端点，中心显示数值
- 右侧上方：属性名（7px 大写灰色）+ 数值（10px 等宽加粗）
- 右侧下方：线性进度条（`lv_bar`），高度 2px，强调色渐变，宽度动画 0.8s 展开

### 元数据网格
2×2 卡片布局（SPECIES / EYE / HAT / SHINY）：
- 深色圆角背景 + 细边框
- 上方标签 7px 灰色，下方值 9px 白色

---

## 伙伴选择（Pet Select 页）

### 顶部标题
- "选择伙伴"：13px 加粗
- 副标题 "自定义你的 Buddy"：8px 灰色大写

### 物种轮播
- 中央预览区（90px 舞台）：简化脸部表情，9px 等宽
- 预览圆环：78px 细圆环，颜色随所选颜色变化
- 帽子徽章：若选了帽子，在预览区顶部显示帽子符号（14px）
- 左右侧窥：两侧 32px 小圆，半透明显示相邻物种迷你图标
- 滑动/点击切换物种（18 种循环）

### 选择器行
三行水平滚动选择器：
- ◉ 眼睛：6 种，圆形选项按钮
- ▲ 帽子：8 种，符号显示
- ● 颜色：8 个色块，当前选中带白圈边框
- 选中项用强调色高亮

### 缩略图条
底部 5 个圆形小缩略（当前及前后各 2 个物种图标），当前项高亮

### 底部动作栏
- 左：上一个（28px 小圆按钮）
- 中：生成（44px 强调色按钮）— 基于当前物种做确定性随机，重新 roll 眼睛/帽子/颜色
- 右：下一个（28px 小圆按钮）

### 确认逻辑
- 选择完成后滑动离开即生效，数据写入 NVS
- 若已有保存的宠物，进入时自动恢复选择状态

---

## 动画与特效系统

| 动画 | 实现 |
|------|------|
| 精灵浮空 | `lv_timer` 每 40ms 更新 y 偏移，正弦波 ±4px，周期 2.5s |
| 圆环脉冲 | `lv_timer` 周期 3s，调整 scale（1.0→1.04）和 opa（0.4→0.7），玩耍时加速到 0.7s |
| Wiggle 摇晃 | `lv_timer` 每 30ms 更新 rotation：0°→-8°→8°→-4°→0°，0.45s |
| Jump 跳跃 | `lv_timer` 更新 y：0→-28→4→0，0.55s |
| 漂浮特效 | 临时 `lv_label`，`lv_timer` 驱动上移 + 淡出，1s 后删除 |
| 气泡弹出 | `scale(0)`→`scale(1)` 弹性缓出，3s 后隐藏 |
| 长按指示环 | `lv_timer` 每 16ms 缩小 scale（1.6→1.0）并提升 opa，600ms 满后触发菜单 |
| 帽子菜单 | 遮罩 + 面板，`opa` 0→255 过渡 200ms |

所有动画对象在页面销毁时统一清理。

---

## 状态衰减机制

- 新增 `lv_timer`，周期 60 秒
- `hunger` 和 `joy` 各 -1 / 分钟，下限 0
- 每次衰减后 `pet_save()`

**状态影响：**
- `hunger <= 20`：饥饿表情（眼睛变 `×` 或 `-`，停止浮空）
- `joy <= 20`：低落表情（停止浮动）
- `hunger == 0 && joy == 0`：休眠状态（显示 `zZ` 气泡，无交互反应）

**恢复：**
- 喂食 +8 hunger，玩耍 +10 joy，上限 100
- 从极低值恢复时立即恢复正常表情和动画

**首次启动：**
- NVS 无数据 → 基于 MAC 地址确定性生成默认伙伴，hunger=80, joy=70
- 有数据 → 直接恢复上次状态

---

## 颜色与主题

- 画布背景：`#0A0A0F`
- 强调色：`#3DD9D0`（青色）
- 表面色：`#13131A`
- 边框色：`#252530`
- 文字主色：`#E8E8ED`
- 文字次要色：`#6B6B78`
- 稀有度颜色：Common `#8A8A95` / Uncommon `#5EE7DF` / Rare `#6B8CFF` / Epic `#C85EFF` / Legendary `#FFD166`

---

## 文件变更清单

### 新增文件
- `ui_pet_select.h/cpp` — 伙伴选择页
- `pet_storage.h/cpp` — NVS 持久化

### 重写文件
- `pet_sprites.h/cpp` — 扩展数据模型、18 物种精灵模板、帽子线、颜色表
- `ui_pet.h/cpp` — 交互主屏全面重写
- `ui_pet_detail.h/cpp` — 属性面板全面重写

### 修改文件
- `vibemate.ino` — tileview 从 4 页扩为 5 页，调整启动默认页、增加 UI 更新逻辑
- `ui_usage.h/cpp` — 可能微调页面索引引用（如有硬编码）
- `ui_device.h/cpp` — 同上

---

## 非目标（明确不做）

- 对话按钮不走 Kimi API（本地预置消息）
- 不添加页面点状指示器
- 不在硬件上实现 swipe 手势（依赖 tileview 原生滑动）
