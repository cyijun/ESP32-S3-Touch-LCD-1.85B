#include "ui_pet.h"
#include "pet_sprites.h"
#include "pet_manager.h"
#include "ui_anim.h"
#include "ui_helpers.h"
#include <Esp.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// ========== UI Objects ==========
static lv_obj_t *s_label_name;
static lv_obj_t *s_label_rarity;
static lv_obj_t *s_bar_hunger;
static lv_obj_t *s_label_hunger;
static lv_obj_t *s_bar_joy;
static lv_obj_t *s_label_joy;
static lv_obj_t *s_bubble;
static lv_obj_t *s_bubble_label;
static lv_obj_t *s_label_sprite;
static lv_obj_t *s_overlay;
static lv_obj_t *s_hat_menu;
static lv_obj_t *s_hat_grid;
static lv_obj_t *s_hat_buttons[HAT_COUNT];

static lv_obj_t *s_btn_feed;
static lv_obj_t *s_btn_talk;
static lv_obj_t *s_btn_play;
static lv_obj_t *s_label_feed;
static lv_obj_t *s_label_talk;
static lv_obj_t *s_label_play;

// ========== Animation State ==========
static lv_timer_t *s_timer_float = NULL;
static uint32_t s_anim_frame = 0;

// ========== Bubble Timer ==========
static lv_timer_t *s_bubble_timer = NULL;

// ========== Effect Animation State ==========
static lv_timer_t *s_wiggle_timer = NULL;
static lv_timer_t *s_jump_timer = NULL;
static lv_timer_t *s_float_text_timer = NULL;
static lv_timer_t *s_feed_bubble_timer = NULL;
static lv_timer_t *s_play_bubble_timer = NULL;
static lv_timer_t *s_auto_bubble_timer = NULL;
static lv_obj_t *s_float_text_label = NULL;
static int s_wiggle_step = 0;
static int s_jump_step = 0;
static int s_float_text_step = 0;

// ========== Page Active Flag ==========
static bool s_pet_active = true;

// ========== Touch State ==========
static uint32_t s_press_tick = 0;
static uint8_t s_long_triggered = 0;
static lv_timer_t *s_long_press_timer = NULL;
static lv_timer_t *s_defer_timer = NULL;

// ========== Message Pools ==========

// Idle messages - shown on talk button or tap
static const char *IDLE_MESSAGES[] = {
    "在写 bug 呢？需要我帮忙吗？",
    "休息一下吧，眼睛会感谢你的。",
    "加油，离 commit 还有 47 个文件。",
    "我看好你，真的。",
    "你的代码风格…挺有创意的。",
    "这个变量名是认真的吗？",
    "又在写代码？注意休息。",
    "这个函数名…很有想象力。",
    "我看到你删了 200 行代码，干得好。",
    "编译通过了？不可思议。",
    "去喝杯水吧，我替你盯着。",
    "你的光标已经 5 分钟没动了。",
    "这个 bug 看起来很有意思。",
    "加油，离下班还有 3 小时。",
    "你在发呆吗？",
    "需要我帮你找找灵感吗？",
    "今天的代码量达标了吗？",
    "我饿的时候也会脾气不好，你也是吧？",
    "你屏幕上的光比我的还亮。",
    "按 Ctrl+S 了吗？",
    "这个 commit message 可以再敷衍一点。",
};
static const int IDLE_MSG_COUNT = sizeof(IDLE_MESSAGES) / sizeof(IDLE_MESSAGES[0]);

// Feed responses - categorized by hunger level after eating
static const char *FEED_HUNGRY_MSGS[] = {
    "终于…",
    "等好久了！",
    "再来一份！",
    "太及时了！",
    "救命恩人！",
};
static const int FEED_HUNGRY_COUNT = sizeof(FEED_HUNGRY_MSGS) / sizeof(FEED_HUNGRY_MSGS[0]);

static const char *FEED_NORMAL_MSGS[] = {
    "谢谢款待。",
    "味道不错。",
    "刚好饿了。",
    "好吃~",
    "正是我想要的。",
};
static const int FEED_NORMAL_COUNT = sizeof(FEED_NORMAL_MSGS) / sizeof(FEED_NORMAL_MSGS[0]);

static const char *FEED_FULL_MSGS[] = {
    "吃不下了…",
    "肚子圆了。",
    "满足~",
    "好撑…",
    "已经很饱了！",
};
static const int FEED_FULL_COUNT = sizeof(FEED_FULL_MSGS) / sizeof(FEED_FULL_MSGS[0]);

static const char *PLAY_RESPONSES[] = {
    "好玩！",
    "再来！",
    "我飞起来了~",
    "catch me if you can!",
    "这个比写代码有趣多了。",
    "耶！",
};
static const int PLAY_RESP_COUNT = sizeof(PLAY_RESPONSES) / sizeof(PLAY_RESPONSES[0]);

