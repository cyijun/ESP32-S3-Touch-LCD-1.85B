# Multi-Key Kimi Usage Display — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend the Usage page to display multiple Kimi API keys in vertically-stacked tiles, with new keys appearing above the original key.

**Architecture:** Center-row shift: all non-usage tiles sit on `y = KIMI_ACCOUNT_COUNT - 1`; usage tiles span `y = 0` (newest) to `y = count-1` (oldest). A single FreeRTOS task requests keys serially with 500ms spacing. Each key has an independent `usage_page_t` UI.

**Tech Stack:** ESP32 Arduino core, LVGL 8.x, ArduinoJson, FreeRTOS

---

## File Structure

| File | Action | Responsibility |
|------|--------|----------------|
| `config.h` | Modify | Add `kimi_account_t` struct + `KIMI_ACCOUNTS[]` array |
| `kimi_api.h` | Modify | `g_kimi_data` → array; add `KIMI_MAX_ACCOUNTS`, `g_kimi_account_count` |
| `kimi_api.cpp` | Modify | `s_do_http_request` takes `api_key` param; serial loop in `s_api_task` |
| `ui_usage.h` | Modify | Add `account_index` parameter to create/update functions |
| `ui_usage.cpp` | Modify | `usage_page_t` struct array; per-index page creation and update |
| `vibemate.ino` | Modify | Dynamic `tile_usage[]`; center-y shift; updated event/timer handlers |

---

### Task 1: config.h — Multi-Key Account Configuration

**Files:**
- Modify: `config.h`

Add the `kimi_account_t` struct and `KIMI_ACCOUNTS[]` array after the existing API key macros. The array order: index 0 = oldest key (on main row), append new keys to the end (they display at the top).

- [ ] **Step 1: Add struct and array after the Kimi API key macros**

Insert after line 24 (after `KIMI_API_KEY_2` warning block, before the DeepSeek section):

```cpp
// Kimi 多账号配置
struct kimi_account_t {
    const char *name;   // 显示名称
    const char *key;    // API key（引用上面的宏）
};

// 数组顺序：index 0 = 最老的 key（默认显示在主行）
// 新增 key 时 append 到数组末尾，显示时会自动出现在最上方
static const kimi_account_t KIMI_ACCOUNTS[] = {
    { "主号", KIMI_API_KEY_1 },   // index 0，最老，显示在 y = count-1
    { "副号", KIMI_API_KEY_2 },   // index 1，较新，显示在 y = count-2
};
#define KIMI_ACCOUNT_COUNT (sizeof(KIMI_ACCOUNTS) / sizeof(KIMI_ACCOUNTS[0]))
```

- [ ] **Step 2: Verify config.h compiles as a standalone header**

No compile command needed yet (header only). Just verify no syntax errors.

- [ ] **Step 3: Commit**

```bash
git add config.h
git commit -m "feat(config): add multi-key Kimi account array"
```

---

### Task 2: kimi_api.h — Data Structure Array

**Files:**
- Modify: `kimi_api.h`

Change `g_kimi_data` from a single instance to an array, add `KIMI_MAX_ACCOUNTS` constant and `g_kimi_account_count`.

- [ ] **Step 1: Replace the data structure declarations**

Replace lines 29-32 with:

```cpp
#define KIMI_MAX_ACCOUNTS 8

// 全局数据数组（每个 key 一个 slot）
extern kimi_usage_t g_kimi_data[KIMI_MAX_ACCOUNTS];
extern int g_kimi_account_count;
extern bool g_ui_needs_update;
extern SemaphoreHandle_t kimi_mutex;
```

Function declarations remain unchanged:
```cpp
void kimi_api_init(void);
void kimi_api_refresh_now(void);
void kimi_api_timer_cb(lv_timer_t *timer);
```

- [ ] **Step 2: Commit**

```bash
git add kimi_api.h
git commit -m "feat(kimi_api): change g_kimi_data to array, add KIMI_MAX_ACCOUNTS"
```

---

### Task 3: kimi_api.cpp — Serial Multi-Key HTTP Requests

**Files:**
- Modify: `kimi_api.cpp`

