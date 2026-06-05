#include "ui_voice.h"
#include "voice_network.h"

#define COLOR_BG       0x0a0e17
#define COLOR_TEXT     0xf0f4f8
#define COLOR_SUBTEXT  0x94a3b8
#define COLOR_ACCENT   0x4fc3f7
#define COLOR_GREEN    0x4ade80
#define COLOR_ORANGE   0xffa726
#define COLOR_RED      0xef4444

static lv_obj_t *status_dot;
static lv_obj_t *label_status;
static lv_obj_t *label_host;
static lv_obj_t *btn_ptt_mode;
static lv_obj_t *btn_duplex_mode;
static lv_obj_t *btn_ptt_big;
static lv_obj_t *label_ptt_big;
static lv_obj_t *btn_duplex_toggle;
static lv_obj_t *label_duplex_toggle;
static lv_obj_t *bar_volume;

static voice_state_t s_last_state = VOICE_IDLE;
static voice_mode_t s_current_mode = VOICE_MODE_PTT;
static bool s_duplex_active = false;

static const char* state_to_text(voice_state_t state) {
    switch (state) {
        case VOICE_IDLE:         return "未连接";
        case VOICE_DISCOVERING:  return "发现中...";
        case VOICE_CONNECTING:   return "连接中...";
        case VOICE_CONNECTED:    return "已连接";
        case VOICE_TRANSMITTING: return "发送中";
        default:                 return "未知";
    }
}

static lv_color_t state_to_color(voice_state_t state) {
    switch (state) {
        case VOICE_IDLE:         return lv_color_hex(COLOR_RED);
        case VOICE_DISCOVERING:  return lv_color_hex(COLOR_ORANGE);
        case VOICE_CONNECTING:   return lv_color_hex(COLOR_ORANGE);
        case VOICE_CONNECTED:    return lv_color_hex(COLOR_GREEN);
        case VOICE_TRANSMITTING: return lv_color_hex(COLOR_ACCENT);
        default:                 return lv_color_hex(COLOR_SUBTEXT);
    }
}

static void update_mode_buttons(void) {
    if (s_current_mode == VOICE_MODE_PTT) {
        lv_obj_set_style_bg_color(btn_ptt_mode, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(btn_ptt_mode, 0), lv_color_hex(COLOR_BG), 0);
        lv_obj_set_style_bg_color(btn_duplex_mode, lv_color_hex(0x1e293b), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(btn_duplex_mode, 0), lv_color_hex(COLOR_TEXT), 0);
        lv_obj_clear_flag(btn_ptt_big, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_duplex_toggle, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_set_style_bg_color(btn_ptt_mode, lv_color_hex(0x1e293b), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(btn_ptt_mode, 0), lv_color_hex(COLOR_TEXT), 0);
        lv_obj_set_style_bg_color(btn_duplex_mode, lv_color_hex(COLOR_ACCENT), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(btn_duplex_mode, 0), lv_color_hex(COLOR_BG), 0);
        lv_obj_add_flag(btn_ptt_big, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_duplex_toggle, LV_OBJ_FLAG_HIDDEN);
    }
}

static void mode_btn_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    if (btn == btn_ptt_mode && s_current_mode != VOICE_MODE_PTT) {
        s_current_mode = VOICE_MODE_PTT;
        voice_set_mode(VOICE_MODE_PTT);
        update_mode_buttons();
    } else if (btn == btn_duplex_mode && s_current_mode != VOICE_MODE_DUPLEX) {
        s_current_mode = VOICE_MODE_DUPLEX;
        voice_set_mode(VOICE_MODE_DUPLEX);
        update_mode_buttons();
    }
}

static void ptt_press_cb(lv_event_t *e) {
    (void)e;
    voice_ptt_set(true);
    lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_GREEN), 0);
    lv_label_set_text(label_ptt_big, "发送中...");
}

static void ptt_release_cb(lv_event_t *e) {
    (void)e;
    voice_ptt_set(false);
    lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_ACCENT), 0);
    lv_label_set_text(label_ptt_big, "按住说话");
}

static void duplex_toggle_cb(lv_event_t *e) {
    (void)e;
    s_duplex_active = !s_duplex_active;
    if (s_duplex_active) {
        lv_obj_set_style_bg_color(btn_duplex_toggle, lv_color_hex(COLOR_RED), 0);
        lv_label_set_text(label_duplex_toggle, "结束通话");
    } else {
        lv_obj_set_style_bg_color(btn_duplex_toggle, lv_color_hex(COLOR_GREEN), 0);
        lv_label_set_text(label_duplex_toggle, "开始通话");
    }
}