// Auto bubble messages - pet talks spontaneously
static const char *AUTO_MESSAGES[] = {
    "你还在啊？",
    "我刚刚打了个盹。",
    "这个屏幕看久了眼睛会酸哦。",
    "又在写代码？",
    "我数了一下，你敲了 347 下键盘。",
    "今天天气怎么样？",
    "我在思考人生…骗你的，我在想吃的。",
    "你的代码编译过了吗？",
    "我有点想你了。",
    "咕噜咕噜…",
    "你相信光吗？",
    "检测到人类活动。",
};
static const int AUTO_MSG_COUNT = sizeof(AUTO_MESSAGES) / sizeof(AUTO_MESSAGES[0]);

// Greetings when entering the pet page
static const char *ENTER_GREETINGS[] = {
    "你回来了！",
    "好久不见！",
    "想我了吗？",
    "欢迎回来！",
    "终于等到你了！",
};
static const int ENTER_GREET_COUNT = sizeof(ENTER_GREETINGS) / sizeof(ENTER_GREETINGS[0]);

// Hat change responses
static const char *HAT_RESPONSES[] = {
    "很适合我！",
    "这样好看吗？",
    "有点紧…",
    "新造型！",
    "我喜欢这个！",
};
static const int HAT_RESP_COUNT = sizeof(HAT_RESPONSES) / sizeof(HAT_RESPONSES[0]);

// ========== Rarity Colors ==========
static uint32_t rarity_color(PetRarity r)
{
    switch (r) {
        case RARITY_COMMON:    return 0x8A8A95;
        case RARITY_UNCOMMON:  return 0x5EE7DF;
        case RARITY_RARE:      return 0x6B8CFF;
        case RARITY_EPIC:      return 0xC85EFF;
        case RARITY_LEGENDARY: return 0xFFD166;
        default:               return 0x8A8A95;
    }
}

// ========== Forward Declarations ==========
static void s_render_sprite(void);
static void s_show_bubble(const char *text, uint32_t duration_ms);
static void s_hide_bubble_cb(lv_timer_t *t);
static void s_update_bars(void);
static void s_on_feed(lv_event_t *e);
static void s_on_talk(lv_event_t *e);
static void s_on_play(lv_event_t *e);
static void s_on_overlay_pressed(lv_event_t *e);
static void s_on_overlay_released(lv_event_t *e);
static void s_long_press_cb(lv_timer_t *t);
static void s_show_hat_menu(void);
static void s_hide_hat_menu(void);
static void s_on_hat_pick(lv_event_t *e);
static void s_anim_float_cb(lv_timer_t *t);
static void s_wiggle_cb(lv_timer_t *t);
static void s_jump_cb(lv_timer_t *t);
static void s_float_text_cb(lv_timer_t *t);
static void s_overlay_talk_cb(lv_timer_t *t);
static void s_feed_bubble_cb(lv_timer_t *t);
static void s_play_bubble_cb(lv_timer_t *t);
static void s_start_wiggle(void);
static void s_start_jump(void);
static void s_show_float_text(const char *text);
static lv_obj_t* s_make_circle_btn(lv_obj_t *parent, const char *icon, const char *text_label,
                                    uint32_t bg_color, uint32_t border_color, uint32_t text_color,
                                    lv_event_cb_t cb, int x_offset,
                                    const lv_font_t *icon_font);

