#ifndef KIMI_API_H
#define KIMI_API_H

#include <Arduino.h>
#include <lvgl.h>

struct kimi_usage_t {
    // 周限额
    float week_limit;
    float week_remaining;
    float week_used;
    float week_pct;
    String week_reset_time;

    // 5小时窗口
    float window_limit;
    float window_remaining;
    float window_used;
    float window_pct;
    String window_reset_time;

    // 元信息
    String tier_name;
    bool api_ok;
    String last_error;
    unsigned long last_update_ms;
};

// 全局数据实例（定义在 .cpp 中）
extern kimi_usage_t g_kimi_data;
extern bool g_ui_needs_update;
extern SemaphoreHandle_t kimi_mutex;

void kimi_api_init(void);
void kimi_api_refresh_now(void);
void kimi_api_timer_cb(lv_timer_t *timer);

#endif
