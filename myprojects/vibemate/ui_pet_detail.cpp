#include "ui_pet_detail.h"
#include "pet_sprites.h"
#include <stdio.h>

static lv_obj_t *s_label_title;
static lv_obj_t *s_label_subtitle;
static lv_obj_t *s_bar_stats[PET_STAT_COUNT];
static lv_obj_t *s_label_stat_vals[PET_STAT_COUNT];

void ui_pet_detail_create(lv_obj_t *parent_tile)
{
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // Title: pet name
    s_label_title = lv_label_create(parent_tile);
    static char title_buf[48];
    snprintf(title_buf, sizeof(title_buf), "%s", g_pet.name);
    lv_label_set_text(s_label_title, title_buf);
    lv_obj_set_style_text_color(s_label_title, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(s_label_title, &lv_font_montserrat_16, 0);
    lv_obj_align(s_label_title, LV_ALIGN_TOP_MID, 0, 40);

    // Subtitle: species + rarity + stars
    s_label_subtitle = lv_label_create(parent_tile);
    static char sub_buf[64];
    snprintf(sub_buf, sizeof(sub_buf), "%s  |  %s  %s",
             SPECIES_NAMES[g_pet.species],
             RARITY_NAMES[g_pet.rarity],
             RARITY_STARS[g_pet.rarity]);
    lv_label_set_text(s_label_subtitle, sub_buf);
    lv_obj_set_style_text_color(s_label_subtitle, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(s_label_subtitle, &lv_font_montserrat_14, 0);
    lv_obj_align(s_label_subtitle, LV_ALIGN_TOP_MID, 0, 72);

    // Stat bars
    int bar_y_start = 120;
    int bar_spacing = 38;
    for (int i = 0; i < PET_STAT_COUNT; i++) {
        int y = bar_y_start + i * bar_spacing;

        // Stat name label (left)
        lv_obj_t *label_name = lv_label_create(parent_tile);
        lv_label_set_text(label_name, STAT_LABELS[i]);
        lv_obj_set_style_text_color(label_name, lv_color_hex(0x94a3b8), 0);
        lv_obj_set_style_text_font(label_name, &lv_font_montserrat_12, 0);
        lv_obj_align(label_name, LV_ALIGN_TOP_LEFT, 30, y);

        // Value label (right of bar)
        s_label_stat_vals[i] = lv_label_create(parent_tile);
        static char val_buf[8];
        snprintf(val_buf, sizeof(val_buf), "%d", g_pet.stats[i]);
        lv_label_set_text(s_label_stat_vals[i], val_buf);
        lv_obj_set_style_text_color(s_label_stat_vals[i], lv_color_hex(0xf0f4f8), 0);
        lv_obj_set_style_text_font(s_label_stat_vals[i], &lv_font_montserrat_12, 0);
        lv_obj_align(s_label_stat_vals[i], LV_ALIGN_TOP_RIGHT, -30, y);

        // Bar
        s_bar_stats[i] = lv_bar_create(parent_tile);
        lv_obj_set_size(s_bar_stats[i], 220, 8);
        lv_obj_align(s_bar_stats[i], LV_ALIGN_TOP_MID, 0, y + 22);
        lv_bar_set_range(s_bar_stats[i], 0, 100);
        lv_bar_set_value(s_bar_stats[i], g_pet.stats[i], LV_ANIM_OFF);

        lv_obj_set_style_bg_color(s_bar_stats[i], lv_color_hex(0x1e293b), LV_PART_MAIN);
        lv_obj_set_style_bg_color(s_bar_stats[i], lv_color_hex(0x38bdf8), LV_PART_INDICATOR);
        lv_obj_set_style_radius(s_bar_stats[i], 4, LV_PART_MAIN);
        lv_obj_set_style_radius(s_bar_stats[i], 4, LV_PART_INDICATOR);
    }
}
