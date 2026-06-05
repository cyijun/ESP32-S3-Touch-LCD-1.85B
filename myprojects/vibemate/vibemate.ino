#include "config.h"
#include "Display_ST77916.h"
#include "LVGL_Driver.h"
#include "I2C_Driver.h"
#include "network_manager.h"
#include "kimi_api.h"
#include "ui_usage.h"
#include "ui_device.h"
#include "ui_pet.h"
#include "ui_pet_detail.h"
#include "pet_sprites.h"
#include "rtc_bsp.h"
#include <lvgl.h>
#include <Wire.h>
#include <BQ27220.h>

static lv_obj_t *tileview;
static lv_obj_t *tile_pet_detail;
static lv_obj_t *tile_pet;
static lv_obj_t *tile_usage;
static lv_obj_t *tile_device;
static lv_timer_t *api_timer = NULL;
static lv_timer_t *device_timer = NULL;
static lv_timer_t *ui_timer = NULL;

SemaphoreHandle_t wire_mutex;
BQ27220 g_bq27220;

static void api_timer_cb(lv_timer_t *timer)
{
    if (network_is_connected()) {
        kimi_api_refresh_now();
    }
}

static void device_timer_cb(lv_timer_t *timer)
{
    ui_device_update();
    ui_pet_update();
}

static void ui_timer_cb(lv_timer_t *timer)
{
    if (g_ui_needs_update) {
        g_ui_needs_update = false;
        ui_usage_update(&g_kimi_data);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("VibeMate starting...");
    wire_mutex = xSemaphoreCreateMutex();

    Serial.println("[INIT] I2C...");
    I2C_Init();

    Serial.println("[INIT] Backlight OFF...");
    pinMode(LCD_Backlight_PIN, OUTPUT);
    digitalWrite(LCD_Backlight_PIN, LOW);
    Backlight_Init();
    Set_Backlight(0);

    Serial.println("[INIT] LCD...");
    LCD_Init();

    Serial.println("[INIT] LVGL...");
    Lvgl_Init();

    Serial.println("[INIT] BQ27220...");
    if (!g_bq27220.begin(Wire, 0x55, I2C_SDA_PIN, I2C_SCL_PIN, 400000)) {
        Serial.println("BQ27220 not found");
    } else {
        Serial.println("BQ27220 ready");
    }

    Serial.println("[INIT] RTC...");
    rtc_init();

    Serial.println("[INIT] Network...");
    network_init();

    if (network_is_connected()) {
        Serial.println("[INIT] NTP sync...");
        network_sync_ntp_to_rtc();
    }

    Serial.println("[INIT] Kimi API...");
    kimi_api_init();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0e17), 0);

    tileview = lv_tileview_create(scr);
    lv_obj_set_size(tileview, 360, 360);
    lv_obj_set_style_bg_color(tileview, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tileview, 0, 0);
    lv_obj_set_style_border_width(tileview, 0, 0);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

    tile_pet_detail = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
    tile_pet    = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
    tile_usage  = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);
    tile_device = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR);

    lv_obj_set_style_bg_color(tile_pet_detail, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_pet,    lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_usage,  lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_device, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tile_pet_detail, 0, 0);
    lv_obj_set_style_pad_all(tile_pet, 0, 0);
    lv_obj_set_style_pad_all(tile_usage, 0, 0);
    lv_obj_set_style_pad_all(tile_device, 0, 0);
    lv_obj_set_style_border_width(tile_pet_detail, 0, 0);
    lv_obj_set_style_border_width(tile_pet, 0, 0);
    lv_obj_set_style_border_width(tile_usage, 0, 0);
    lv_obj_set_style_border_width(tile_device, 0, 0);

    lv_obj_set_tile(tileview, tile_usage, LV_ANIM_OFF);

    pet_generate();
    ui_pet_detail_create(tile_pet_detail);
    ui_pet_create(tile_pet);
    ui_usage_create(tile_usage);
    ui_device_create(tile_device);

    Serial.println("[INIT] Backlight ON...");
    Set_Backlight(BACKLIGHT_BRIGHTNESS);

    api_timer = lv_timer_create(api_timer_cb, API_REFRESH_INTERVAL_MS, NULL);
    device_timer = lv_timer_create(device_timer_cb, 1000, NULL);
    ui_timer = lv_timer_create(ui_timer_cb, 500, NULL);

    if (network_is_connected()) {
        kimi_api_refresh_now();
    }

    Serial.println("VibeMate ready!");
}

void loop()
{
    Lvgl_Loop();
    network_check();
    vTaskDelay(pdMS_TO_TICKS(10));
}
