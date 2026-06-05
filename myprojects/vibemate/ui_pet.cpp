#include "ui_pet.h"
#include "pet_sprites.h"
#include <stdio.h>
#include <string.h>

// ========== UI Objects ==========
static lv_obj_t *s_label_name;
static lv_obj_t *s_label_rarity;
static lv_obj_t *s_label_sprite;
static lv_obj_t *s_bubble;
static lv_obj_t *s_bubble_label;
static lv_obj_t *s_overlay;

// ========== State Machine ==========
typedef enum {
    STATE_IDLE = 0,
    STATE_BLINK,
    STATE_HAPPY,
    STATE_TALKING,
} PetState;

static PetState s_state = STATE_IDLE;
static int s_frame = 0;
static int s_state_tick = 0;
static lv_timer_t *s_anim_timer = NULL;

// ========== Config ==========
#define ANIM_INTERVAL_MS    400
#define IDLE_SEQUENCE_LEN   15
static const int8_t IDLE_SEQUENCE[IDLE_SEQUENCE_LEN] = {
    0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 2, 0, 0, 0
};

static const char *IDLE_MESSAGES[] = {
    "Coding hard...",
    "Take a break~",
    "Stay hydrated!",
    "You got this!",
    "Hello world!",
    "Bug or feature?",
    "Stay positive!",
    "Keep vibing!"
};
static const int IDLE_MSG_COUNT = sizeof(IDLE_MESSAGES) / sizeof(IDLE_MESSAGES[0]);

static const char *HAPPY_MESSAGES[] = {
    "I love you!",
    "Best friend!",
    "So happy!",
    "You're awesome!",
    "*happy noises*"
};
static const int HAPPY_MSG_COUNT = sizeof(HAPPY_MESSAGES) / sizeof(HAPPY_MESSAGES[0]);

// ========== Rendering ==========
static void render_sprite(char *buf, size_t len, int frame_index, char eye_char, bool blink)
{
    const SpriteTemplate *tmpl = &SPECIES_TEMPLATES[g_pet.species];
    const char *tmpl_str;
    switch (frame_index % 3) {
        case 0: tmpl_str = tmpl->frame0; break;
        case 1: tmpl_str = tmpl->frame1; break;
        default: tmpl_str = tmpl->frame2; break;
    }

    char eye = blink ? '-' : eye_char;

    if (tmpl->eye_count == 2) {
        snprintf(buf, len, tmpl_str, eye, eye);
    } else if (tmpl->eye_count == 1) {
        snprintf(buf, len, tmpl_str, eye);
    } else {
        strncpy(buf, tmpl_str, len);
        buf[len - 1] = '\0';
    }
}

static void update_sprite(void)
{
    static char buf[128];
    int seq_idx = s_frame % IDLE_SEQUENCE_LEN;
    int8_t seq_val = IDLE_SEQUENCE[seq_idx];

    int frame_idx;
    bool blink = false;

    if (s_state == STATE_HAPPY) {
        frame_idx = 1;
        blink = false;
    } else if (s_state == STATE_BLINK) {
        frame_idx = 0;
        blink = true;
    } else {
        if (seq_val < 0) {
            frame_idx = 0;
            blink = true;
        } else {
            frame_idx = seq_val;
            blink = false;
        }
    }

    render_sprite(buf, sizeof(buf), frame_idx, EYE_CHARS[g_pet.eye], blink);
    lv_label_set_text(s_label_sprite, buf);
}

static void show_bubble(const char *text)
{
    lv_label_set_text(s_bubble_label, text);
    lv_obj_clear_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
}

static void hide_bubble(void)
{
    lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
}

static void set_state(PetState state)
{
    s_state = state;
    s_state_tick = 0;
    if (state == STATE_IDLE) {
        hide_bubble();
    }
}