Three changes: (a) `s_do_http_request` takes `api_key` parameter, (b) `s_api_task` loops through all keys serially, (c) `kimi_api_init` initializes the array.

- [ ] **Step 1: Change `s_do_http_request` signature**

Replace line 63:
```cpp
static void s_do_http_request(const char *api_key, kimi_usage_t *out)
```

Replace line 77 (the `http.addHeader` line):
```cpp
    http.addHeader("Authorization", String("Bearer ") + api_key);
```

- [ ] **Step 2: Rewrite `kimi_api_init`**

Replace lines 54-61:

```cpp
void kimi_api_init(void) {
    for (int i = 0; i < KIMI_MAX_ACCOUNTS; i++) {
        memset(&g_kimi_data[i], 0, sizeof(g_kimi_data[i]));
        g_kimi_data[i].api_ok = false;
        g_kimi_data[i].last_error = "";
        g_kimi_data[i].last_update_ms = 0;
    }
    g_kimi_account_count = KIMI_ACCOUNT_COUNT;
    g_ui_needs_update = false;
    kimi_mutex = xSemaphoreCreateMutex();
}
```

- [ ] **Step 3: Rewrite `s_api_task` for serial key fetching**

Replace lines 164-182 with:

```cpp
static void s_api_task(void *arg)
{
    (void)arg;
    TRACE_KIMI("API task start");

    for (int i = 0; i < g_kimi_account_count; i++) {
        kimi_usage_t result;
        s_do_http_request(KIMI_ACCOUNTS[i].key, &result);

        if (kimi_mutex && xSemaphoreTake(kimi_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            g_kimi_data[i] = result;
            xSemaphoreGive(kimi_mutex);
        }

        if (i < g_kimi_account_count - 1) {
            vTaskDelay(pdMS_TO_TICKS(500));  // 防限流
        }
    }

    g_ui_needs_update = true;
    s_request_busy = false;
    s_api_task_handle = NULL;
    vTaskDelete(NULL);
}
```

- [ ] **Step 4: Add config.h include if missing**

Verify `kimi_api.cpp` line 2 has `#include "config.h"` (it already does).

- [ ] **Step 5: Commit**

```bash
git add kimi_api.cpp kimi_api.h
git commit -m "feat(kimi_api): serial multi-key HTTP requests with rate-limit delay"
```

---

### Task 4: ui_usage.h — Add account_index Parameter

**Files:**
- Modify: `ui_usage.h`

- [ ] **Step 1: Update function signatures**

Replace the entire file content:

```cpp
#ifndef UI_USAGE_H
#define UI_USAGE_H

#include <lvgl.h>
#include "kimi_api.h"

void ui_usage_create(lv_obj_t *parent_tile, int account_index);
void ui_usage_update(int account_index, const kimi_usage_t *data);

#endif
```

- [ ] **Step 2: Commit**

```bash
git add ui_usage.h
git commit -m "feat(ui_usage): add account_index to create/update signatures"
```

---

### Task 5: ui_usage.cpp — Multi-Page UI Support

**Files:**
- Modify: `ui_usage.cpp`

Major refactor: replace individual static `lv_obj_t*` variables with a `usage_page_t` struct array. Each page is independently created and updated by `account_index`.

- [ ] **Step 1: Replace static variables with struct array**

Replace lines 1-20 with:

```cpp
#include "ui_usage.h"
#include "kimi_api.h"
#include "config.h"

struct usage_page_t {
    lv_obj_t *arc_week;
    lv_obj_t *arc_window;
    lv_obj_t *label_percent;
    lv_obj_t *label_center;
    lv_obj_t *label_window_pct;
    lv_obj_t *label_plan;
    lv_obj_t *label_tier;
    lv_obj_t *status_dot;
    lv_obj_t *label_legend_week;
    lv_obj_t *label_legend_window;
    lv_obj_t *dot_legend_week;
    lv_obj_t *dot_legend_window;
    float last_week_pct;
    float last_window_pct;
    bool has_data;
};

static usage_page_t s_pages[KIMI_MAX_ACCOUNTS];
```

- [ ] **Step 2: Rewrite `ui_usage_create`**

Replace the entire `ui_usage_create` function (lines 69-185) with:

```cpp
void ui_usage_create(lv_obj_t *parent_tile, int account_index) {
    if (account_index < 0 || account_index >= KIMI_MAX_ACCOUNTS) return;

    usage_page_t *p = &s_pages[account_index];
    p->last_week_pct = 0.0f;
    p->last_window_pct = 0.0f;
    p->has_data = false;

    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // --- Status dot (top-right) ---
    p->status_dot = lv_obj_create(parent_tile);
    lv_obj_set_size(p->status_dot, 8, 8);
    lv_obj_set_style_radius(p->status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(p->status_dot, lv_color_hex(0xef4444), 0);
    lv_obj_set_style_border_width(p->status_dot, 0, 0);
    lv_obj_set_style_pad_all(p->status_dot, 0, 0);
    lv_obj_align(p->status_dot, LV_ALIGN_TOP_MID, 85, 30);

    // --- Top title area ---
    p->label_plan = lv_label_create(parent_tile);
    lv_label_set_text(p->label_plan, KIMI_ACCOUNTS[account_index].name);
    lv_obj_set_style_text_color(p->label_plan, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(p->label_plan, &lv_font_montserrat_14, 0);
    lv_obj_align(p->label_plan, LV_ALIGN_TOP_MID, 0, 30);

    p->label_tier = lv_label_create(parent_tile);
    lv_label_set_text(p->label_tier, "--");
    lv_obj_set_style_text_color(p->label_tier, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(p->label_tier, &lv_font_montserrat_16, 0);
    lv_obj_align(p->label_tier, LV_ALIGN_TOP_MID, 0, 50);

    // --- Outer arc (week usage) ---
    p->arc_week = lv_arc_create(parent_tile);
    lv_obj_set_size(p->arc_week, 260, 260);
    lv_arc_set_rotation(p->arc_week, 270);
    lv_arc_set_bg_angles(p->arc_week, 0, 360);
    lv_arc_set_range(p->arc_week, 0, 100);
    lv_arc_set_value(p->arc_week, 0);
    lv_obj_set_style_arc_width(p->arc_week, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(p->arc_week, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(p->arc_week, lv_color_hex(0x4fc3f7), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(p->arc_week, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_remove_style(p->arc_week, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(p->arc_week, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(p->arc_week, LV_ALIGN_CENTER, 0, 0);

    // --- Inner arc (5h window) ---
    p->arc_window = lv_arc_create(parent_tile);
    lv_obj_set_size(p->arc_window, 200, 200);
    lv_arc_set_rotation(p->arc_window, 270);
    lv_arc_set_bg_angles(p->arc_window, 0, 360);
    lv_arc_set_range(p->arc_window, 0, 100);
    lv_arc_set_value(p->arc_window, 0);
    lv_obj_set_style_arc_width(p->arc_window, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(p->arc_window, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(p->arc_window, lv_color_hex(0xffa726), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(p->arc_window, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_remove_style(p->arc_window, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(p->arc_window, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(p->arc_window, LV_ALIGN_CENTER, 0, 0);

    // --- Center text ---
    p->label_percent = lv_label_create(parent_tile);
    lv_label_set_text(p->label_percent, "0%");
    lv_obj_set_style_text_color(p->label_percent, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(p->label_percent, &lv_font_montserrat_24, 0);
    lv_obj_align(p->label_percent, LV_ALIGN_CENTER, 0, -10);

    p->label_center = lv_label_create(parent_tile);
    lv_label_set_text(p->label_center, "周用量已用");
    lv_obj_set_style_text_color(p->label_center, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(p->label_center, &font_cjk_14, 0);
    lv_obj_align(p->label_center, LV_ALIGN_CENTER, 0, 18);

    p->label_window_pct = lv_label_create(parent_tile);
    lv_label_set_text(p->label_window_pct, "5h: --%");
    lv_obj_set_style_text_color(p->label_window_pct, lv_color_hex(0xffa726), 0);
    lv_obj_set_style_text_font(p->label_window_pct, &lv_font_montserrat_14, 0);
    lv_obj_align(p->label_window_pct, LV_ALIGN_CENTER, 0, 38);

    // --- Bottom legend ---
    p->dot_legend_week = lv_obj_create(parent_tile);
    lv_obj_set_size(p->dot_legend_week, 8, 8);
    lv_obj_set_style_radius(p->dot_legend_week, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(p->dot_legend_week, lv_color_hex(0x4fc3f7), 0);
    lv_obj_set_style_border_width(p->dot_legend_week, 0, 0);
    lv_obj_set_style_pad_all(p->dot_legend_week, 0, 0);
    lv_obj_align(p->dot_legend_week, LV_ALIGN_BOTTOM_MID, -40, -35);

    p->label_legend_week = lv_label_create(parent_tile);
    lv_label_set_text(p->label_legend_week, "7天");
    lv_obj_set_style_text_color(p->label_legend_week, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(p->label_legend_week, &font_cjk_14, 0);
    lv_obj_align_to(p->label_legend_week, p->dot_legend_week, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    p->dot_legend_window = lv_obj_create(parent_tile);
    lv_obj_set_size(p->dot_legend_window, 8, 8);
    lv_obj_set_style_radius(p->dot_legend_window, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(p->dot_legend_window, lv_color_hex(0xffa726), 0);
    lv_obj_set_style_border_width(p->dot_legend_window, 0, 0);
    lv_obj_set_style_pad_all(p->dot_legend_window, 0, 0);
    lv_obj_align(p->dot_legend_window, LV_ALIGN_BOTTOM_MID, 30, -35);

    p->label_legend_window = lv_label_create(parent_tile);
    lv_label_set_text(p->label_legend_window, "5小时");
    lv_obj_set_style_text_color(p->label_legend_window, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(p->label_legend_window, &font_cjk_14, 0);
    lv_obj_align_to(p->label_legend_window, p->dot_legend_window, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
}
```

