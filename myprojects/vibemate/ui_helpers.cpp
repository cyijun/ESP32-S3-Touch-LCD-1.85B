#include "ui_helpers.h"

// ========== Container helpers ==========

lv_obj_t* ui_create_container(lv_obj_t* parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, w, h);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    return cont;
}

lv_obj_t* ui_create_scrollable_container(lv_obj_t* parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, w, h);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    return cont;
}

lv_obj_t* ui_create_card(lv_obj_t* parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, UI_COLOR_CARD, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, UI_COLOR_BORDER, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_pad_all(card, 4, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    return card;
}

// ========== Tile / page setup ==========

void ui_style_tile(lv_obj_t* tile)
{
    lv_obj_set_style_bg_color(tile, UI_COLOR_BG, 0);
    lv_obj_set_style_pad_all(tile, 0, 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);
}

void ui_style_tile_alt(lv_obj_t* tile)
{
    lv_obj_set_style_bg_color(tile, UI_COLOR_BG_ALT, 0);
    lv_obj_set_style_pad_all(tile, 0, 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);
}

// ========== Button helpers ==========

lv_obj_t* ui_create_circle_btn(lv_obj_t* parent, lv_coord_t size,
                                const char* icon, const lv_font_t* icon_font,
                                lv_color_t bg_color, lv_color_t border_color,
                                lv_color_t icon_color, lv_event_cb_t cb)
{
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, size, size);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, bg_color, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, border_color, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, icon);
    lv_obj_set_style_text_color(lbl, icon_color, 0);
    lv_obj_set_style_text_font(lbl, icon_font, 0);
    lv_obj_center(lbl);

    return btn;
}

lv_obj_t* ui_create_text_btn(lv_obj_t* parent, lv_coord_t w, lv_coord_t h,
                              const char* text, const lv_font_t* font,
                              lv_color_t bg_color, lv_color_t border_color,
                              lv_color_t text_color, lv_event_cb_t cb)
{
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_bg_color(btn, bg_color, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, border_color, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, text_color, 0);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_center(lbl);

    return btn;
}

// ========== Label helpers ==========

lv_obj_t* ui_create_label(lv_obj_t* parent, const char* text,
                           const lv_font_t* font, lv_color_t color)
{
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    return lbl;
}

lv_obj_t* ui_create_label_aligned(lv_obj_t* parent, const char* text,
                                   const lv_font_t* font, lv_color_t color,
                                   lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    lv_obj_t* lbl = ui_create_label(parent, text, font, color);
    lv_obj_align(lbl, align, x_ofs, y_ofs);
    return lbl;
}

// ========== Bar helpers ==========

lv_obj_t* ui_create_bar(lv_obj_t* parent, lv_coord_t w, lv_coord_t h,
                         int32_t min_val, int32_t max_val,
                         lv_color_t bg_color, lv_color_t indicator_color)
{
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_size(bar, w, h);
    lv_bar_set_range(bar, min_val, max_val);
    lv_obj_set_style_bg_color(bar, bg_color, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, indicator_color, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 2, LV_PART_INDICATOR);
    return bar;
}

// ========== Arc helpers ==========

lv_obj_t* ui_create_arc(lv_obj_t* parent, lv_coord_t size,
                         lv_color_t bg_color, lv_color_t indicator_color,
                         lv_coord_t width)
{
    lv_obj_t* arc = lv_arc_create(parent);
    lv_obj_set_size(arc, size, size);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, 100);
    lv_obj_set_style_arc_width(arc, width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, width, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, bg_color, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, indicator_color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    return arc;
}