// ========== Sprite Rendering ==========
// 5-line sprites (line 0 = hat slot). 3-frame idle animation.
// Hat is drawn into line 0 when empty; effects on line 0 take priority.
static void s_render_sprite(void)
{
    static uint32_t s_render_count = 0;
    s_render_count++;
    bool do_trace = (s_render_count % 60) == 1;  // ~2.4s at 25fps
    if (do_trace) TRACE_INTERACT_ENTER();

    static char buf[384];
    const SpriteTemplate *tmpl = &SPECIES_TEMPLATES[g_pet.species];
    const char *frame_raw;
    switch ((s_anim_frame / 12) % 3) {
        case 0:  frame_raw = tmpl->frame0; break;
        case 1:  frame_raw = tmpl->frame1; break;
        default: frame_raw = tmpl->frame2; break;
    }
    const char *eye = EYE_STRINGS[g_pet.eye];
    size_t eye_len = strlen(eye);

    // Step 1: substitute eyes into a temp buffer
    static char eye_buf[384];
    size_t eye_out = 0;
    for (const char *p = frame_raw; *p != '\0' && eye_out < sizeof(eye_buf) - 1; p++) {
        if (p[0] == '%' && p[1] == 'c') {
            for (size_t j = 0; j < eye_len && eye_out < sizeof(eye_buf) - 1; j++) {
                eye_buf[eye_out++] = eye[j];
            }
            p++; // skip 'c'
        } else {
            eye_buf[eye_out++] = *p;
        }
    }
    eye_buf[eye_out] = '\0';

    // Step 2: check line 0 (hat slot) — up to first '\n'
    const char *line0_end = strchr(eye_buf, '\n');
    bool line0_empty = true;
    if (line0_end) {
        for (const char *p = eye_buf; p < line0_end; p++) {
            if (*p != ' ') { line0_empty = false; break; }
        }
    }

    // Step 3: if line0 is empty and we have a hat, replace line 0 with hat
    size_t out = 0;
    if (line0_empty && g_pet.hat != HAT_NONE && HAT_LINES[g_pet.hat][0] != '\0') {
        const char *hat = HAT_LINES[g_pet.hat];
        size_t hat_len = strlen(hat);
        for (size_t i = 0; i < hat_len && out < sizeof(buf) - 1; i++) {
            buf[out++] = hat[i];
        }
        // copy remainder starting after line0's '\n'
        if (line0_end) {
            for (const char *p = line0_end; *p != '\0' && out < sizeof(buf) - 1; p++) {
                buf[out++] = *p;
            }
        }
    } else {
        // use as-is
        for (const char *p = eye_buf; *p != '\0' && out < sizeof(buf) - 1; p++) {
            buf[out++] = *p;
        }
    }
    buf[out] = '\0';

    lv_label_set_text(s_label_sprite, buf);
    lv_obj_set_style_text_color(s_label_sprite, lv_color_hex(COLOR_HEX[g_pet.color]), 0);

    if (do_trace) {
        TRACE_INTERACT("sprite species=%d eye=%d hat=%d color=%d frame=%lu",
                       g_pet.species, g_pet.eye, g_pet.hat, g_pet.color,
                       (s_anim_frame / 12) % 3);
        TRACE_INTERACT_EXIT();
    }
}

// ========== Bubble ==========
static void s_hide_bubble_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
    TRACE_INTERACT_EXIT();
}

static void s_show_bubble(const char *text, uint32_t duration_ms)
{
    TRACE_INTERACT_ENTER();
    if (!s_pet_active) {
        TRACE_INTERACT("skipped, page inactive");
        TRACE_INTERACT_EXIT();
        return;
    }
    ui_anim_stop(&s_bubble_timer);
    lv_label_set_text(s_bubble_label, text);
    lv_obj_clear_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
    ui_anim_start_once(&s_bubble_timer, s_hide_bubble_cb, duration_ms);
    if (!s_bubble_timer) {
        TRACE_INTERACT("ERROR: lv_timer_create failed for bubble");
    }
    TRACE_INTERACT("bubble text='%s' duration=%lu", text, duration_ms);
    TRACE_INTERACT_EXIT();
}

// ========== Stats Bars ==========
static void s_update_bars(void)
{
    TRACE_INTERACT_ENTER();
    lv_bar_set_value(s_bar_hunger, g_pet.hunger, LV_ANIM_OFF);
    lv_bar_set_value(s_bar_joy, g_pet.joy, LV_ANIM_OFF);

    static char buf[8];
    snprintf(buf, sizeof(buf), "%d", g_pet.hunger);
    lv_label_set_text(s_label_hunger, buf);
    snprintf(buf, sizeof(buf), "%d", g_pet.joy);
    lv_label_set_text(s_label_joy, buf);
    TRACE_INTERACT("bars hunger=%d joy=%d", g_pet.hunger, g_pet.joy);
    TRACE_INTERACT_EXIT();
}

// ========== Effect Animations ==========
static void s_wiggle_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    static const int8_t angles[] = {0, -80, 80, -40, 0};
    s_wiggle_step++;
    if (s_wiggle_step >= sizeof(angles) / sizeof(angles[0])) {
        lv_obj_set_style_transform_angle(s_label_sprite, 0, 0);
        s_wiggle_step = 0;
        TRACE_INTERACT("wiggle done");
        TRACE_INTERACT_EXIT();
        return;
    }
    lv_obj_set_style_transform_angle(s_label_sprite, angles[s_wiggle_step] * 10, 0);
    lv_obj_set_style_transform_pivot_x(s_label_sprite, 50, 0);
    lv_obj_set_style_transform_pivot_y(s_label_sprite, 100, 0);
    TRACE_INTERACT("wiggle step=%d angle=%d", s_wiggle_step, angles[s_wiggle_step]);
    TRACE_INTERACT_EXIT();
}

static void s_start_wiggle(void)
{
    TRACE_INTERACT_ENTER();
    ui_anim_stop(&s_wiggle_timer);
    s_wiggle_step = 0;
    ui_anim_start_once(&s_wiggle_timer, s_wiggle_cb, 100);
    TRACE_INTERACT_EXIT();
}

