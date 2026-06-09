#include "ui_usage.h"
#include "kimi_api.h"
#include "ui_helpers.h"

static lv_obj_t *arc_week;
static lv_obj_t *arc_window;
static lv_obj_t *label_percent;
static lv_obj_t *label_center;
static lv_obj_t *label_window_pct;
static lv_obj_t *label_plan;
static lv_obj_t *label_tier;
static lv_obj_t *status_dot;
static lv_obj_t *label_legend_week;
static lv_obj_t *label_legend_window;
static lv_obj_t *dot_legend_week;
static lv_obj_t *dot_legend_window;
static lv_obj_t *tick_ring;

static float s_last_week_pct = 0.0f;
static float s_last_window_pct = 0.0f;
static bool s_has_data = false;

/*static void tile_click_cb(lv_event_t *e) {
    (void)e;
    kimi_api_refresh_now();
}*/

static void tick_ring_draw_cb(lv_event_t *e) {
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    lv_obj_t *obj = lv_event_get_target(e);

    const lv_area_t *clip = (const lv_area_t *)lv_event_get_param(e);
    (void)clip;

    const int32_t cx = 180;
    const int32_t cy = 180;
    const int32_t r_outer = 150;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.round_start = false;
    line_dsc.round_end = false;

    for (int i = 0; i < 60; i++) {
        bool is_major = (i % 5 == 0);
        int32_t len = is_major ? 6 : 3;
        int32_t width = is_major ? 2 : 1;
        lv_color_t color = is_major ? lv_color_hex(0x334155) : lv_color_hex(0x1e293b);

        float angle_deg = (i * 6.0f) - 90.0f;
        float rad = angle_deg * 3.14159265f / 180.0f;

        float cos_a = cosf(rad);
        float sin_a = sinf(rad);

        int32_t x1 = cx + (int32_t)(cos_a * (r_outer - len));
        int32_t y1 = cy + (int32_t)(sin_a * (r_outer - len));
        int32_t x2 = cx + (int32_t)(cos_a * r_outer);
        int32_t y2 = cy + (int32_t)(sin_a * r_outer);

        lv_point_t p1 = { (lv_coord_t)x1, (lv_coord_t)y1 };
        lv_point_t p2 = { (lv_coord_t)x2, (lv_coord_t)y2 };

        line_dsc.width = width;
        line_dsc.color = color;
        lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
    }
}

void ui_usage_create(lv_obj_t *parent_tile) {
    ui_style_tile_alt(parent_tile);

    // lv_obj_add_event_cb(parent_tile, tile_click_cb, LV_EVENT_CLICKED, NULL);

    // --- Tick ring (decorative, behind arcs) ---
    // tick_ring = lv_obj_create(parent_tile);
    // lv_obj_set_size(tick_ring, 360, 360);
    // lv_obj_set_style_bg_opa(tick_ring, LV_OPA_TRANSP, 0);
    // lv_obj_set_style_border_width(tick_ring, 0, 0);
    // lv_obj_set_style_pad_all(tick_ring, 0, 0);
    // lv_obj_clear_flag(tick_ring, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_add_event_cb(tick_ring, tick_ring_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    // lv_obj_align(tick_ring, LV_ALIGN_CENTER, 0, 0);

    // --- Status dot (top-right) ---
    status_dot = lv_obj_create(parent_tile);
    lv_obj_set_size(status_dot, 8, 8);
    lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(0xef4444), 0);
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_set_style_pad_all(status_dot, 0, 0);
    lv_obj_align(status_dot, LV_ALIGN_TOP_MID, 85, 30);

    // --- Top title area ---
    label_plan = lv_label_create(parent_tile);
    lv_label_set_text(label_plan, "Kimi Coding Plan");
    lv_obj_set_style_text_color(label_plan, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_plan, &lv_font_montserrat_14, 0);
    lv_obj_align(label_plan, LV_ALIGN_TOP_MID, 0, 30);

    label_tier = lv_label_create(parent_tile);
    lv_label_set_text(label_tier, "--");
    lv_obj_set_style_text_color(label_tier, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_tier, &lv_font_montserrat_16, 0);
    lv_obj_align(label_tier, LV_ALIGN_TOP_MID, 0, 50);

    // --- Outer arc (week usage) ---
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
    lv_obj_clear_flag(arc_week, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(arc_week, LV_ALIGN_CENTER, 0, 0);

    // --- Inner arc (5h window) ---
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
    lv_obj_clear_flag(arc_window, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(arc_window, LV_ALIGN_CENTER, 0, 0);

    // --- Center text ---
    label_percent = lv_label_create(parent_tile);
    lv_label_set_text(label_percent, "0%");
    lv_obj_set_style_text_color(label_percent, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_24, 0);
    lv_obj_align(label_percent, LV_ALIGN_CENTER, 0, -10);

    label_center = lv_label_create(parent_tile);
    lv_label_set_text(label_center, "周用量已用");
    lv_obj_set_style_text_color(label_center, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_center, &font_cjk_14, 0);
    lv_obj_align(label_center, LV_ALIGN_CENTER, 0, 18);

    label_window_pct = lv_label_create(parent_tile);
    lv_label_set_text(label_window_pct, "5h: --%");
    lv_obj_set_style_text_color(label_window_pct, lv_color_hex(0xffa726), 0);
    lv_obj_set_style_text_font(label_window_pct, &lv_font_montserrat_14, 0);
    lv_obj_align(label_window_pct, LV_ALIGN_CENTER, 0, 38);

    // --- Bottom legend ---
    dot_legend_week = lv_obj_create(parent_tile);
    lv_obj_set_size(dot_legend_week, 8, 8);
    lv_obj_set_style_radius(dot_legend_week, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot_legend_week, lv_color_hex(0x4fc3f7), 0);
    lv_obj_set_style_border_width(dot_legend_week, 0, 0);
    lv_obj_set_style_pad_all(dot_legend_week, 0, 0);
    lv_obj_align(dot_legend_week, LV_ALIGN_BOTTOM_MID, -40, -35);

    label_legend_week = lv_label_create(parent_tile);
    lv_label_set_text(label_legend_week, "7天");
    lv_obj_set_style_text_color(label_legend_week, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_legend_week, &font_cjk_14, 0);
    lv_obj_align_to(label_legend_week, dot_legend_week, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    dot_legend_window = lv_obj_create(parent_tile);
    lv_obj_set_size(dot_legend_window, 8, 8);
    lv_obj_set_style_radius(dot_legend_window, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot_legend_window, lv_color_hex(0xffa726), 0);
    lv_obj_set_style_border_width(dot_legend_window, 0, 0);
    lv_obj_set_style_pad_all(dot_legend_window, 0, 0);
    lv_obj_align(dot_legend_window, LV_ALIGN_BOTTOM_MID, 30, -35);

    label_legend_window = lv_label_create(parent_tile);
    lv_label_set_text(label_legend_window, "5小时");
    lv_obj_set_style_text_color(label_legend_window, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_legend_window, &font_cjk_14, 0);
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

        lv_label_set_text(label_tier, data->tier_name.c_str());

        lv_snprintf(buf, sizeof(buf), "5h: %d%%", (int)data->window_pct);
        lv_label_set_text(label_window_pct, buf);

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
