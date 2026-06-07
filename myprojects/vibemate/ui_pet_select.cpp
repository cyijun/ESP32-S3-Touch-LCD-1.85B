#include "ui_pet_select.h"
#include "ui_pet_detail.h"
#include "ui_pet.h"
#include "pet_sprites.h"
#include "pet_storage.h"
#include <Esp.h>
#include <string.h>

// ========== Mini icons for side peeks ==========
static const char* MINI_ICONS[PET_SPECIES_COUNT] = {
    "D", "G", "B", "C", "N", "8", "o", "P", "T", "@",
    "~", "A", "K", "X", "R", "r", "M", "+"
};

// ========== Simplified face previews ==========
static const char* FACE_PREVIEWS[PET_SPECIES_COUNT] = {
    "<(·)", "(·>)", "(··)", "(=ω=)", "<~~>",
    "~(··)~", "(··)", "(·>)", "[_ _]", "·(@)",
    "/··\\", "}··{", "(oo)", "|··|", "[··]",
    "(..)", "|··|", "(..)"
};

// ========== State variables ==========
static int s_sel_species = 0;
static int s_sel_eye = 0;
static int s_sel_hat = 0;
static int s_sel_color = 0;

// ========== UI references ==========
static lv_obj_t *s_preview_arc;
static lv_obj_t *s_preview_sprite;
static lv_obj_t *s_hat_badge;
static lv_obj_t *s_label_title;
static lv_obj_t *s_label_subtitle;
static lv_obj_t *s_label_species_name;
static lv_obj_t *s_label_rarity;
static lv_obj_t *s_left_peek;
static lv_obj_t *s_right_peek;
static lv_obj_t *s_eye_btns[EYE_COUNT];
static lv_obj_t *s_color_btns[COLOR_COUNT];

// ========== Forward declarations ==========
static void update_preview(void);
static void update_peeks(void);
static void update_species_info(void);
static void apply_and_save(void);
static void on_prev_click(lv_event_t *e);
static void on_next_click(lv_event_t *e);
static void on_generate_click(lv_event_t *e);
static void on_eye_click(lv_event_t *e);
static void on_color_click(lv_event_t *e);

// ========== Helpers ==========
static PetRarity rarity_for_species(int species)
{
    uint32_t seed = (uint32_t)(species * 7919);
    uint32_t r = seed % 100;
    if (r < 60) return RARITY_COMMON;
    if (r < 85) return RARITY_UNCOMMON;
    if (r < 95) return RARITY_RARE;
    if (r < 99) return RARITY_EPIC;
    return RARITY_LEGENDARY;
}

static void build_face(char *buf, size_t buf_size, int species, int eye)
{
    const char *template_str = FACE_PREVIEWS[species];
    const char *eye_str = EYE_STRINGS[eye];
    size_t out = 0;
    size_t eye_len = strlen(eye_str);
    size_t tmpl_len = strlen(template_str);

    for (size_t i = 0; i < tmpl_len && out < buf_size - 1; ) {
        // Match UTF-8 middle dot U+00B7 = C2 B7
        if ((unsigned char)template_str[i] == 0xC2 &&
            i + 1 < tmpl_len &&
            (unsigned char)template_str[i + 1] == 0xB7) {
            for (size_t j = 0; j < eye_len && out < buf_size - 1; j++) {
                buf[out++] = eye_str[j];
            }
            i += 2;
        } else {
            buf[out++] = template_str[i++];
        }
    }
    buf[out] = '\0';
}

static void apply_and_save(void)
{
    TRACE_SELECT_ENTER();
    g_pet.species = (PetSpecies)s_sel_species;
    g_pet.eye = (PetEye)s_sel_eye;
    g_pet.hat = (PetHat)s_sel_hat;
    g_pet.color = (PetColor)s_sel_color;
    pet_save();
    ui_pet_detail_update();
    ui_pet_update();
}

