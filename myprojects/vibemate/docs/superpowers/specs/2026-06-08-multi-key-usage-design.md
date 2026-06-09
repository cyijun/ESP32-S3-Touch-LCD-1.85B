# Multi-Key Kimi Usage Display — Design Document

**Date:** 2026-06-08  
**Project:** VibeMate (ESP32-S3, 360×360 round LCD)  
**Status:** Approved

## 1. Objective

Extend the Usage page (tile index 3) to support multiple Kimi API keys. Each key gets its own full-screen tile in a vertically-extended column. Newly added keys appear at the top (y=0), while the original key stays on the "main row" where the user lands when swiping right from the Pet page.

## 2. Design Decisions

### 2.1 Tileview Layout — Center-Row Shift

The key insight: shift the "center row" of the tileview to `y = KIMI_ACCOUNT_COUNT - 1` so that:

- The original key (key0) sits on the same row as the Pet page
- Swiping right from Pet lands on key0 (same y)
- Newer keys are stacked above (smaller y)
- The layout works for any number of keys (1, 2, 3, ...)

```
center_y = KIMI_ACCOUNT_COUNT - 1

       x=0          x=1          x=2           x=3          x=4
y=0                                              [keyN-1 newest]
y=1                                              [keyN-2]
...                                              ...
y=c-1  [Pet Select] [Pet Detail] [Pet Main]    [key0]       [Device]
```

### 2.2 Configuration — C Struct Array (Compile-Time)

Uses a `kimi_account_t` struct array in `config.h`. Zero filesystem dependency.

```cpp
struct kimi_account_t {
    const char *name;   // Display label (e.g. "主号")
    const char *key;    // API key macro
};

// Top-to-bottom = display top-to-bottom (newest → oldest)
// Append new keys to the END of the array — they display at the TOP (y=0)
static const kimi_account_t KIMI_ACCOUNTS[] = {
    { "主号", KIMI_API_KEY_1 },   // index 0, oldest, y = count-1 (main row)
    { "副号", KIMI_API_KEY_2 },   // index 1, newer, y = count-2
};
#define KIMI_ACCOUNT_COUNT (sizeof(KIMI_ACCOUNTS) / sizeof(KIMI_ACCOUNTS[0]))
```

### 2.3 API Request Strategy — Serial with Delay

A single FreeRTOS task (`s_api_task`, 12KB stack) requests all keys serially:

1. Loop through `KIMI_ACCOUNTS[]`
2. For each key: HTTP GET → parse JSON → write to `g_kimi_data[i]` (mutex-protected)
3. 500ms delay between requests (except after the last) to avoid rate limiting
4. After all keys done: set `g_ui_needs_update = true`

Single task minimizes memory and avoids concurrent HTTP connection issues.

## 3. Architecture

### 3.1 File Changes

| File | Change Type | Description |
|------|-------------|-------------|
| `config.h` | Modify | Add `kimi_account_t` struct and `KIMI_ACCOUNTS[]` array |
| `kimi_api.h` | Modify | `g_kimi_data` → array, add `KIMI_MAX_ACCOUNTS`, `g_kimi_account_count` |
| `kimi_api.cpp` | Modify | `s_do_http_request` takes `api_key` param; `s_api_task` serial loop |
| `ui_usage.h` | Modify | Add `account_index` param to `create`/`update` |
| `ui_usage.cpp` | Modify | `usage_page_t` struct array; per-page creation and update |
| `vibemate.ino` | Modify | Dynamic `tile_usage[]`; center-y shift; updated event handler |

### 3.2 Data Structures

**`kimi_api.h`:**
```cpp
#define KIMI_MAX_ACCOUNTS 8

extern kimi_usage_t g_kimi_data[KIMI_MAX_ACCOUNTS];
extern int g_kimi_account_count;
extern bool g_ui_needs_update;
extern SemaphoreHandle_t kimi_mutex;
```

**`ui_usage.cpp`:**
```cpp
struct usage_page_t {
    lv_obj_t *arc_week;
    lv_obj_t *arc_window;
    lv_obj_t *label_percent;
    lv_obj_t *label_center;
    lv_obj_t *label_window_pct;
    lv_obj_t *label_plan;        // Shows account name
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

### 3.3 Tileview Construction (vibemate.ino)

```cpp
static int s_center_y = 0;
static lv_obj_t *tile_usage[KIMI_MAX_ACCOUNTS];
static int s_usage_tile_count = 0;

// In setup():
s_usage_tile_count = KIMI_ACCOUNT_COUNT;
s_center_y = KIMI_ACCOUNT_COUNT - 1;

// Non-usage tiles on the center row
tile_pet_select = lv_tileview_add_tile(tileview, 0, s_center_y, LV_DIR_HOR);
tile_pet_detail = lv_tileview_add_tile(tileview, 1, s_center_y, LV_DIR_HOR);
tile_pet       = lv_tileview_add_tile(tileview, 2, s_center_y, LV_DIR_HOR);
tile_device    = lv_tileview_add_tile(tileview, 4, s_center_y, LV_DIR_HOR);

// Usage tiles: newest at top (y=0), oldest on center row
for (int i = 0; i < s_usage_tile_count; i++) {
    int y = s_center_y - i;  // i=0 (oldest) → y=center_y; i=N-1 → y=0
    tile_usage[i] = lv_tileview_add_tile(tileview, 3, y, LV_DIR_ALL);
    // ... style setup ...
    ui_usage_create(tile_usage[i], i);
}

lv_obj_set_tile(tileview, tile_pet, LV_ANIM_OFF);
```

### 3.4 Event Handler

```cpp
static void tileview_event_cb(lv_event_t *e) {
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

    if (tile == tile_pet) ui_pet_resume_anim();
    else ui_pet_pause_anim();
}
```

### 3.5 UI Update Flow

```cpp
static void ui_timer_cb(lv_timer_t *timer) {
    if (!g_ui_needs_update || !kimi_mutex) return;
    if (xSemaphoreTake(kimi_mutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

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
```

## 4. Error Handling

| Scenario | Behavior |
|----------|----------|
| Single key fails (401/403/network) | That tile shows error state; continues to next key |
| All keys fail | All tiles show respective errors |
| Mixed success/fail | Successful tiles normal; failed tiles show error |
| Task creation fails | `g_kimi_data[0].last_error = "Task create failed"`; triggers UI update |
| JSON parse error | `last_error` set; `api_ok = false` for that key |
| WiFi disconnected | All keys get "WiFi not connected" error |

## 5. Future Extension — DeepSeek Balance

The center-row shift design naturally supports adding DeepSeek below the Kimi keys:

```
y=0  [Kimi newest]
...
y=c-1 [Kimi oldest] [Pet Main] [Device]
y=c   [DeepSeek 1]          ← 新增，向下延伸
```

Place DeepSeek tiles at `(3, c)`, `(3, c+1)`, etc. They sit below the center row, accessible by swiping down from key0.

## 6. Constraints & Limits

- `KIMI_MAX_ACCOUNTS = 8` (compile-time constant)
- Each HTTP request timeout: 10s
- Inter-request delay: 500ms (rate limiting protection)
- Total refresh time for N keys: ~N × (RTT + 500ms)
- Memory: `g_kimi_data[8]`, `s_pages[8]`, `tile_usage[8]` — all static, ~2KB total
