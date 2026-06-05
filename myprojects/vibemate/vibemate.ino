#include "config.h"
#include "Display_ST77916.h"
#include "LVGL_Driver.h"
#include "I2C_Driver.h"
#include "network_manager.h"
#include "kimi_api.h"
#include "ui_usage.h"
#include "ui_device.h"
#include "ui_pet.h"
#include "ui_voice.h"
#include "audio_manager.h"
#include "voice_network.h"
#include "rtc_bsp.h"
#include <lvgl.h>
#include <Wire.h>
#include <BQ27220.h>

static lv_obj_t *tileview;
static lv_obj_t *tile_usage;
static lv_obj_t *tile_device;
static lv_obj_t *tile_pet;
static lv_obj_t *tile_voice;
static lv_timer_t *api_timer = NULL;
static lv_timer_t *device_timer = NULL;
static lv_timer_t *ui_timer = NULL;

SemaphoreHandle_t wire_mutex;
BQ27220 g_bq27220;

static void voice_tx_task(void *pvParameters);
static void voice_rx_task(void *pvParameters);

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
    ui_voice_update();
    voice_network_update();
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

    Serial.println("[INIT] Audio manager...");
    audio_manager_init();

    Serial.println("[INIT] Voice network...");
    voice_network_init();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0e17), 0);

    tileview = lv_tileview_create(scr);
    lv_obj_set_size(tileview, 360, 360);
    lv_obj_set_style_bg_color(tileview, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tileview, 0, 0);
    lv_obj_set_style_border_width(tileview, 0, 0);

    tile_usage  = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
    tile_device = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
    tile_pet    = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);
    tile_voice  = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR);

    lv_obj_set_style_bg_color(tile_usage,  lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_device, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_pet,    lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_color(tile_voice,  lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_pad_all(tile_usage, 0, 0);
    lv_obj_set_style_pad_all(tile_device, 0, 0);
    lv_obj_set_style_pad_all(tile_pet, 0, 0);
    lv_obj_set_style_pad_all(tile_voice, 0, 0);
    lv_obj_set_style_border_width(tile_usage, 0, 0);
    lv_obj_set_style_border_width(tile_device, 0, 0);
    lv_obj_set_style_border_width(tile_pet, 0, 0);
    lv_obj_set_style_border_width(tile_voice, 0, 0);

    lv_obj_set_tile(tileview, tile_usage, LV_ANIM_OFF);

    ui_usage_create(tile_usage);
    ui_device_create(tile_device);
    ui_pet_create(tile_pet);
    ui_voice_create(tile_voice);

    // 监听页面切换，进入/离开 Voice 页时控制功放
    lv_obj_add_event_cb(tileview, [](lv_event_t *e) {
        lv_obj_t *tv = lv_event_get_target(e);
        lv_obj_t *current = lv_tileview_get_tile_act(tv);
        if (current == tile_voice) {
            audio_manager_amp_enable(true);
            voice_start_discovery();
        } else {
            audio_manager_amp_enable(false);
            voice_disconnect();
        }
    }, LV_EVENT_VALUE_CHANGED, NULL);

    Serial.println("[INIT] Backlight ON...");
    Set_Backlight(BACKLIGHT_BRIGHTNESS);

    api_timer = lv_timer_create(api_timer_cb, API_REFRESH_INTERVAL_MS, NULL);
    device_timer = lv_timer_create(device_timer_cb, 1000, NULL);
    ui_timer = lv_timer_create(ui_timer_cb, 500, NULL);

    // 启动音频收发任务（Core 1，优先级 5）
    xTaskCreatePinnedToCore(voice_tx_task, "voice_tx", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(voice_rx_task, "voice_rx", 4096, NULL, 5, NULL, 1);

    if (network_is_connected()) {
        kimi_api_refresh_now();
    }

    Serial.println("VibeMate ready!");
}

// 音频发送任务：从 ES7210 读取 PCM，通过 TCP 发送
static void voice_tx_task(void *pvParameters)
{
    static int16_t pcm_buffer[AUDIO_FRAME_SAMPLES];
    while (true) {
        if (audio_read_frame(pcm_buffer)) {
            voice_state_t state = voice_get_state();
            voice_mode_t mode = voice_get_mode();
            bool should_send = false;
            if (mode == VOICE_MODE_DUPLEX) {
                should_send = (state == VOICE_CONNECTED);
            } else {
                should_send = (state == VOICE_TRANSMITTING);
            }
            if (should_send) {
                voice_send_audio_frame((const int8_t *)pcm_buffer, AUDIO_FRAME_BYTES);
            }
        }
    }
}

// 音频接收任务：从 TCP 接收 PCM，写入 ES8311 播放
static void voice_rx_task(void *pvParameters)
{
    static uint8_t pcm_buffer[AUDIO_FRAME_BYTES];
    while (true) {
        size_t recv_len = voice_recv_audio_frame(pcm_buffer, sizeof(pcm_buffer));
        if (recv_len == AUDIO_FRAME_BYTES) {
            audio_write_frame((const int16_t *)pcm_buffer);
        } else {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void loop()
{
    Lvgl_Loop();
    network_check();
    vTaskDelay(pdMS_TO_TICKS(10));
}
