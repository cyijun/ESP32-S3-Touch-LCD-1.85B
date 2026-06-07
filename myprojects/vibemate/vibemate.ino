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
#include "ui_pet_select.h"
#include "pet_sprites.h"
#include "pet_storage.h"
#include "rtc_bsp.h"
#include "debug_trace.h"
#include <lvgl.h>
#include <Wire.h>
#include <BQ27220.h>
#include <esp_heap_caps.h>

static lv_obj_t *tileview;
static lv_obj_t *tile_pet_select;
static lv_obj_t *tile_pet_detail;
static lv_obj_t *tile_pet;
static lv_obj_t *tile_usage;
static lv_obj_t *tile_device;
static lv_timer_t *api_timer = NULL;
static lv_timer_t *device_timer = NULL;
static lv_timer_t *ui_timer = NULL;
static lv_timer_t *decay_timer = NULL;

SemaphoreHandle_t wire_mutex;
BQ27220 g_bq27220;

static void api_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    TRACE_MAIN("api_timer_cb");
    if (network_is_connected()) {
        kimi_api_refresh_now();
    }
}

static void tileview_event_cb(lv_event_t *e)
{
    (void)e;
    lv_obj_t *tv = lv_event_get_target(e);
    lv_obj_t *tile = lv_tileview_get_tile_act(tv);

    /* Debounce: ignore repeated events for the same tile.  lv_tileview can
       fire VALUE_CHANGED multiple times during inertial scroll bounces. */
    static lv_obj_t *s_last_reported_tile = NULL;
    if (tile == s_last_reported_tile) return;
    s_last_reported_tile = tile;

    int idx = -1;
    if (tile == tile_pet_select) idx = 0;
    else if (tile == tile_pet_detail) idx = 1;
    else if (tile == tile_pet) idx = 2;
    else if (tile == tile_usage) idx = 3;
    else if (tile == tile_device) idx = 4;
    TRACE_MAIN("tileview changed to tile=%d", idx);

    if (tile == tile_pet) {
        ui_pet_resume_anim();
    } else {
        ui_pet_pause_anim();
    }
}

static void device_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    TRACE_MAIN_ENTER();
    ui_device_update();
    ui_pet_update();
    TRACE_MAIN_HEAP();
    TRACE_MAIN_EXIT();
}

static void ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (g_ui_needs_update && kimi_mutex &&
        xSemaphoreTake(kimi_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        g_ui_needs_update = false;
        kimi_usage_t snapshot = g_kimi_data;
        xSemaphoreGive(kimi_mutex);
        TRACE_MAIN("ui_timer_cb updating usage");
        ui_usage_update(&snapshot);
    }
}

static void decay_timer_cb(lv_timer_t *timer) {
    (void)timer;
    bool changed = false;
    if (g_pet.hunger > 0) { g_pet.hunger--; changed = true; }
    if (g_pet.joy > 0) { g_pet.joy--; changed = true; }
    if (changed) {
        TRACE_MAIN("decay hunger=%d joy=%d", g_pet.hunger, g_pet.joy);
        pet_save();
        ui_pet_update();
    }
}

