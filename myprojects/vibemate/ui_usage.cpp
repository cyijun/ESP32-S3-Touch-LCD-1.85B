#include "ui_usage.h"
#include "kimi_api.h"

static lv_obj_t *arc_week;
static lv_obj_t *arc_window;
static lv_obj_t *label_percent;
static lv_obj_t *label_title;
static lv_obj_t *label_sub;
static lv_obj_t *label_tier;
static lv_obj_t *status_dot;
static lv_obj_t *label_legend_week;
static lv_obj_t *label_legend_window;
static lv_obj_t *dot_legend_week;
static lv_obj_t *dot_legend_window;

static float s_last_week_pct = 0.0f;
static float s_last_window_pct = 0.0f;
static bool s_has_data = false;

static void tile_click_cb(lv_event_t *e) {
    (void)e;
    kimi_api_refresh_now();
}

void ui_usage_create(lv_obj_t *parent_tile) {
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    lv_obj_add_event_cb(parent_tile, tile_click_cb, LV_EVENT_CLICKED, NULL);

    // --- Top title area ---
    lv_obj_t *label_kimi = lv_label_create(parent_tile);
    lv_label_set_text(label_kimi, "KIMI CODING");
    lv_obj_set_style_text_color(label_kimi, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_kimi, &lv_font_montserrat_14, 0);
    lv_obj_align(label_kimi, LV_ALIGN_TOP_MID, 0, 30);

    label_tier = lv_label_create(parent_tile);
    lv_label_set_text(label_tier, "--");
    lv_obj_set_style_text_color(label_tier, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_tier, &lv_font_montserrat_16, 0);
    lv_obj_align(label_tier, LV_ALIGN_TOP_MID, 0, 50);

    // --- Status dot ---
    status_dot = lv_obj_create(parent_tile);
    lv_obj_set_size(status_dot, 8, 8);
    lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(0xef4444), 0);
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_set_style_pad_all(status_dot, 0, 0);
    lv_obj_align(status_dot, LV_ALIGN_TOP_RIGHT, -30, 34);

    // --- Outer arc (week) ---
    arc_week = lv_arc_create(parent_tile);
    lv_obj_set_size(arc_week, 260, 260);
    lv_arc_set_rotation(arc_week, 270);
    lv_arc_set_bg_angles(arc_week, 0, 360);
    lv_arc_set_range(arc_week, 0, 100);
    lv_arc_set_value(arc_week, 0);
    lv_obj_set_style_arc_width(arc_week, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc_week, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_week, lv_color_hex(0x4fc3f7), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_week, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_remove_style(arc_week, NULL, LV_PART_KNOB);
    lv_obj_align(arc_week, LV_ALIGN_CENTER, 0, 0);

    // --- Inner arc (window) ---
    arc_window = lv_arc_create(parent_tile);
    lv_obj_set_size(arc_window, 200, 200);
    lv_arc_set_rotation(arc_window, 270);
    lv_arc_set_bg_angles(arc_window, 0, 360);
    lv_arc_set_range(arc_window, 0, 100);
    lv_arc_set_value(arc_window, 0);
    lv_obj_set_style_arc_width(arc_window, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc_window, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_window, lv_color_hex(0xffa726), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_window, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_remove_style(arc_window, NULL, LV_PART_KNOB);
    lv_obj_align(arc_window, LV_ALIGN_CENTER, 0, 0);

    // --- Center text ---
    label_percent = lv_label_create(parent_tile);
    lv_label_set_text(label_percent, "0%");
    lv_obj_set_style_text_color(label_percent, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_36, 0);
    lv_obj_align(label_percent, LV_ALIGN_CENTER, 0, -10);

    label_title = lv_label_create(parent_tile);
    lv_label_set_text(label_title, "周用量剩余");
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);
    lv_obj_align(label_title, LV_ALIGN_CENTER, 0, 22);

    label_sub = lv_label_create(parent_tile);
    lv_label_set_text(label_sub, "-- / --");
    lv_obj_set_style_text_color(label_sub, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_sub, &lv_font_montserrat_14, 0);
    lv_obj_align(label_sub, LV_ALIGN_CENTER, 0, 42);

    // --- Bottom legend ---
    dot_legend_week = lv_obj_create(parent_tile);
    lv_obj_set_size(dot_legend_week, 8, 8);
    lv_obj_set_style_radius(dot_legend_week, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot_legend_week, lv_color_hex(0x4fc3f7), 0);
    lv_obj_set_style_border_width(dot_legend_week, 0, 0);
    lv_obj_set_style_pad_all(dot_legend_week, 0, 0);
    lv_obj_align(dot_legend_week, LV_ALIGN_BOTTOM_MID, -50, -35);

    label_legend_week = lv_label_create(parent_tile);
    lv_label_set_text(label_legend_week, "周配额");
    lv_obj_set_style_text_color(label_legend_week, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_legend_week, &lv_font_montserrat_14, 0);
    lv_obj_align_to(label_legend_week, dot_legend_week, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    dot_legend_window = lv_obj_create(parent_tile);
    lv_obj_set_size(dot_legend_window, 8, 8);
    lv_obj_set_style_radius(dot_legend_window, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot_legend_window, lv_color_hex(0xffa726), 0);
    lv_obj_set_style_border_width(dot_legend_window, 0, 0);
    lv_obj_set_style_pad_all(dot_legend_window, 0, 0);
    lv_obj_align(dot_legend_window, LV_ALIGN_BOTTOM_MID, 30, -35);

    label_legend_window = lv_label_create(parent_tile);
    lv_label_set_text(label_legend_window, "5h窗口");
    lv_obj_set_style_text_color(label_legend_window, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_legend_window, &lv_font_montserrat_14, 0);
    lv_obj_align_to(label_legend_window, dot_legend_window, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
}

void ui_usage_update(const kimi_usage_t *data) {
    if (!data) return;

    if (data->api_ok) {
        s_has_data = true;
        s_last_week_pct = data->week_pct;
        s_last_window_pct = data->window_pct;

        lv_arc_set_value(arc_week, (int32_t)data->week_pct);
        lv_arc_set_value(arc_window, (int32_t)data->window_pct);

        char buf[64];
        lv_snprintf(buf, sizeof(buf), "%d%%", (int)data->week_pct);
        lv_label_set_text(label_percent, buf);

        lv_label_set_text(label_title, "周用量剩余");

        lv_snprintf(buf, sizeof(buf), "%.0f / %.0f", data->week_remaining, data->week_limit);
        lv_label_set_text(label_sub, buf);

        lv_label_set_text(label_tier, data->tier_name.c_str());

        lv_obj_set_style_bg_color(status_dot, lv_color_hex(0x4ade80), 0);
    } else {
        if (s_has_data) {
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(0xffa726), 0);
        } else {
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(0xef4444), 0);
        }

        if (data->last_error.indexOf("401") >= 0 || data->last_error.indexOf("403") >= 0) {
            lv_label_set_text(label_percent, "Invalid Key");
            lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_16, 0);
        }
    }
}