// ========== Animation Timer ==========
static void anim_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    s_frame++;
    s_state_tick++;

    switch (s_state) {
        case STATE_IDLE: {
            if (s_state_tick > 20 && pet_rng_range(100) < 3) {
                const char *msg = IDLE_MESSAGES[pet_rng_range(IDLE_MSG_COUNT)];
                show_bubble(msg);
                set_state(STATE_TALKING);
                break;
            }
            if (pet_rng_range(100) < 2) {
                set_state(STATE_BLINK);
                break;
            }
            update_sprite();
            break;
        }

        case STATE_BLINK: {
            update_sprite();
            if (s_state_tick >= 1) {
                set_state(STATE_IDLE);
                update_sprite();
            }
            break;
        }

        case STATE_HAPPY: {
            update_sprite();
            if (s_state_tick >= 5) {
                set_state(STATE_IDLE);
                update_sprite();
            }
            break;
        }

        case STATE_TALKING: {
            update_sprite();
            if (s_state_tick >= 10) {
                set_state(STATE_IDLE);
                update_sprite();
            }
            break;
        }
    }
}

// ========== Touch Handler ==========
static void pet_event_cb(lv_event_t *e)
{
    (void)e;
    if (s_state == STATE_HAPPY) return;
    const char *msg = HAPPY_MESSAGES[pet_rng_range(HAPPY_MSG_COUNT)];
    show_bubble(msg);
    set_state(STATE_HAPPY);
    update_sprite();
}

// ========== Public API ==========
void ui_pet_create(lv_obj_t *parent_tile)
{
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // Name label (top)
    s_label_name = lv_label_create(parent_tile);
    lv_label_set_text(s_label_name, g_pet.name);
    lv_obj_set_style_text_color(s_label_name, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(s_label_name, &lv_font_montserrat_16, 0);
    lv_obj_align(s_label_name, LV_ALIGN_TOP_MID, 0, 30);

    // Rarity stars (below name)
    s_label_rarity = lv_label_create(parent_tile);
    static char rarity_buf[48];
    snprintf(rarity_buf, sizeof(rarity_buf), "%s  %s",
             RARITY_NAMES[g_pet.rarity], RARITY_STARS[g_pet.rarity]);
    lv_label_set_text(s_label_rarity, rarity_buf);
    lv_obj_set_style_text_color(s_label_rarity, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(s_label_rarity, &lv_font_montserrat_12, 0);
    lv_obj_align(s_label_rarity, LV_ALIGN_TOP_MID, 0, 52);

    // Speech bubble (hidden by default, above sprite)
    s_bubble = lv_obj_create(parent_tile);
    lv_obj_set_size(s_bubble, 220, 50);
    lv_obj_set_style_radius(s_bubble, 8, 0);
    lv_obj_set_style_bg_color(s_bubble, lv_color_hex(0x1e293b), 0);
    lv_obj_set_style_bg_opa(s_bubble, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_bubble, lv_color_hex(0x475569), 0);
    lv_obj_set_style_border_width(s_bubble, 1, 0);
    lv_obj_set_style_pad_all(s_bubble, 4, 0);
    lv_obj_set_style_pad_top(s_bubble, 10, 0);
    lv_obj_align(s_bubble, LV_ALIGN_TOP_MID, 0, 76);
    lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);

    s_bubble_label = lv_label_create(s_bubble);
    lv_label_set_text(s_bubble_label, "");
    lv_obj_set_style_text_color(s_bubble_label, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(s_bubble_label, &lv_font_montserrat_12, 0);
    lv_label_set_long_mode(s_bubble_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_bubble_label, 210);
    lv_obj_center(s_bubble_label);

    // Sprite label (center, larger font)
    s_label_sprite = lv_label_create(parent_tile);
    lv_obj_set_style_text_color(s_label_sprite, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(s_label_sprite, &font_pet_20, 0);
    lv_label_set_text(s_label_sprite, "^_^");
    lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, 10);

    // Overlay for touch detection
    s_overlay = lv_obj_create(parent_tile);
    lv_obj_set_size(s_overlay, 360, 360);
    lv_obj_set_style_bg_opa(s_overlay, LV_OPA_0, 0);
    lv_obj_set_style_border_width(s_overlay, 0, 0);
    lv_obj_set_style_pad_all(s_overlay, 0, 0);
    lv_obj_align(s_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(s_overlay, pet_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // Initial render
    s_frame = 0;
    s_state = STATE_IDLE;
    s_state_tick = 0;
    update_sprite();

    // Animation timer
    s_anim_timer = lv_timer_create(anim_timer_cb, ANIM_INTERVAL_MS, NULL);
}

void ui_pet_update(void)
{
    // 1-second updates if needed
}