static void s_jump_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    static const int8_t offsets[] = {0, -20, -28, -12, 4, 0};
    s_jump_step++;
    if (s_jump_step >= sizeof(offsets) / sizeof(offsets[0])) {
        lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, 0);
        s_jump_step = 0;
        TRACE_INTERACT("jump done");
        TRACE_INTERACT_EXIT();
        return;
    }
    lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, offsets[s_jump_step]);
    TRACE_INTERACT("jump step=%d offset=%d", s_jump_step, offsets[s_jump_step]);
    TRACE_INTERACT_EXIT();
}

static void s_start_jump(void)
{
    TRACE_INTERACT_ENTER();
    ui_anim_stop(&s_jump_timer);
    s_jump_step = 0;
    ui_anim_start_once(&s_jump_timer, s_jump_cb, 90);
    TRACE_INTERACT_EXIT();
}

static void s_float_text_cb(lv_timer_t *t)
{
    (void)t;
    s_float_text_step++;
    if (s_float_text_step >= 8 || !s_float_text_label) {
        if (s_float_text_label) {
            lv_obj_del(s_float_text_label);
            s_float_text_label = NULL;
        }
        s_float_text_step = 0;
        TRACE_INTERACT("float_text done");
        return;
    }
    int y_off = -6 - s_float_text_step * 5;
    lv_opa_t opa = LV_OPA_COVER - (s_float_text_step * 32);
    if (opa < 0) opa = 0;
    lv_obj_align(s_float_text_label, LV_ALIGN_CENTER, 0, y_off);
    lv_obj_set_style_text_opa(s_float_text_label, opa, 0);
    TRACE_INTERACT("float_text step=%d y=%d opa=%d", s_float_text_step, y_off, opa);
}

static void s_show_float_text(const char *text)
{
    TRACE_INTERACT_ENTER();
    if (!s_pet_active) {
        TRACE_INTERACT("skipped, page inactive");
        TRACE_INTERACT_EXIT();
        return;
    }
    if (s_float_text_label) {
        lv_obj_del(s_float_text_label);
    }
    ui_anim_stop(&s_float_text_timer);
    s_float_text_label = lv_label_create(lv_obj_get_parent(s_label_sprite));
    lv_label_set_text(s_float_text_label, text);
    lv_obj_set_style_text_color(s_float_text_label, lv_color_hex(0x3DD9D0), 0);
    lv_obj_set_style_text_font(s_float_text_label, &lv_font_montserrat_12, 0);
    lv_obj_align(s_float_text_label, LV_ALIGN_CENTER, 0, -6);
    s_float_text_step = 0;
    ui_anim_start_once(&s_float_text_timer, s_float_text_cb, 100);
    TRACE_INTERACT("text='%s'", text);
    TRACE_INTERACT_EXIT();
}

static void s_feed_bubble_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    const char **msgs;
    int count;
    if (g_pet.hunger < 30) {
        msgs = FEED_HUNGRY_MSGS;
        count = FEED_HUNGRY_COUNT;
    } else if (g_pet.hunger > 85) {
        msgs = FEED_FULL_MSGS;
        count = FEED_FULL_COUNT;
    } else {
        msgs = FEED_NORMAL_MSGS;
        count = FEED_NORMAL_COUNT;
    }
    s_show_bubble(msgs[pet_rng_range(count)], 1800);
    TRACE_INTERACT_EXIT();
}

static void s_play_bubble_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    s_show_bubble(PLAY_RESPONSES[pet_rng_range(PLAY_RESP_COUNT)], 1800);
    TRACE_INTERACT_EXIT();
}

static void s_auto_bubble_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_pet_active) return;
    if (s_bubble_timer) return;
    s_show_bubble(AUTO_MESSAGES[pet_rng_range(AUTO_MSG_COUNT)], 2500);
}

static void s_overlay_talk_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    s_on_talk(NULL);
    TRACE_INTERACT_EXIT();
}

// ========== Button Handlers ==========
static void s_on_feed(lv_event_t *e)
{
    (void)e;
    TRACE_INTERACT_ENTER();
    TRACE_INTERACT("hunger=%d", g_pet.hunger);
    if (g_pet.hunger >= 100) {
        s_show_bubble("已经很饱了！", 2000);
        TRACE_INTERACT_EXIT();
        return;
    }
    pet_feed(8);
    s_update_bars();
    s_start_wiggle();
    s_show_float_text("+8");
    if (s_feed_bubble_timer) {
        ui_anim_stop(&s_feed_bubble_timer);
    }
    ui_anim_start_once(&s_feed_bubble_timer, s_feed_bubble_cb, 500);
    TRACE_INTERACT("hunger=%d", g_pet.hunger);
    TRACE_INTERACT_EXIT();
}

static void s_on_talk(lv_event_t *e)
{
    (void)e;
    TRACE_INTERACT_ENTER();
    s_show_bubble(IDLE_MESSAGES[pet_rng_range(IDLE_MSG_COUNT)], 3000);
    TRACE_INTERACT_EXIT();
}

