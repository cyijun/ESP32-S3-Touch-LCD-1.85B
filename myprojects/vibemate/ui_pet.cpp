#include "ui_pet.h"

static lv_obj_t *label_title;
static lv_obj_t *label_face;
static lv_obj_t *label_status;

static const char *s_messages[] = {
    "Coding hard...",
    "Take a break~",
    "Stay hydrated!"
};
static const int s_msg_count = sizeof(s_messages) / sizeof(s_messages[0]);
static int s_msg_index = 0;

void ui_pet_create(lv_obj_t *parent_tile) {
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // Title
    label_title = lv_label_create(parent_tile);
    lv_label_set_text(label_title, "VIBEMATE");
    lv_obj_set_style_text_color(label_title, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_16, 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 30);

    // Face
    label_face = lv_label_create(parent_tile);
    lv_label_set_text(label_face, "^_^");
    lv_obj_set_style_text_color(label_face, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_face, &lv_font_montserrat_36, 0);
    lv_obj_align(label_face, LV_ALIGN_CENTER, 0, 0);

    // Status
    label_status = lv_label_create(parent_tile);
    lv_label_set_text(label_status, s_messages[0]);
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -40);
}

void ui_pet_update(void) {
    s_msg_index++;
    if (s_msg_index >= s_msg_count) {
        s_msg_index = 0;
    }
    lv_label_set_text(label_status, s_messages[s_msg_index]);
}