- [ ] **Step 3: Rewrite `ui_usage_update`**

Replace the entire `ui_usage_update` function (lines 187-220) with:

```cpp
void ui_usage_update(int account_index, const kimi_usage_t *data) {
    if (!data || account_index < 0 || account_index >= KIMI_MAX_ACCOUNTS) return;

    usage_page_t *p = &s_pages[account_index];

    if (data->api_ok) {
        p->has_data = true;
        p->last_week_pct = data->week_pct;
        p->last_window_pct = data->window_pct;

        lv_arc_set_value(p->arc_week, (int32_t)data->week_pct);
        lv_arc_set_value(p->arc_window, (int32_t)data->window_pct);

        char buf[64];
        lv_snprintf(buf, sizeof(buf), "%d%%", (int)data->week_pct);
        lv_label_set_text(p->label_percent, buf);

        lv_label_set_text(p->label_tier, data->tier_name.c_str());

        lv_snprintf(buf, sizeof(buf), "5h: %d%%", (int)data->window_pct);
        lv_label_set_text(p->label_window_pct, buf);

        lv_obj_set_style_bg_color(p->status_dot, lv_color_hex(0x4ade80), 0);
    } else {
        if (p->has_data) {
            lv_obj_set_style_bg_color(p->status_dot, lv_color_hex(0xffa726), 0);
        } else {
            lv_obj_set_style_bg_color(p->status_dot, lv_color_hex(0xef4444), 0);
        }

        if (data->last_error.indexOf("401") >= 0 || data->last_error.indexOf("403") >= 0) {
            lv_label_set_text(p->label_percent, "Invalid Key");
            lv_obj_set_style_text_font(p->label_percent, &lv_font_montserrat_16, 0);
        }
    }
}
```

- [ ] **Step 4: Remove unused `tick_ring_draw_cb` if desired (optional cleanup)**

The `tick_ring_draw_cb` function (lines 27-67) is already commented out in usage. It can be left as-is or removed. Leaving it does no harm.

- [ ] **Step 5: Commit**

```bash
git add ui_usage.cpp ui_usage.h
git commit -m "feat(ui_usage): multi-page support with per-account UI structs"
```

---

### Task 6: vibemate.ino — Dynamic Tileview with Center-Row Shift

**Files:**
- Modify: `vibemate.ino`

Five changes: (a) replace single `tile_usage` with array, (b) compute `s_center_y`, (c) create usage tiles in a loop, (d) update event handler, (e) update UI timer for multi-key.