static void s_on_play(lv_event_t *e)
{
    (void)e;
    TRACE_INTERACT_ENTER();
    TRACE_INTERACT("joy=%d", g_pet.joy);
    if (g_pet.joy >= 100) {
        s_show_bubble("已经很开心了！", 2000);
        TRACE_INTERACT_EXIT();
        return;
    }
    pet_play(10);
    s_update_bars();
    s_start_jump();
    s_show_float_text("+10");
    if (s_play_bubble_timer) {
        ui_anim_stop(&s_play_bubble_timer);
    }
    ui_anim_start_once(&s_play_bubble_timer, s_play_bubble_cb, 400);
    TRACE_INTERACT("joy=%d", g_pet.joy);
    TRACE_INTERACT_EXIT();
}

// ========== Touch / Long Press ==========
static void s_long_press_cb(lv_timer_t *t)
{
    (void)t;
    TRACE_INTERACT_ENTER();
    s_long_triggered = 1;
    // ui_anim_start_once wrapper auto-deletes the timer after this callback.
    // Clear our pointer so the release handler knows the timer is already done.
    s_long_press_timer = NULL;
    TRACE_INTERACT_EXIT();
}

static void s_on_overlay_pressed(lv_event_t *e)
{
    (void)e;
    TRACE_INTERACT_ENTER();
    s_press_tick = lv_tick_get();
    s_long_triggered = 0;
    ui_anim_stop(&s_long_press_timer);
    ui_anim_start_once(&s_long_press_timer, s_long_press_cb, 600);
    TRACE_INTERACT_EXIT();
}

static void s_on_overlay_released(lv_event_t *e)
{
    (void)e;
    TRACE_INTERACT_ENTER();
    uint32_t elapsed = lv_tick_elaps(s_press_tick);
    TRACE_INTERACT("elapsed=%lu triggered=%d", elapsed, s_long_triggered);
    ui_anim_stop(&s_long_press_timer);
    if (s_long_triggered) {
        s_show_hat_menu();
    } else {
        if (elapsed < 600) {
            ui_anim_stop(&s_defer_timer);
            ui_anim_start_once(&s_defer_timer, s_overlay_talk_cb, 10);
        }
    }
    TRACE_INTERACT_EXIT();
}

// ========== Hat Menu ==========
static void s_on_hat_pick(lv_event_t *e)
{
    TRACE_INTERACT_ENTER();
    lv_obj_t *btn = lv_event_get_current_target(e);
    for (int i = 0; i < HAT_COUNT; i++) {
        if (s_hat_buttons[i] == btn) {
            pet_set_hat((PetHat)i);
            s_render_sprite();
            s_show_bubble(HAT_RESPONSES[pet_rng_range(HAT_RESP_COUNT)], 1800);
            TRACE_INTERACT("hat=%d", i);
            break;
        }
    }
    s_hide_hat_menu();
    TRACE_INTERACT_EXIT();
}

static void s_on_hat_close(lv_event_t *e)
{
    (void)e;
    TRACE_INTERACT_ENTER();
    s_hide_hat_menu();
    TRACE_INTERACT_EXIT();
}

static void s_show_hat_menu(void)
{
    TRACE_INTERACT_ENTER();
    lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_hat_menu);
    lv_obj_clear_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);
    TRACE_INTERACT_EXIT();
}

static void s_hide_hat_menu(void)
{
    TRACE_INTERACT_ENTER();
    lv_obj_add_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    TRACE_INTERACT_EXIT();
}

// ========== Animations ==========
static void s_anim_float_cb(lv_timer_t *t)
{
    (void)t;
    s_anim_frame++;
    s_render_sprite();
    float phase = (float)(s_anim_frame % 62) / 62.0f * 6.28318f;
    int offset = (int)(sinf(phase) * 4.0f);
    lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, offset);
    if (s_anim_frame % 60 == 0) {
        TRACE_INTERACT("float frame=%lu offset=%d", s_anim_frame, offset);
    }
}

