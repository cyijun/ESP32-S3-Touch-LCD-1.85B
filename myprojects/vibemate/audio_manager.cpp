#include "audio_manager.h"
#include "es8311.h"
#include "es7210.h"
#include "esp_check.h"

static I2SClass i2s;
static es8311_handle_t es8311_handle = NULL;
static es7210_dev_handle_t es7210_handle = NULL;
static bool audio_initialized = false;

static bool es8311_codec_init(void)
{
    es8311_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
    if (!es8311_handle) {
        Serial.println("ES8311 create failed");
        return false;
    }

    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = AUDIO_SAMPLE_RATE * 256,
        .sample_frequency = AUDIO_SAMPLE_RATE
    };

    if (es8311_init(es8311_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) != ESP_OK) {
        Serial.println("ES8311 init failed");
        return false;
    }

    if (es8311_voice_volume_set(es8311_handle, 60, NULL) != ESP_OK) {
        Serial.println("ES8311 set volume failed");
        return false;
    }

    if (es8311_microphone_config(es8311_handle, false) != ESP_OK) {
        Serial.println("ES8311 microphone config failed");
        return false;
    }

    return true;
}

static bool es7210_codec_init(void)
{
    es7210_i2c_config_t es7210_i2c_conf = {
        .i2c_addr = ES7210_I2C_ADDR
    };

    if (es7210_new_codec(&es7210_i2c_conf, &es7210_handle) != ESP_OK) {
        Serial.println("ES7210 create failed");
        return false;
    }

    es7210_codec_config_t codec_conf = {};
    codec_conf.i2s_format = ES7210_I2S_FMT_I2S;
    codec_conf.mclk_ratio = 256;
    codec_conf.sample_rate_hz = AUDIO_SAMPLE_RATE;
    codec_conf.bit_width = ES7210_I2S_BITS_16B;
    codec_conf.mic_bias = ES7210_MIC_BIAS_2V87;
    codec_conf.mic_gain = ES7210_MIC_GAIN_30DB;
    codec_conf.flags.tdm_enable = false;

    if (es7210_config_codec(es7210_handle, &codec_conf) != ESP_OK) {
        Serial.println("ES7210 config codec failed");
        return false;
    }

    if (es7210_config_volume(es7210_handle, 40) != ESP_OK) {
        Serial.println("ES7210 config volume failed");
        return false;
    }

    return true;
}

bool audio_manager_init(void)
{
    if (audio_initialized) {
        return true;
    }

    pinMode(AMP_EN_PIN, OUTPUT);
    digitalWrite(AMP_EN_PIN, LOW);

    if (!es8311_codec_init()) {
        return false;
    }

    if (!es7210_codec_init()) {
        return false;
    }

    i2s.setPins(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN, I2S_PIN_MCK);
    i2s.setTimeout(1000);

    if (!i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, AUDIO_BIT_WIDTH, AUDIO_CHANNELS, I2S_STD_SLOT_LEFT)) {
        Serial.println("Failed to initialize I2S bus");
        return false;
    }

    audio_manager_amp_enable(true);

    audio_initialized = true;
    return true;
}

void audio_manager_deinit(void)
{
    audio_manager_amp_enable(false);

    i2s.end();

    audio_initialized = false;
}

void audio_manager_amp_enable(bool enable)
{
    digitalWrite(AMP_EN_PIN, enable ? HIGH : LOW);
}

bool audio_read_frame(int16_t *buffer)
{
    if (!audio_initialized || buffer == NULL) {
        return false;
    }

    size_t bytes_read = i2s.readBytes((char *)buffer, AUDIO_FRAME_BYTES);
    return bytes_read == AUDIO_FRAME_BYTES;
}

bool audio_write_frame(const int16_t *buffer)
{
    if (!audio_initialized || buffer == NULL) {
        return false;
    }

    size_t bytes_written = i2s.write((uint8_t *)buffer, AUDIO_FRAME_BYTES);
    return bytes_written == AUDIO_FRAME_BYTES;
}

void audio_set_playback_volume(uint8_t percent)
{
    if (es8311_handle == NULL) {
        return;
    }

    if (percent > 100) {
        percent = 100;
    }

    es8311_voice_volume_set(es8311_handle, percent, NULL);
}

void audio_set_capture_gain(uint8_t percent)
{
    if (es7210_handle == NULL) {
        return;
    }

    if (percent > 100) {
        percent = 100;
    }

    es7210_config_volume(es7210_handle, percent);
}
