#include "ui_device.h"
#include "network_manager.h"

static lv_obj_t *label_bat_soc;
static lv_obj_t *label_bat_detail;
static lv_obj_t *label_wifi;
static lv_obj_t *label_ip;
static lv_obj_t *label_time;
static lv_obj_t *bar_bat;

void ui_device_create(lv_obj_t *parent_tile) {
    lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(parent_tile, 0, 0);
    lv_obj_set_style_border_width(parent_tile, 0, 0);

    // Title
    lv_obj_t *label_title = lv_label_create(parent_tile);
    lv_label_set_text(label_title, "DEVICE STATUS");
    lv_obj_set_style_text_color(label_title, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_16, 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 30);

    int y = 70;
    int gap = 45;

    // Battery SOC label
    label_bat_soc = lv_label_create(parent_tile);
    lv_label_set_text(label_bat_soc, "Battery");
    lv_obj_set_style_text_color(label_bat_soc, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_bat_soc, &lv_font_montserrat_14, 0);
    lv_obj_align(label_bat_soc, LV_ALIGN_TOP_LEFT, 30, y);

    // Battery progress bar
    bar_bat = lv_bar_create(parent_tile);
    lv_obj_set_size(bar_bat, 200, 12);
    lv_bar_set_range(bar_bat, 0, 100);
    lv_bar_set_value(bar_bat, 78, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_bat, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar_bat, lv_color_hex(0x4ade80), LV_PART_INDICATOR);
    lv_obj_align(bar_bat, LV_ALIGN_TOP_LEFT, 30, y + 20);

    // Battery detail text
    label_bat_detail = lv_label_create(parent_tile);
    lv_label_set_text(label_bat_detail, "SOC: 78% | 4123 mV | -120 mA | 28.5C");
    lv_obj_set_style_text_color(label_bat_detail, lv_color_hex(0x94a3b8), 0);
    lv_obj_set_style_text_font(label_bat_detail, &lv_font_montserrat_14, 0);
    lv_obj_align(label_bat_detail, LV_ALIGN_TOP_LEFT, 30, y + 38);

    y += gap + 20;

    // WiFi
    label_wifi = lv_label_create(parent_tile);
    lv_label_set_text(label_wifi, "WiFi: -- dBm");
    lv_obj_set_style_text_color(label_wifi, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_wifi, &lv_font_montserrat_14, 0);
    lv_obj_align(label_wifi, LV_ALIGN_TOP_LEFT, 30, y);

    y += gap;

    // IP Address
    label_ip = lv_label_create(parent_tile);
    lv_label_set_text(label_ip, "IP: --");
    lv_obj_set_style_text_color(label_ip, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_ip, &lv_font_montserrat_14, 0);
    lv_obj_align(label_ip, LV_ALIGN_TOP_LEFT, 30, y);

    y += gap;

    // RTC Time
    label_time = lv_label_create(parent_tile);
    lv_label_set_text(label_time, "Time: --:--:--");
    lv_obj_set_style_text_color(label_time, lv_color_hex(0xf0f4f8), 0);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_14, 0);
    lv_obj_align(label_time, LV_ALIGN_TOP_LEFT, 30, y);
}

void ui_device_update(void) {
    char buf[64];

    int rssi = network_get_rssi();
    lv_snprintf(buf, sizeof(buf), "WiFi: %d dBm", rssi);
    lv_label_set_text(label_wifi, buf);

    String ip = network_get_ip();
    lv_snprintf(buf, sizeof(buf), "IP: %s", ip.c_str());
    lv_label_set_text(label_ip, buf);

    // RTC placeholder for now
    lv_label_set_text(label_time, "Time: --:--:--");
}