// ========== Helper: create circular action button ==========
static lv_obj_t* s_make_circle_btn(lv_obj_t *parent, const char *icon, const char *text_label,
                                    uint32_t bg_color, uint32_t border_color, uint32_t text_color,
                                    lv_event_cb_t cb, int x_offset,
                                    const lv_font_t *icon_font)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 48, 48);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(bg_color), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(border_color), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_align(btn, LV_ALIGN_CENTER, x_offset, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *icon_label = lv_label_create(btn);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_color(icon_label, lv_color_hex(text_color), 0);
    lv_obj_set_style_text_font(icon_label, icon_font, 0);
    lv_obj_center(icon_label);

    lv_obj_t *txt = lv_label_create(parent);
    lv_label_set_text(txt, text_label);
    lv_obj_set_style_text_color(txt, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_font(txt, &lv_font_montserrat_8, 0);
    lv_obj_align(txt, LV_ALIGN_CENTER, x_offset, 32);

    return btn;
}

// ========== Public API ==========
void ui_pet_create(lv_obj_t *parent_tile)
{
    TRACE_INTERACT_ENTER();
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // --- 1. Top info area ---
    s_label_name = lv_label_create(parent_tile);
    lv_label_set_text(s_label_name, g_pet.name);
    lv_obj_set_style_text_color(s_label_name, lv_color_hex(0xE8E8ED), 0);
    lv_obj_set_style_text_font(s_label_name, &font_cjk_14, 0);
    lv_obj_align(s_label_name, LV_ALIGN_TOP_MID, 0, 26);

    s_label_rarity = lv_label_create(parent_tile);
    lv_label_set_text(s_label_rarity, RARITY_STARS[g_pet.rarity]);
    lv_obj_set_style_text_color(s_label_rarity, lv_color_hex(rarity_color(g_pet.rarity)), 0);
    lv_obj_set_style_text_font(s_label_rarity, &lv_font_montserrat_10, 0);
    lv_obj_align(s_label_rarity, LV_ALIGN_TOP_MID, 0, 44);

    // --- 2. Mini stats ---
    lv_obj_t *stats_cont = lv_obj_create(parent_tile);
    lv_obj_set_size(stats_cont, 220, 28);
    lv_obj_set_style_bg_opa(stats_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(stats_cont, 0, 0);
    lv_obj_set_style_pad_all(stats_cont, 0, 0);
    lv_obj_clear_flag(stats_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(stats_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(stats_cont, LV_ALIGN_TOP_MID, 0, 58);

    // Hunger
    lv_obj_t *hunger_icon = lv_label_create(stats_cont);
    lv_label_set_text(hunger_icon, "\xef\xa0\x85"); // burger
    lv_obj_set_style_text_color(hunger_icon, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_font(hunger_icon, &font_cjk_14, 0);
    lv_obj_align(hunger_icon, LV_ALIGN_LEFT_MID, 0, -7);

    s_bar_hunger = lv_bar_create(stats_cont);
    lv_obj_set_size(s_bar_hunger, 180, 3);
    lv_obj_set_style_bg_color(s_bar_hunger, lv_color_hex(0x13131A), LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar_hunger, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar_hunger, lv_color_hex(COLOR_HEX[g_pet.color]), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_bar_hunger, 2, LV_PART_INDICATOR);
    lv_bar_set_range(s_bar_hunger, 0, 100);
    lv_obj_align(s_bar_hunger, LV_ALIGN_LEFT_MID, 16, -7);

    s_label_hunger = lv_label_create(stats_cont);
    lv_label_set_text(s_label_hunger, "0");
    lv_obj_set_style_text_color(s_label_hunger, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_font(s_label_hunger, &lv_font_montserrat_8, 0);
    lv_obj_align(s_label_hunger, LV_ALIGN_RIGHT_MID, 0, -7);

    // Joy
    lv_obj_t *joy_icon = lv_label_create(stats_cont);
    lv_label_set_text(joy_icon, "\xef\x80\x84"); // heart
    lv_obj_set_style_text_color(joy_icon, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_font(joy_icon, &font_cjk_14, 0);
    lv_obj_align(joy_icon, LV_ALIGN_LEFT_MID, 0, 7);

    s_bar_joy = lv_bar_create(stats_cont);
    lv_obj_set_size(s_bar_joy, 180, 3);
    lv_obj_set_style_bg_color(s_bar_joy, lv_color_hex(0x13131A), LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar_joy, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar_joy, lv_color_hex(COLOR_HEX[g_pet.color]), LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_bar_joy, 2, LV_PART_INDICATOR);
    lv_bar_set_range(s_bar_joy, 0, 100);
    lv_obj_align(s_bar_joy, LV_ALIGN_LEFT_MID, 16, 7);

    s_label_joy = lv_label_create(stats_cont);
    lv_label_set_text(s_label_joy, "0");
    lv_obj_set_style_text_color(s_label_joy, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_font(s_label_joy, &lv_font_montserrat_8, 0);
    lv_obj_align(s_label_joy, LV_ALIGN_RIGHT_MID, 0, 7);

    s_update_bars();

    // --- 3. Speech bubble ---
    s_bubble = lv_obj_create(parent_tile);
    lv_obj_set_size(s_bubble, 200, 40);
    lv_obj_set_style_radius(s_bubble, 10, 0);
    lv_obj_set_style_bg_color(s_bubble, lv_color_hex(0x13131A), 0);
    lv_obj_set_style_bg_opa(s_bubble, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_bubble, lv_color_hex(0x252530), 0);
    lv_obj_set_style_border_width(s_bubble, 1, 0);
    lv_obj_set_style_pad_all(s_bubble, 4, 0);
    lv_obj_align(s_bubble, LV_ALIGN_TOP_MID, 0, 86);
    lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_bubble, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_bubble, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(s_bubble, LV_OBJ_FLAG_CLICKABLE);

    s_bubble_label = lv_label_create(s_bubble);
    lv_label_set_text(s_bubble_label, "");
    lv_obj_set_style_text_color(s_bubble_label, lv_color_hex(0xE8E8ED), 0);
    lv_obj_set_style_text_font(s_bubble_label, &font_cjk_14, 0);
    lv_label_set_long_mode(s_bubble_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_bubble_label, 192);
    lv_obj_center(s_bubble_label);

    // --- 4. Pet sprite ---
    s_label_sprite = lv_label_create(parent_tile);
    lv_label_set_text(s_label_sprite, "");
    lv_obj_set_style_text_color(s_label_sprite, lv_color_hex(COLOR_HEX[g_pet.color]), 0);
    lv_obj_set_style_text_font(s_label_sprite, &font_mono_20, 0);
    lv_obj_set_style_text_align(s_label_sprite, LV_TEXT_ALIGN_CENTER, 0);
    // NOTE: Do NOT use LONG_DOT for multi-line ASCII art; it breaks layout.
    // Position is controlled by animation timer, not here.
    lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, 0);

    // --- 5. Overlay for touch detection (created BEFORE action bar so buttons are on top) ---
    // Overlay covers only the pet sprite + hat area (~160x140 centered at y=160)
    // This prevents false long-press triggers on action bar / stats while
    // still allowing short-press talk and long-press hat detection.
    s_overlay = lv_obj_create(parent_tile);
    lv_obj_set_size(s_overlay, 160, 140);
    lv_obj_set_style_bg_opa(s_overlay, LV_OPA_0, 0);
    lv_obj_set_style_border_width(s_overlay, 0, 0);
    lv_obj_set_style_pad_all(s_overlay, 0, 0);
    lv_obj_align(s_overlay, LV_ALIGN_CENTER, 0, -20);
    lv_obj_add_event_cb(s_overlay, s_on_overlay_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(s_overlay, s_on_overlay_released, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // --- 6. Action bar (created AFTER overlay so buttons are on top in z-order) ---
    lv_obj_t *action_cont = lv_obj_create(parent_tile);
    lv_obj_set_size(action_cont, 200, 56);
    lv_obj_set_style_bg_opa(action_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(action_cont, 0, 0);
    lv_obj_set_style_pad_all(action_cont, 0, 0);
    lv_obj_clear_flag(action_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(action_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(action_cont, LV_ALIGN_BOTTOM_MID, 0, -28);

    // Left - FEED
    s_btn_feed = s_make_circle_btn(action_cont, "\xef\x97\x91", "FEED",
                                    0x13131A, 0x252530, 0xE8E8ED,
                                    s_on_feed, -52,
                                    &font_cjk_14);

    // Center - TALK
    s_btn_talk = s_make_circle_btn(action_cont, "\xef\x89\xba", "TALK",
                                    0x3DD9D0, 0x3DD9D0, 0x3DD9D0,
                                    s_on_talk, 0,
                                    &font_cjk_14);
    lv_obj_set_style_bg_opa(s_btn_talk, LV_OPA_20, 0);

    // Right - PLAY
    s_btn_play = s_make_circle_btn(action_cont, "\xef\x90\xb2", "PLAY",
                                    0x13131A, 0x252530, 0xE8E8ED,
                                    s_on_play, 52,
                                    &font_cjk_14);

    // --- 7. Hat picker menu (full-screen modal) ---
    s_hat_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_hat_menu, 360, 360);
    lv_obj_set_style_radius(s_hat_menu, 0, 0);
    lv_obj_set_style_bg_color(s_hat_menu, lv_color_hex(0x0A0A0F), 0);
    lv_obj_set_style_bg_opa(s_hat_menu, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_hat_menu, 0, 0);
    lv_obj_set_style_pad_all(s_hat_menu, 0, 0);
    lv_obj_clear_flag(s_hat_menu, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_hat_menu, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(s_hat_menu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *hat_title = lv_label_create(s_hat_menu);
    lv_label_set_text(hat_title, "选择帽子");
    lv_obj_set_style_text_color(hat_title, lv_color_hex(0xE8E8ED), 0);
    lv_obj_set_style_text_font(hat_title, &font_cjk_14, 0);
    lv_obj_align(hat_title, LV_ALIGN_TOP_MID, 0, 20);

    s_hat_grid = lv_obj_create(s_hat_menu);
    lv_obj_set_size(s_hat_grid, 320, 260);
    lv_obj_set_style_bg_opa(s_hat_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_hat_grid, 0, 0);
    lv_obj_set_style_pad_all(s_hat_grid, 8, 0);
    lv_obj_set_flex_flow(s_hat_grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(s_hat_grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(s_hat_grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_hat_grid, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(s_hat_grid, LV_ALIGN_CENTER, 0, -8);

    for (int i = 0; i < HAT_COUNT; i++) {
        lv_obj_t *btn = lv_btn_create(s_hat_grid);
        lv_obj_set_size(btn, 136, 56);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x13131A), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_add_event_cb(btn, s_on_hat_pick, LV_EVENT_CLICKED, NULL);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

        // Hat text (mono font, centered)
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, HAT_LINES[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE8E8ED), 0);
        lv_obj_set_style_text_font(lbl, &font_mono_16, 0);
        lv_obj_center(lbl);

        // Hat name below the ASCII art
        lv_obj_t *name_lbl = lv_label_create(btn);
        lv_label_set_text(name_lbl, HAT_NAMES[i]);
        lv_obj_set_style_text_color(name_lbl, lv_color_hex(0x6B6B78), 0);
        lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_10, 0);
        lv_obj_align(name_lbl, LV_ALIGN_BOTTOM_MID, 0, -2);

        s_hat_buttons[i] = btn;
    }

    lv_obj_t *close_btn = lv_btn_create(s_hat_menu);
    lv_obj_set_size(close_btn, 100, 36);
    lv_obj_set_style_radius(close_btn, 10, 0);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x13131A), 0);
    lv_obj_set_style_bg_opa(close_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(close_btn, lv_color_hex(0x252530), 0);
    lv_obj_set_style_border_width(close_btn, 1, 0);
    lv_obj_set_style_pad_all(close_btn, 0, 0);
    lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(close_btn, s_on_hat_close, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(close_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *close_lbl = lv_label_create(close_btn);
    lv_label_set_text(close_lbl, "取消");
    lv_obj_set_style_text_color(close_lbl, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_font(close_lbl, &font_cjk_14, 0);
    lv_obj_center(close_lbl);

    // --- Initial render ---
    s_anim_frame = 0;
    s_render_sprite();

    // --- Timers ---
    ui_anim_register(&s_timer_float);
    ui_anim_register(&s_auto_bubble_timer);
    ui_anim_register(&s_bubble_timer);
    ui_anim_register(&s_wiggle_timer);
    ui_anim_register(&s_jump_timer);
    ui_anim_register(&s_float_text_timer);
    ui_anim_register(&s_feed_bubble_timer);
    ui_anim_register(&s_play_bubble_timer);
    ui_anim_register(&s_long_press_timer);
    ui_anim_register(&s_defer_timer);
    ui_anim_start_loop(&s_timer_float, s_anim_float_cb, 40);
    ui_anim_start_loop(&s_auto_bubble_timer, s_auto_bubble_cb, 45000);
    TRACE_INTERACT_EXIT();
}

void ui_pet_update(void)
{
    TRACE_INTERACT_ENTER();
    uint32_t c = COLOR_HEX[g_pet.color];
    lv_obj_set_style_bg_color(s_bar_hunger, lv_color_hex(c), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s_bar_joy, lv_color_hex(c), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(s_label_sprite, lv_color_hex(c), 0);
    TRACE_INTERACT_EXIT();
}

void ui_pet_pause_anim(void)
{
    TRACE_INTERACT_ENTER();
    s_pet_active = false;

    ui_anim_pause_all();

    /* Hide bubble */
    lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);

    /* Stop all one-shot effect timers */
    ui_anim_stop(&s_bubble_timer);
    ui_anim_stop(&s_defer_timer);
    ui_anim_stop(&s_long_press_timer);
    s_press_tick = 0;
    s_long_triggered = 0;
    ui_anim_stop(&s_wiggle_timer);
    s_wiggle_step = 0;
    lv_obj_set_style_transform_angle(s_label_sprite, 0, 0);
    ui_anim_stop(&s_jump_timer);
    s_jump_step = 0;
    lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, 0);
    ui_anim_stop(&s_float_text_timer);
    if (s_float_text_label) {
        lv_obj_del(s_float_text_label);
        s_float_text_label = NULL;
    }
    s_float_text_step = 0;
    ui_anim_stop(&s_feed_bubble_timer);
    ui_anim_stop(&s_play_bubble_timer);

    /* Hide hat menu */
    s_hide_hat_menu();

    TRACE_INTERACT_EXIT();
}

void ui_pet_resume_anim(void)
{
    s_pet_active = true;
    ui_anim_resume_all();
    // Greeting bubble when entering the page
    if (!s_bubble_timer) {
        s_show_bubble(ENTER_GREETINGS[pet_rng_range(ENTER_GREET_COUNT)], 2000);
    }
}

void ui_pet_delete(void)
{
    TRACE_INTERACT_ENTER();
    ui_anim_stop_all();
    TRACE_INTERACT_EXIT();
}