- [ ] **Step 1: Update static variables at the top**

Replace lines 21-26 with:

```cpp
static lv_obj_t *tileview;
static lv_obj_t *tile_pet_select;
static lv_obj_t *tile_pet_detail;
static lv_obj_t *tile_pet;
static lv_obj_t *tile_usage[KIMI_MAX_ACCOUNTS];
static lv_obj_t *tile_device;
static int s_usage_tile_count = 0;
static int s_center_y = 0;
```

Also add `#include "config.h"` at the top if not already present (it is, line 1).

- [ ] **Step 2: Remove old `api_timer_cb` (it's redundant)**

Lines 35-42 define `api_timer_cb` which just calls `kimi_api_refresh_now()`. This is identical to `kimi_api_timer_cb` in `kimi_api.cpp`. Remove `api_timer_cb` entirely and later use `kimi_api_timer_cb` directly. But **don't change the timer creation yet** (that comes in Step 5).

Actually, looking at the code more carefully: `api_timer_cb` in `vibemate.ino` calls `kimi_api_refresh_now()`, and `kimi_api_timer_cb` in `kimi_api.cpp` also just calls `kimi_api_refresh_now()`. They are duplicates. We can simplify by using `kimi_api_timer_cb` directly.

Replace lines 35-42 with: (delete them)

- [ ] **Step 3: Update `tileview_event_cb`**

Replace lines 44-69 with:

```cpp
static void tileview_event_cb(lv_event_t *e)
{
    (void)e;
    lv_obj_t *tv = lv_event_get_target(e);
    lv_obj_t *tile = lv_tileview_get_tile_act(tv);

    static lv_obj_t *s_last_reported_tile = NULL;
    if (tile == s_last_reported_tile) return;
    s_last_reported_tile = tile;

    int idx = -1;
    if (tile == tile_pet_select) idx = 0;
    else if (tile == tile_pet_detail) idx = 1;
    else if (tile == tile_pet) idx = 2;
    else if (tile == tile_device) idx = 3 + s_usage_tile_count;
    else {
        for (int i = 0; i < s_usage_tile_count; i++) {
            if (tile == tile_usage[i]) { idx = 3 + i; break; }
        }
    }
    TRACE_MAIN("tileview changed to tile=%d", idx);

    if (tile == tile_pet) {
        ui_pet_resume_anim();
    } else {
        ui_pet_pause_anim();
    }
}
```

- [ ] **Step 4: Update `ui_timer_cb` for multi-key**

Replace lines 81-92 with:

```cpp
static void ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (g_ui_needs_update && kimi_mutex &&
        xSemaphoreTake(kimi_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        g_ui_needs_update = false;

        int count = g_kimi_account_count;
        kimi_usage_t snapshots[KIMI_MAX_ACCOUNTS];
        for (int i = 0; i < count; i++) {
            snapshots[i] = g_kimi_data[i];
        }
        xSemaphoreGive(kimi_mutex);

        for (int i = 0; i < count; i++) {
            ui_usage_update(i, &snapshots[i]);
        }
    }
}
```

- [ ] **Step 5: Rewrite tile creation in `setup()`**

Replace lines 173-199 with:

```cpp
    s_usage_tile_count = KIMI_ACCOUNT_COUNT;
    s_center_y = KIMI_ACCOUNT_COUNT - 1;

    tile_pet_select = lv_tileview_add_tile(tileview, 0, s_center_y, LV_DIR_HOR);
    tile_pet_detail = lv_tileview_add_tile(tileview, 1, s_center_y, LV_DIR_HOR);
    tile_pet    = lv_tileview_add_tile(tileview, 2, s_center_y, LV_DIR_HOR);
    tile_device = lv_tileview_add_tile(tileview, 4, s_center_y, LV_DIR_HOR);

    for (int i = 0; i < s_usage_tile_count; i++) {
        int y = s_center_y - i;
        tile_usage[i] = lv_tileview_add_tile(tileview, 3, y, LV_DIR_ALL);
        lv_obj_set_style_bg_color(tile_usage[i], lv_color_hex(0x0a0e17), 0);
        lv_obj_set_style_pad_all(tile_usage[i], 0, 0);
        lv_obj_set_style_border_width(tile_usage[i], 0, 0);
        lv_obj_clear_flag(tile_usage[i], LV_OBJ_FLAG_SCROLLABLE);
    }
```

Then remove the old single-tile style setup block (lines 179-199 in original) since it's now handled in the loop.

Also update the `lv_obj_clear_flag` calls — remove the old `tile_usage` line and keep the others:

```cpp
    lv_obj_clear_flag(tile_pet_select, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_pet_detail, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_pet,    LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_device, LV_OBJ_FLAG_SCROLLABLE);
```

- [ ] **Step 6: Update UI creation calls in `setup()`**

The `ui_usage_create` call on line 218 needs the account index:

Replace line 218:
```cpp
    ui_usage_create(tile_usage, 0);
```

With a loop after the other UI creation calls:

```cpp
    for (int i = 0; i < s_usage_tile_count; i++) {
        ui_usage_create(tile_usage[i], i);
    }
```

Move this loop to replace line 218. The surrounding lines (212-220) become:

```cpp
    TRACE_MAIN("Creating ui_pet_select...");
    ui_pet_select_create(tile_pet_select);
    TRACE_MAIN("Creating ui_pet_detail...");
    ui_pet_detail_create(tile_pet_detail);
    TRACE_MAIN("Creating ui_pet...");
    ui_pet_create(tile_pet);
    TRACE_MAIN("Creating ui_usage...");
    for (int i = 0; i < s_usage_tile_count; i++) {
        ui_usage_create(tile_usage[i], i);
    }
    TRACE_MAIN("Creating ui_device...");
    ui_device_create(tile_device);
```

- [ ] **Step 7: Update timer creation**

Replace line 229:
```cpp
    api_timer = lv_timer_create(api_timer_cb, API_REFRESH_INTERVAL_MS, NULL);
```

With:
```cpp
    api_timer = lv_timer_create(kimi_api_timer_cb, API_REFRESH_INTERVAL_MS, NULL);
```

This uses the `kimi_api_timer_cb` from `kimi_api.cpp` instead of the now-removed local `api_timer_cb`.

- [ ] **Step 8: Commit**

```bash
git add vibemate.ino
git commit -m "feat(tileview): dynamic center-row shift for multi-key usage tiles"
```

---

### Task 7: Compile Verification

**Files:** None (verification only)

- [ ] **Step 1: Compile the project**

```bash
cd /Users/cyijun/Desktop/dev/ESP32-S3-Touch-LCD-1.85B/myprojects/vibemate
arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc" .
```

**Expected:** Clean compile with zero errors.

**Common issues to watch for:**
- `KIMI_ACCOUNTS` not found in `kimi_api.cpp` → missing `#include "config.h"`
- `KIMI_MAX_ACCOUNTS` not found in `ui_usage.cpp` → missing `#include "kimi_api.h"` (already there)
- `lv_obj_t *tile_usage` vs `lv_obj_t *tile_usage[]` type mismatch in old references

- [ ] **Step 2: Fix any compile errors**

If errors occur, fix and recompile. Repeat until clean.

- [ ] **Step 3: Final commit**

```bash
git add -A
git commit -m "feat(usage): multi-key Kimi usage display with center-row tileview"
```

---

## Self-Review Checklist

**Spec coverage:**
- [x] C struct array config (Task 1)
- [x] `g_kimi_data` array + `KIMI_MAX_ACCOUNTS` (Task 2)
- [x] Serial HTTP with 500ms delay (Task 3)
- [x] Per-account UI pages (Task 5)
- [x] Center-row shift tileview (Task 6)
- [x] Dynamic tile creation in loop (Task 6)
- [x] Multi-key UI timer update (Task 6)
- [x] Account name display (Task 5, Step 2)

**Placeholder scan:**
- [x] No TBD/TODO
- [x] No vague "add error handling" without specifics
- [x] All code blocks contain actual implementation code
- [x] No "similar to Task N" references

**Type consistency:**
- [x] `account_index` is `int` in all signatures
- [x] `KIMI_MAX_ACCOUNTS` used consistently across all files
- [x] `g_kimi_data[]` array access uses `int i` index everywhere