void setup()
{
    Serial.begin(115200);
    delay(200);  // Wait for USB Serial/JTAG to be ready

    // Print last reset reason to help diagnose crashes
    const char *rst_reason = "UNKNOWN";
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON:  rst_reason = "POWERON"; break;
        case ESP_RST_SW:       rst_reason = "SW"; break;
        case ESP_RST_PANIC:    rst_reason = "PANIC"; break;
        case ESP_RST_INT_WDT:  rst_reason = "INT_WDT"; break;
        case ESP_RST_TASK_WDT: rst_reason = "TASK_WDT"; break;
        case ESP_RST_WDT:      rst_reason = "WDT"; break;
        case ESP_RST_DEEPSLEEP: rst_reason = "DEEPSLEEP"; break;
        case ESP_RST_BROWNOUT: rst_reason = "BROWNOUT"; break;
        default: break;
    }
    TRACE_MAIN("VibeMate starting... reset_reason=%s", rst_reason);
    wire_mutex = xSemaphoreCreateMutex();

    TRACE_MAIN("INIT I2C...");
    I2C_Init();

    TRACE_MAIN("INIT Backlight OFF...");
    pinMode(LCD_Backlight_PIN, OUTPUT);
    digitalWrite(LCD_Backlight_PIN, LOW);
    Backlight_Init();
    Set_Backlight(0);

    TRACE_MAIN("INIT LCD...");
    LCD_Init();

    TRACE_MAIN("INIT LVGL...");
    Lvgl_Init();

    TRACE_MAIN("INIT BQ27220...");
    if (!g_bq27220.begin(Wire, 0x55, I2C_SDA_PIN, I2C_SCL_PIN, 400000)) {
        TRACE_MAIN("BQ27220 not found");
    } else {
        TRACE_MAIN("BQ27220 ready");
    }

    TRACE_MAIN("INIT RTC...");
    rtc_init();

    TRACE_MAIN("INIT Network...");
    network_init();

    if (network_is_connected()) {
        TRACE_MAIN("INIT NTP sync...");
        network_sync_ntp_to_rtc();
    }

    TRACE_MAIN("INIT Kimi API...");
    kimi_api_init();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0e17), 0);

    tileview = lv_tileview_create(scr);
    lv_obj_set_size(tileview, 360, 360);
    lv_obj_set_style_bg_color(tileview, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tileview, 0, 0);
    lv_obj_set_style_border_width(tileview, 0, 0);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

    tile_pet_select = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
    tile_pet_detail = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
    tile_pet    = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);
    tile_usage  = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR);
    tile_device = lv_tileview_add_tile(tileview, 4, 0, LV_DIR_HOR);

    lv_obj_set_style_bg_color(tile_pet_select, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_pet_detail, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_pet,    lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_usage,  lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_device, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tile_pet_select, 0, 0);
    lv_obj_set_style_pad_all(tile_pet_detail, 0, 0);
    lv_obj_set_style_pad_all(tile_pet, 0, 0);
    lv_obj_set_style_pad_all(tile_usage, 0, 0);
    lv_obj_set_style_pad_all(tile_device, 0, 0);
    lv_obj_set_style_border_width(tile_pet_select, 0, 0);
    lv_obj_set_style_border_width(tile_pet_detail, 0, 0);
    lv_obj_set_style_border_width(tile_pet, 0, 0);
    lv_obj_set_style_border_width(tile_usage, 0, 0);
    lv_obj_set_style_border_width(tile_device, 0, 0);

    lv_obj_clear_flag(tile_pet_select, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_pet_detail, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_pet,    LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_usage,  LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tile_device, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_tile(tileview, tile_pet, LV_ANIM_OFF);
    lv_obj_add_event_cb(tileview, tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    if (!pet_load()) {
        TRACE_MAIN("No saved pet, generating...");
        pet_generate();
        pet_save();
    } else {
        TRACE_MAIN("Pet loaded from NVS");
    }
    TRACE_MAIN("Creating ui_pet_select...");
    ui_pet_select_create(tile_pet_select);
    TRACE_MAIN("Creating ui_pet_detail...");
    ui_pet_detail_create(tile_pet_detail);
    TRACE_MAIN("Creating ui_pet...");
    ui_pet_create(tile_pet);
    TRACE_MAIN("Creating ui_usage...");
    ui_usage_create(tile_usage);
    TRACE_MAIN("Creating ui_device...");
    ui_device_create(tile_device);

    TRACE_MAIN("INIT Backlight ON...");
    /* Force a full LVGL refresh before turning on backlight so the first
       frame the user sees is the complete pet page, not white/garbage. */
    lv_timer_handler();
    Set_Backlight(BACKLIGHT_BRIGHTNESS);

    TRACE_MAIN("Creating timers...");
    api_timer = lv_timer_create(api_timer_cb, API_REFRESH_INTERVAL_MS, NULL);
    device_timer = lv_timer_create(device_timer_cb, 1000, NULL);
    ui_timer = lv_timer_create(ui_timer_cb, 500, NULL);
    decay_timer = lv_timer_create(decay_timer_cb, 60000, NULL);

    if (network_is_connected()) {
        kimi_api_refresh_now();
    }

    TRACE_MAIN("VibeMate ready!");
    TRACE_MAIN_HEAP();
}

void loop()
{
    static uint32_t last_heap_print = 0;
    Lvgl_Loop();
    network_check();
    uint32_t now = millis();
    if (now - last_heap_print >= 5000) {
        last_heap_print = now;
        TRACE_MAIN_HEAP();
        size_t largest_int = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        size_t largest_psram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
        Serial.printf("[MAIN] LARGEST free int=%u psram=%u\n", largest_int, largest_psram);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}