// ========== Update functions ==========
static void update_preview(void)
{
    TRACE_SELECT_ENTER();
    // Update arc color
    lv_obj_set_style_arc_color(s_preview_arc, lv_color_hex(COLOR_HEX[s_sel_color]), LV_PART_INDICATOR);

    // Update hat badge
    if (s_sel_hat == HAT_NONE) {
        lv_obj_add_flag(s_hat_badge, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(s_hat_badge, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_hat_badge, HAT_LINES[s_sel_hat]);
        lv_obj_set_style_text_color(s_hat_badge, lv_color_hex(COLOR_HEX[s_sel_color]), 0);
    }

    // Update face preview
    char face_buf[32];
    build_face(face_buf, sizeof(face_buf), s_sel_species, s_sel_eye);
    lv_label_set_text(s_preview_sprite, face_buf);
    lv_obj_set_style_text_color(s_preview_sprite, lv_color_hex(COLOR_HEX[s_sel_color]), 0);
}

static void update_peeks(void)
{
    TRACE_SELECT_ENTER();
    int prev = (s_sel_species - 1 + PET_SPECIES_COUNT) % PET_SPECIES_COUNT;
    int next = (s_sel_species + 1) % PET_SPECIES_COUNT;
    lv_label_set_text(s_left_peek, MINI_ICONS[prev]);
    lv_label_set_text(s_right_peek, MINI_ICONS[next]);
}

static void update_species_info(void)
{
    TRACE_SELECT_ENTER();
    lv_label_set_text(s_label_species_name, SPECIES_NAMES[s_sel_species]);
    PetRarity r = rarity_for_species(s_sel_species);
    lv_label_set_text(s_label_rarity, RARITY_STARS[r]);
}

static void update_eye_buttons(void)
{
    TRACE_SELECT_ENTER();
    for (int i = 0; i < EYE_COUNT; i++) {
        if (i == s_sel_eye) {
            lv_obj_set_style_border_color(s_eye_btns[i], lv_color_hex(0x3DD9D0), 0);
            lv_obj_set_style_bg_color(s_eye_btns[i], lv_color_hex(0x3DD9D0), 0);
            lv_obj_set_style_bg_opa(s_eye_btns[i], LV_OPA_20, 0);
        } else {
            lv_obj_set_style_border_color(s_eye_btns[i], lv_color_hex(0x252530), 0);
            lv_obj_set_style_bg_color(s_eye_btns[i], lv_color_hex(0x13131A), 0);
            lv_obj_set_style_bg_opa(s_eye_btns[i], LV_OPA_COVER, 0);
        }
    }
}

static void update_color_buttons(void)
{
    TRACE_SELECT_ENTER();
    for (int i = 0; i < COLOR_COUNT; i++) {
        if (i == s_sel_color) {
            lv_obj_set_style_border_width(s_color_btns[i], 2, 0);
            lv_obj_set_style_border_color(s_color_btns[i], lv_color_hex(0xE8E8ED), 0);
        } else {
            lv_obj_set_style_border_width(s_color_btns[i], 1, 0);
            lv_obj_set_style_border_color(s_color_btns[i], lv_color_hex(0x252530), 0);
        }
    }
}

// ========== Event handlers ==========
static void on_prev_click(lv_event_t *e)
{
    (void)e;
    TRACE_SELECT_ENTER();
    TRACE_SELECT("prev species=%d", s_sel_species);
    s_sel_species = (s_sel_species - 1 + PET_SPECIES_COUNT) % PET_SPECIES_COUNT;
    update_preview();
    update_peeks();
    update_species_info();
    apply_and_save();
    TRACE_SELECT_EXIT();
}

static void on_next_click(lv_event_t *e)
{
    (void)e;
    TRACE_SELECT_ENTER();
    TRACE_SELECT("next species=%d", s_sel_species);
    s_sel_species = (s_sel_species + 1) % PET_SPECIES_COUNT;
    update_preview();
    update_peeks();
    update_species_info();
    apply_and_save();
    TRACE_SELECT_EXIT();
}

static void on_generate_click(lv_event_t *e)
{
    (void)e;
    TRACE_SELECT_ENTER();
    TRACE_SELECT("generate click");
    uint64_t mac = ESP.getEfuseMac();
    uint32_t seed = (uint32_t)(mac + (uint64_t)s_sel_species * 7919ULL);
    // Use local RNG to avoid depending on pet_sprites.cpp internal state
    uint32_t rng = seed;
    auto rng_next = [&rng]() { rng = rng * 1103515245u + 12345u; return rng; };
    auto rng_range = [&rng](uint32_t max) { rng = rng * 1103515245u + 12345u; return rng % max; };
    s_sel_eye = (int)rng_range(EYE_COUNT);
    s_sel_hat = (int)rng_range(HAT_COUNT);
    s_sel_color = (int)rng_range(COLOR_COUNT);
    update_preview();
    update_eye_buttons();
    update_color_buttons();
    apply_and_save();
    TRACE_SELECT("generated eye=%d hat=%d color=%d", s_sel_eye, s_sel_hat, s_sel_color);
    TRACE_SELECT_EXIT();
}

static void on_eye_click(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    TRACE_SELECT_ENTER();
    for (int i = 0; i < EYE_COUNT; i++) {
        if (s_eye_btns[i] == btn) {
            s_sel_eye = i;
            TRACE_SELECT("eye=%d", s_sel_eye);
            break;
        }
    }
    update_eye_buttons();
    update_preview();
    apply_and_save();
}

static void on_color_click(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    TRACE_SELECT_ENTER();
    for (int i = 0; i < COLOR_COUNT; i++) {
        if (s_color_btns[i] == btn) {
            s_sel_color = i;
            TRACE_SELECT("color=%d", s_sel_color);
            break;
        }
    }
    update_color_buttons();
    update_preview();
    apply_and_save();
}

// ========== Create ==========
void ui_pet_select_create(lv_obj_t *parent_tile)
{
    TRACE_SELECT_ENTER();
    TRACE_SELECT_HEAP();
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);
    lv_obj_clear_flag(parent_tile, LV_OBJ_FLAG_SCROLLABLE);

    // Load current pet state
    s_sel_species = g_pet.species;
    s_sel_eye = g_pet.eye;
    s_sel_hat = g_pet.hat;
    s_sel_color = g_pet.color;

    // --- Header ---
    s_label_title = lv_label_create(parent_tile);
    lv_label_set_text(s_label_title, "选择伙伴");
    lv_obj_set_style_text_font(s_label_title, &font_cjk_14, 0);
    lv_obj_set_style_text_color(s_label_title, lv_color_hex(0xE8E8ED), 0);
    lv_obj_align(s_label_title, LV_ALIGN_TOP_MID, 0, 8);

    s_label_subtitle = lv_label_create(parent_tile);
    lv_label_set_text(s_label_subtitle, "自定义你的 Buddy");
    lv_obj_set_style_text_font(s_label_subtitle, &font_cjk_14, 0);
    lv_obj_set_style_text_color(s_label_subtitle, lv_color_hex(0x6B6B78), 0);
    lv_obj_align(s_label_subtitle, LV_ALIGN_TOP_MID, 0, 26);

    // --- Side peeks ---
    s_left_peek = lv_label_create(parent_tile);
    lv_obj_set_style_text_font(s_left_peek, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_left_peek, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_opa(s_left_peek, LV_OPA_50, 0);
    lv_obj_align(s_left_peek, LV_ALIGN_LEFT_MID, 12, -60);

    s_right_peek = lv_label_create(parent_tile);
    lv_obj_set_style_text_font(s_right_peek, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_right_peek, lv_color_hex(0x6B6B78), 0);
    lv_obj_set_style_text_opa(s_right_peek, LV_OPA_50, 0);
    lv_obj_align(s_right_peek, LV_ALIGN_RIGHT_MID, -12, -60);

    // --- Preview stage (center) ---
    // Preview ring: lv_arc, 100x100, full circle, 3px width
    s_preview_arc = lv_arc_create(parent_tile);
    lv_obj_set_size(s_preview_arc, 100, 100);
    lv_arc_set_rotation(s_preview_arc, 270);
    lv_arc_set_bg_angles(s_preview_arc, 0, 360);
    lv_arc_set_angles(s_preview_arc, 0, 360);
    lv_arc_set_value(s_preview_arc, 100);
    lv_obj_set_style_arc_width(s_preview_arc, 3, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_preview_arc, 3, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_preview_arc, lv_color_hex(0x252530), LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_preview_arc, lv_color_hex(COLOR_HEX[s_sel_color]), LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(s_preview_arc, true, LV_PART_INDICATOR);
    lv_obj_remove_style(s_preview_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(s_preview_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(s_preview_arc, LV_ALIGN_CENTER, 0, -60);

    // Hat badge above preview
    s_hat_badge = lv_label_create(parent_tile);
    lv_obj_set_style_text_font(s_hat_badge, &font_cjk_14, 0);
    lv_obj_set_style_text_color(s_hat_badge, lv_color_hex(COLOR_HEX[s_sel_color]), 0);
    lv_obj_align(s_hat_badge, LV_ALIGN_CENTER, 0, -116);

    // Preview sprite centered in ring
    s_preview_sprite = lv_label_create(parent_tile);
    lv_obj_set_style_text_font(s_preview_sprite, &font_mono_16, 0);
    lv_obj_set_style_text_color(s_preview_sprite, lv_color_hex(0xE8E8ED), 0);
    lv_obj_align(s_preview_sprite, LV_ALIGN_CENTER, 0, -60);

    // --- Species name + rarity ---
    s_label_species_name = lv_label_create(parent_tile);
    lv_obj_set_style_text_font(s_label_species_name, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(s_label_species_name, lv_color_hex(0xE8E8ED), 0);
    lv_obj_align(s_label_species_name, LV_ALIGN_TOP_MID, 0, 130);

    s_label_rarity = lv_label_create(parent_tile);
    lv_obj_set_style_text_font(s_label_rarity, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(s_label_rarity, lv_color_hex(0x6B6B78), 0);
    lv_obj_align(s_label_rarity, LV_ALIGN_TOP_MID, 0, 146);

    // --- Eye selector (Y=164, container 280x40) ---
    lv_obj_t *eye_cont = lv_obj_create(parent_tile);
    lv_obj_set_size(eye_cont, 280, 40);
    lv_obj_set_pos(eye_cont, (360 - 280) / 2, 164);
    lv_obj_set_style_bg_opa(eye_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(eye_cont, 0, 0);
    lv_obj_set_style_pad_all(eye_cont, 0, 0);
    lv_obj_clear_flag(eye_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(eye_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *eye_icon = lv_label_create(eye_cont);
    lv_label_set_text(eye_icon, "◉");
    lv_obj_set_style_text_font(eye_icon, &font_mono_16, 0);
    lv_obj_set_style_text_color(eye_icon, lv_color_hex(0x6B6B78), 0);
    lv_obj_align(eye_icon, LV_ALIGN_LEFT_MID, 4, 0);

    int eye_start_x = 36;
    int eye_gap = (280 - eye_start_x - 36) / (EYE_COUNT - 1);
    for (int i = 0; i < EYE_COUNT; i++) {
        lv_obj_t *btn = lv_obj_create(eye_cont);
        lv_obj_set_size(btn, 36, 36);
        lv_obj_set_pos(btn, eye_start_x + i * eye_gap, 2);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x13131A), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, EYE_STRINGS[i]);
        lv_obj_set_style_text_font(lbl, &font_mono_16, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE8E8ED), 0);
        lv_obj_center(lbl);

        lv_obj_add_event_cb(btn, on_eye_click, LV_EVENT_CLICKED, NULL);
        s_eye_btns[i] = btn;
    }

    // --- Color selector (Y=208, container 280x28) ---
    lv_obj_t *color_cont = lv_obj_create(parent_tile);
    lv_obj_set_size(color_cont, 280, 28);
    lv_obj_set_pos(color_cont, (360 - 280) / 2, 208);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(color_cont, 0, 0);
    lv_obj_set_style_pad_all(color_cont, 0, 0);
    lv_obj_clear_flag(color_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(color_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *color_icon = lv_label_create(color_cont);
    lv_label_set_text(color_icon, "●");
    lv_obj_set_style_text_font(color_icon, &font_cjk_14, 0);
    lv_obj_set_style_text_color(color_icon, lv_color_hex(0x6B6B78), 0);
    lv_obj_align(color_icon, LV_ALIGN_LEFT_MID, 4, 0);

    int color_start_x = 30;
    int color_gap = (280 - color_start_x - 22) / (COLOR_COUNT - 1);
    for (int i = 0; i < COLOR_COUNT; i++) {
        lv_obj_t *btn = lv_obj_create(color_cont);
        lv_obj_set_size(btn, 22, 22);
        lv_obj_set_pos(btn, color_start_x + i * color_gap, 3);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_HEX[i]), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_add_event_cb(btn, on_color_click, LV_EVENT_CLICKED, NULL);
        s_color_btns[i] = btn;
    }

    // --- Action bar (bottom, Y=-8 from bottom, container 192x60) ---
    lv_obj_t *action_cont = lv_obj_create(parent_tile);
    lv_obj_set_size(action_cont, 192, 60);
    lv_obj_align(action_cont, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_opa(action_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(action_cont, 0, 0);
    lv_obj_set_style_pad_all(action_cont, 0, 0);
    lv_obj_clear_flag(action_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(action_cont, LV_SCROLLBAR_MODE_OFF);

    // Left "<" button: 48x48
    lv_obj_t *btn_prev = lv_obj_create(action_cont);
    lv_obj_set_size(btn_prev, 48, 48);
    lv_obj_align(btn_prev, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_radius(btn_prev, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_prev, lv_color_hex(0x13131A), 0);
    lv_obj_set_style_border_width(btn_prev, 1, 0);
    lv_obj_set_style_border_color(btn_prev, lv_color_hex(0x252530), 0);
    lv_obj_set_style_pad_all(btn_prev, 0, 0);
    lv_obj_clear_flag(btn_prev, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_prev, on_prev_click, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_prev = lv_label_create(btn_prev);
    lv_label_set_text(lbl_prev, "<");
    lv_obj_set_style_text_font(lbl_prev, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_prev, lv_color_hex(0xE8E8ED), 0);
    lv_obj_center(lbl_prev);

    // Center "↻" button: 56x56
    lv_obj_t *btn_gen = lv_obj_create(action_cont);
    lv_obj_set_size(btn_gen, 56, 56);
    lv_obj_align(btn_gen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(btn_gen, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_gen, lv_color_hex(0x3DD9D0), 0);
    lv_obj_set_style_bg_opa(btn_gen, LV_OPA_20, 0);
    lv_obj_set_style_border_width(btn_gen, 1, 0);
    lv_obj_set_style_border_color(btn_gen, lv_color_hex(0x3DD9D0), 0);
    lv_obj_set_style_pad_all(btn_gen, 0, 0);
    lv_obj_clear_flag(btn_gen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_gen, on_generate_click, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_gen = lv_label_create(btn_gen);
    lv_label_set_text(lbl_gen, "↻");
    lv_obj_set_style_text_font(lbl_gen, &font_cjk_14, 0);
    lv_obj_set_style_text_color(lbl_gen, lv_color_hex(0x3DD9D0), 0);
    lv_obj_center(lbl_gen);

    // Right ">" button: 48x48
    lv_obj_t *btn_next = lv_obj_create(action_cont);
    lv_obj_set_size(btn_next, 48, 48);
    lv_obj_align(btn_next, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_radius(btn_next, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_next, lv_color_hex(0x13131A), 0);
    lv_obj_set_style_border_width(btn_next, 1, 0);
    lv_obj_set_style_border_color(btn_next, lv_color_hex(0x252530), 0);
    lv_obj_set_style_pad_all(btn_next, 0, 0);
    lv_obj_clear_flag(btn_next, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_next, on_next_click, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl_next = lv_label_create(btn_next);
    lv_label_set_text(lbl_next, ">");
    lv_obj_set_style_text_font(lbl_next, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_next, lv_color_hex(0xE8E8ED), 0);
    lv_obj_center(lbl_next);

    // --- Initial update ---
    update_preview();
    update_peeks();
    update_species_info();
    update_eye_buttons();
    update_color_buttons();
    TRACE_SELECT_EXIT();
}