void ui_voice_create(lv_obj_t *parent_tile) {
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // --- Status dot + label (top area) ---
    status_dot = lv_obj_create(parent_tile);
    lv_obj_set_size(status_dot, 10, 10);
    lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(COLOR_RED), 0);
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_set_style_pad_all(status_dot, 0, 0);
    lv_obj_align(status_dot, LV_ALIGN_TOP_MID, -55, 25);

    label_status = lv_label_create(parent_tile);
    lv_label_set_text(label_status, "未连接");
    lv_obj_set_style_text_color(label_status, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
    lv_obj_align_to(label_status, status_dot, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    label_host = lv_label_create(parent_tile);
    lv_label_set_text(label_host, "--");
    lv_obj_set_style_text_color(label_host, lv_color_hex(COLOR_SUBTEXT), 0);
    lv_obj_set_style_text_font(label_host, &lv_font_montserrat_12, 0);
    lv_obj_align(label_host, LV_ALIGN_TOP_MID, 0, 45);

    // --- Mode toggle buttons (y ~75) ---
    btn_ptt_mode = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_ptt_mode, 80, 32);
    lv_obj_set_style_radius(btn_ptt_mode, 16, 0);
    lv_obj_set_style_bg_color(btn_ptt_mode, lv_color_hex(COLOR_ACCENT), 0);
    lv_obj_set_style_border_width(btn_ptt_mode, 0, 0);
    lv_obj_set_style_shadow_width(btn_ptt_mode, 0, 0);
    lv_obj_align(btn_ptt_mode, LV_ALIGN_TOP_MID, -45, 75);
    lv_obj_add_event_cb(btn_ptt_mode, mode_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_ptt = lv_label_create(btn_ptt_mode);
    lv_label_set_text(label_ptt, "PTT");
    lv_obj_set_style_text_font(label_ptt, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_ptt, lv_color_hex(COLOR_BG), 0);
    lv_obj_center(label_ptt);

    btn_duplex_mode = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_duplex_mode, 80, 32);
    lv_obj_set_style_radius(btn_duplex_mode, 16, 0);
    lv_obj_set_style_bg_color(btn_duplex_mode, lv_color_hex(0x1e293b), 0);
    lv_obj_set_style_border_width(btn_duplex_mode, 0, 0);
    lv_obj_set_style_shadow_width(btn_duplex_mode, 0, 0);
    lv_obj_align(btn_duplex_mode, LV_ALIGN_TOP_MID, 45, 75);
    lv_obj_add_event_cb(btn_duplex_mode, mode_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_duplex = lv_label_create(btn_duplex_mode);
    lv_label_set_text(label_duplex, "全双工");
    lv_obj_set_style_text_font(label_duplex, &font_cjk_14, 0);
    lv_obj_set_style_text_color(label_duplex, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_center(label_duplex);

    // --- Big PTT button (center, 160x160 circle) ---
    btn_ptt_big = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_ptt_big, 160, 160);
    lv_obj_set_style_radius(btn_ptt_big, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_ptt_big, lv_color_hex(COLOR_ACCENT), 0);
    lv_obj_set_style_border_width(btn_ptt_big, 0, 0);
    lv_obj_set_style_shadow_width(btn_ptt_big, 0, 0);
    lv_obj_align(btn_ptt_big, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_event_cb(btn_ptt_big, ptt_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(btn_ptt_big, ptt_press_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(btn_ptt_big, ptt_release_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(btn_ptt_big, ptt_release_cb, LV_EVENT_PRESS_LOST, NULL);

    label_ptt_big = lv_label_create(btn_ptt_big);
    lv_label_set_text(label_ptt_big, "按住说话");
    lv_obj_set_style_text_font(label_ptt_big, &font_cjk_14, 0);
    lv_obj_set_style_text_color(label_ptt_big, lv_color_hex(COLOR_BG), 0);
    lv_obj_center(label_ptt_big);

    // --- Duplex toggle button (same position, hidden by default) ---
    btn_duplex_toggle = lv_btn_create(parent_tile);
    lv_obj_set_size(btn_duplex_toggle, 140, 50);
    lv_obj_set_style_radius(btn_duplex_toggle, 25, 0);
    lv_obj_set_style_bg_color(btn_duplex_toggle, lv_color_hex(COLOR_GREEN), 0);
    lv_obj_set_style_border_width(btn_duplex_toggle, 0, 0);
    lv_obj_set_style_shadow_width(btn_duplex_toggle, 0, 0);
    lv_obj_align(btn_duplex_toggle, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_flag(btn_duplex_toggle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(btn_duplex_toggle, duplex_toggle_cb, LV_EVENT_CLICKED, NULL);

    label_duplex_toggle = lv_label_create(btn_duplex_toggle);
    lv_label_set_text(label_duplex_toggle, "开始通话");
    lv_obj_set_style_text_font(label_duplex_toggle, &font_cjk_14, 0);
    lv_obj_set_style_text_color(label_duplex_toggle, lv_color_hex(COLOR_BG), 0);
    lv_obj_center(label_duplex_toggle);

    // --- Volume bar (y ~ center+105, 200x8) ---
    bar_volume = lv_bar_create(parent_tile);
    lv_obj_set_size(bar_volume, 200, 8);
    lv_bar_set_range(bar_volume, 0, 100);
    lv_bar_set_value(bar_volume, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_volume, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(bar_volume, 4, LV_PART_INDICATOR);
    lv_obj_align(bar_volume, LV_ALIGN_CENTER, 0, 105);
}

void ui_voice_update(void) {
    voice_state_t state = voice_get_state();

    if (state != s_last_state) {
        s_last_state = state;
        lv_obj_set_style_bg_color(status_dot, state_to_color(state), 0);
        lv_label_set_text(label_status, state_to_text(state));
    }

    const char *host_name = voice_get_host_name();
    const char *host_ip = voice_get_host_ip();
    char buf[64];
    if (host_name && host_name[0] && host_ip && host_ip[0]) {
        lv_snprintf(buf, sizeof(buf), "%s %s", host_name, host_ip);
    } else if (host_ip && host_ip[0]) {
        lv_snprintf(buf, sizeof(buf), "%s", host_ip);
    } else {
        lv_snprintf(buf, sizeof(buf), "--");
    }
    lv_label_set_text(label_host, buf);
}
