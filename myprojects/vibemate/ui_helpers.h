#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== Theme colors ==========
// Defined as macros so they can be used at compile time (lv_color_hex is a runtime function).
#define UI_COLOR_BG          lv_color_hex(0x0A0A0F)  // page background
#define UI_COLOR_BG_ALT      lv_color_hex(0x0a0e17)  // alternate page background (usage page)
#define UI_COLOR_CARD        lv_color_hex(0x13131A)  // card / container background
#define UI_COLOR_CARD_ACCENT lv_color_hex(0x3DD9D0)  // accent card background (at 20% opacity)
#define UI_COLOR_BORDER      lv_color_hex(0x252530)  // border / divider
#define UI_COLOR_TEXT        lv_color_hex(0xE8E8ED)  // primary text
#define UI_COLOR_TEXT_LIGHT  lv_color_hex(0xf0f4f8)  // light text (titles, device page)
#define UI_COLOR_TEXT_MUTED  lv_color_hex(0x6B6B78)  // secondary / muted text
#define UI_COLOR_TEXT_DIM    lv_color_hex(0x94a3b8)  // dim text (device page labels)
#define UI_COLOR_ACCENT      lv_color_hex(0x3DD9D0)  // accent / highlight
#define UI_COLOR_BAR_BG      lv_color_hex(0x13131A)  // bar background
#define UI_COLOR_BAR_BG_ALT  lv_color_hex(0x1e293b)  // alternate bar background (device page)
#define UI_COLOR_SUCCESS     lv_color_hex(0x4ade80)  // success / battery full
#define UI_COLOR_WARNING     lv_color_hex(0xffa726)  // warning
#define UI_COLOR_ERROR       lv_color_hex(0xef4444)  // error
#define UI_COLOR_INFO        lv_color_hex(0x4fc3f7)  // info / week arc

// ========== Container helpers ==========

/**
 * Create a transparent, non-scrollable container.
 * Sets: bg_opa=TRANSP, border_width=0, pad_all=0, clears SCROLLABLE, scrollbar_mode=OFF.
 */
lv_obj_t* ui_create_container(lv_obj_t* parent, lv_coord_t w, lv_coord_t h);

/**
 * Create a scrollable container with transparent background.
 * Sets: bg_opa=TRANSP, border_width=0, pad_all=0, scrollbar_mode=OFF.
 */
lv_obj_t* ui_create_scrollable_container(lv_obj_t* parent, lv_coord_t w, lv_coord_t h);

/**
 * Create a card-style container with background, border, and rounded corners.
 * Sets: bg_color=UI_COLOR_CARD, border_width=1, border_color=UI_COLOR_BORDER, radius=6.
 */
lv_obj_t* ui_create_card(lv_obj_t* parent, lv_coord_t w, lv_coord_t h);

// ========== Tile / page setup ==========

/**
 * Apply standard tile styling: bg_color=UI_COLOR_BG, pad_all=0, border_width=0.
 * Also clears LV_OBJ_FLAG_SCROLLABLE and sets scrollbar_mode=OFF.
 */
void ui_style_tile(lv_obj_t* tile);

/**
 * Apply alternate tile styling: bg_color=UI_COLOR_BG_ALT, pad_all=0, border_width=0.
 */
void ui_style_tile_alt(lv_obj_t* tile);

// ========== Button helpers ==========

/**
 * Create a circular button with icon label centered.
 * Returns the button object. The icon label is a child of the button.
 */
lv_obj_t* ui_create_circle_btn(lv_obj_t* parent, lv_coord_t size,
                                const char* icon, const lv_font_t* icon_font,
                                lv_color_t bg_color, lv_color_t border_color,
                                lv_color_t icon_color, lv_event_cb_t cb);

/**
 * Create a text button (rounded rect) with centered text label.
 */
lv_obj_t* ui_create_text_btn(lv_obj_t* parent, lv_coord_t w, lv_coord_t h,
                              const char* text, const lv_font_t* font,
                              lv_color_t bg_color, lv_color_t border_color,
                              lv_color_t text_color, lv_event_cb_t cb);

// ========== Label helpers ==========

/**
 * Create a label with specified text, font, and color.
 * No alignment is applied; caller must position the label.
 */
lv_obj_t* ui_create_label(lv_obj_t* parent, const char* text,
                           const lv_font_t* font, lv_color_t color);

/**
 * Create a label aligned to a specific position.
 */
lv_obj_t* ui_create_label_aligned(lv_obj_t* parent, const char* text,
                                   const lv_font_t* font, lv_color_t color,
                                   lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);

// ========== Bar helpers ==========

/**
 * Create a styled bar with custom background and indicator colors.
 */
lv_obj_t* ui_create_bar(lv_obj_t* parent, lv_coord_t w, lv_coord_t h,
                         int32_t min_val, int32_t max_val,
                         lv_color_t bg_color, lv_color_t indicator_color);

// ========== Arc helpers ==========

/**
 * Create a styled arc (no knob, not clickable) with custom colors.
 * bg_angles defaults to 0-360, rotation to 270.
 */
lv_obj_t* ui_create_arc(lv_obj_t* parent, lv_coord_t size,
                         lv_color_t bg_color, lv_color_t indicator_color,
                         lv_coord_t width);

#ifdef __cplusplus
}
#endif

#endif // UI_HELPERS_H
