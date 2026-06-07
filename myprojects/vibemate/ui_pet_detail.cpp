#include "ui_pet_detail.h"
#include "pet_sprites.h"
#include <stdio.h>
#include <string.h>

// ========== Rarity color mapping ==========
static const uint32_t RARITY_COLORS[RARITY_COUNT] = {
    0x8A8A95, // Common
    0x5EE7DF, // Uncommon
    0x6B8CFF, // Rare
    0xC85EFF, // Epic
    0xFFD166, // Legendary
};

// ========== Static references for update ==========
static lv_obj_t *s_label_face;
static lv_obj_t *s_label_name;
static lv_obj_t *s_label_rarity_badge;
static lv_obj_t *s_arc_stats[PET_STAT_COUNT];
static lv_obj_t *s_label_arc_vals[PET_STAT_COUNT];
static lv_obj_t *s_label_stat_names[PET_STAT_COUNT];
static lv_obj_t *s_label_stat_vals[PET_STAT_COUNT];
static lv_obj_t *s_bar_stats[PET_STAT_COUNT];
static lv_obj_t *s_label_meta_vals[4];

// ========== Helpers ==========
static uint32_t rarity_color(PetRarity r)
{
    if (r >= RARITY_COUNT) return 0x8A8A95;
    return RARITY_COLORS[r];
}

static const char* face_for_pet(void)
{
    TRACE_DETAIL_ENTER();
    const char *face;
    // Simple face expression based on eye type
    switch (g_pet.eye) {
        case EYE_DOT:    face = "=·ω·="; break;
        case EYE_STAR:   face = "=*ω*="; break;
        case EYE_X:      face = "=×ω×="; break;
        case EYE_CIRCLE: face = "=◉ω◉="; break;
        case EYE_AT:     face = "=@ω@="; break;
        case EYE_DEG:    face = "=°ω°="; break;
        default:         face = "=·ω·="; break;
    }
    TRACE_DETAIL("face_for_pet() -> %s", face);
    return face;
}

// ========== Create ==========
void ui_pet_detail_create(lv_obj_t *parent_tile)
{
    TRACE_DETAIL_ENTER();
    TRACE_DETAIL_HEAP();
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);
    lv_obj_clear_flag(parent_tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(parent_tile, LV_SCROLLBAR_MODE_OFF);

    // --- Header area ---
    // 42px circular border container
    lv_obj_t *face_cont = lv_obj_create(parent_tile);
    lv_obj_set_size(face_cont, 42, 42);
    lv_obj_set_style_radius(face_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face_cont, lv_color_hex(0x13131A), 0);
    lv_obj_set_style_border_width(face_cont, 2, 0);
    lv_obj_set_style_border_color(face_cont, lv_color_hex(0x252530), 0);
    lv_obj_set_style_pad_all(face_cont, 0, 0);
    lv_obj_clear_flag(face_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(face_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(face_cont, LV_ALIGN_TOP_MID, 0, 14);

    s_label_face = lv_label_create(face_cont);
    lv_label_set_text(s_label_face, face_for_pet());
    lv_obj_set_style_text_font(s_label_face, &font_mono_16, 0);
    lv_obj_set_style_text_color(s_label_face, lv_color_hex(COLOR_HEX[g_pet.color]), 0);
    lv_obj_center(s_label_face);

    // Pet name
    s_label_name = lv_label_create(parent_tile);
    lv_label_set_text(s_label_name, g_pet.name);
    lv_obj_set_style_text_font(s_label_name, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_label_name, lv_color_hex(0xE8E8ED), 0);
    lv_obj_align(s_label_name, LV_ALIGN_TOP_MID, 0, 62);

    // Rarity badge capsule (hidden)
    s_label_rarity_badge = lv_label_create(parent_tile);
    lv_label_set_text(s_label_rarity_badge, RARITY_NAMES[g_pet.rarity]);
    lv_obj_set_style_text_font(s_label_rarity_badge, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(s_label_rarity_badge, lv_color_hex(rarity_color(g_pet.rarity)), 0);
    lv_obj_set_style_bg_color(s_label_rarity_badge, lv_color_hex(0x3DD9D0), 0);
    lv_obj_set_style_bg_opa(s_label_rarity_badge, LV_OPA_20, 0);
    lv_obj_set_style_radius(s_label_rarity_badge, 8, 0);
    lv_obj_set_style_pad_hor(s_label_rarity_badge, 8, 0);
    lv_obj_set_style_pad_ver(s_label_rarity_badge, 2, 0);
    lv_obj_align(s_label_rarity_badge, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_add_flag(s_label_rarity_badge, LV_OBJ_FLAG_HIDDEN);

    // --- Stats list (5 items) ---
    int stats_y_start = 96;
    int stats_row_h = 30;
    int stats_w = 260;
    int stats_x = (360 - stats_w) / 2; // center horizontally

    for (int i = 0; i < PET_STAT_COUNT; i++) {
        int y = stats_y_start + i * stats_row_h;

        // Row container (transparent)
        lv_obj_t *row = lv_obj_create(parent_tile);
        lv_obj_set_size(row, stats_w, stats_row_h);
        lv_obj_set_pos(row, stats_x, y);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);

        // Left: 22px arc
        lv_obj_t *arc = lv_arc_create(row);
        lv_obj_set_size(arc, 22, 22);
        lv_arc_set_rotation(arc, 270);
        lv_arc_set_bg_angles(arc, 0, 360);
        lv_arc_set_range(arc, 0, 100);
        lv_arc_set_value(arc, g_pet.stats[i] * 100 / 255);
        lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0x252530), LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, lv_color_hex(COLOR_HEX[g_pet.color]), LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(arc, LV_ALIGN_LEFT_MID, 0, 0);
        s_arc_stats[i] = arc;

        // Arc center value label
        lv_obj_t *arc_val = lv_label_create(arc);
        lv_label_set_text_fmt(arc_val, "%d", g_pet.stats[i] * 100 / 255);
        lv_obj_set_style_text_font(arc_val, &lv_font_montserrat_8, 0);
        lv_obj_set_style_text_color(arc_val, lv_color_hex(0xE8E8ED), 0);
        lv_obj_center(arc_val);
        s_label_arc_vals[i] = arc_val;

        // Right: info area container (~220px wide, positioned to the right of arc)
        lv_obj_t *info = lv_obj_create(row);
        lv_obj_set_size(info, 220, stats_row_h);
        lv_obj_set_style_bg_opa(info, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info, 0, 0);
        lv_obj_set_style_pad_all(info, 0, 0);
        lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(info, LV_SCROLLBAR_MODE_OFF);
        lv_obj_align(info, LV_ALIGN_LEFT_MID, 30, 0);

        // Top row: stat name (left) + value (right)
        lv_obj_t *label_name = lv_label_create(info);
        lv_label_set_text(label_name, STAT_LABELS[i]);
        lv_obj_set_style_text_font(label_name, &lv_font_montserrat_8, 0);
        lv_obj_set_style_text_color(label_name, lv_color_hex(0x6B6B78), 0);
        lv_obj_align(label_name, LV_ALIGN_TOP_LEFT, 0, 2);
        s_label_stat_names[i] = label_name;

        lv_obj_t *label_val = lv_label_create(info);
        lv_label_set_text_fmt(label_val, "%d", g_pet.stats[i] * 100 / 255);
        lv_obj_set_style_text_font(label_val, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(label_val, lv_color_hex(0xE8E8ED), 0);
        lv_obj_align(label_val, LV_ALIGN_TOP_RIGHT, 0, 0);
        s_label_stat_vals[i] = label_val;

        // Bottom: bar
        lv_obj_t *bar = lv_bar_create(info);
        lv_obj_set_size(bar, 220, 2);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, g_pet.stats[i] * 100 / 255, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x252530), LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_color_hex(COLOR_HEX[g_pet.color]), LV_PART_INDICATOR);
        lv_obj_set_style_radius(bar, 1, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 1, LV_PART_INDICATOR);
        lv_obj_align(bar, LV_ALIGN_BOTTOM_LEFT, 0, -2);
        s_bar_stats[i] = bar;
    }

    // --- Meta grid (2x2 cards) ---
    const char *META_LABELS[4] = {"SPECIES", "EYE", "HAT", "SHINY"};
    int card_w = 108;
    int card_h = 36;
    int grid_y = 258;
    int gap_x = 8;
    int gap_y = 6;
    int grid_w = card_w * 2 + gap_x;
    int grid_x = (360 - grid_w) / 2;

    for (int i = 0; i < 4; i++) {
        int col = i % 2;
        int row = i / 2;
        int x = grid_x + col * (card_w + gap_x);
        int y = grid_y + row * (card_h + gap_y);

        lv_obj_t *card = lv_obj_create(parent_tile);
        lv_obj_set_size(card, card_w, card_h);
        lv_obj_set_pos(card, x, y);
        lv_obj_set_style_bg_color(card, lv_color_hex(0x13131A), 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0x252530), 0);
        lv_obj_set_style_radius(card, 6, 0);
        lv_obj_set_style_pad_all(card, 4, 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

        // Label (top)
        lv_obj_t *label_top = lv_label_create(card);
        lv_label_set_text(label_top, META_LABELS[i]);
        lv_obj_set_style_text_font(label_top, &lv_font_montserrat_8, 0);
        lv_obj_set_style_text_color(label_top, lv_color_hex(0x6B6B78), 0);
        lv_obj_align(label_top, LV_ALIGN_TOP_LEFT, 0, 0);

        // Value (bottom)
        lv_obj_t *label_val = lv_label_create(card);
        lv_obj_set_style_text_font(label_val, &font_mono_16, 0);
        lv_obj_set_style_text_color(label_val, lv_color_hex(0xE8E8ED), 0);
        lv_obj_align(label_val, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        s_label_meta_vals[i] = label_val;
    }

    // Set meta values
    ui_pet_detail_update();
    TRACE_DETAIL_EXIT();
}

// ========== Update ==========
void ui_pet_detail_update(void)
{
    TRACE_DETAIL_ENTER();
    TRACE_DETAIL_HEAP();
    // Header
    lv_label_set_text(s_label_face, face_for_pet());
    lv_obj_set_style_text_color(s_label_face, lv_color_hex(COLOR_HEX[g_pet.color]), 0);
    lv_label_set_text(s_label_name, g_pet.name);
    lv_label_set_text(s_label_rarity_badge, RARITY_NAMES[g_pet.rarity]);
    lv_obj_set_style_text_color(s_label_rarity_badge, lv_color_hex(rarity_color(g_pet.rarity)), 0);

    // Stats
    uint32_t pet_color = COLOR_HEX[g_pet.color];
    for (int i = 0; i < PET_STAT_COUNT; i++) {
        uint8_t val = g_pet.stats[i] * 100 / 255;
        lv_arc_set_value(s_arc_stats[i], val);
        lv_obj_set_style_arc_color(s_arc_stats[i], lv_color_hex(pet_color), LV_PART_INDICATOR);
        lv_label_set_text_fmt(s_label_arc_vals[i], "%d", val);
        lv_label_set_text_fmt(s_label_stat_vals[i], "%d", val);
        lv_bar_set_value(s_bar_stats[i], val, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(s_bar_stats[i], lv_color_hex(pet_color), LV_PART_INDICATOR);
    }

    // Meta grid values
    lv_label_set_text(s_label_meta_vals[0], SPECIES_NAMES[g_pet.species]);
    lv_label_set_text(s_label_meta_vals[1], EYE_STRINGS[g_pet.eye]);
    lv_label_set_text(s_label_meta_vals[2], (g_pet.hat == HAT_NONE) ? "None" : "Yes");
    lv_label_set_text(s_label_meta_vals[3], g_pet.shiny ? "Yes" : "No");
    TRACE_DETAIL_EXIT();
}
