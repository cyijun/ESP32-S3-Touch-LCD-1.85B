#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <ESP_I2S.h>

#define AUDIO_SAMPLE_RATE       16000
#define AUDIO_BIT_WIDTH         I2S_DATA_BIT_WIDTH_16BIT
#define AUDIO_CHANNELS          I2S_SLOT_MODE_MONO
#define AUDIO_FRAME_MS          20
#define AUDIO_FRAME_SAMPLES     ((AUDIO_SAMPLE_RATE * AUDIO_FRAME_MS) / 1000)  // 320
#define AUDIO_FRAME_BYTES       (AUDIO_FRAME_SAMPLES * sizeof(int16_t))         // 640

#define I2S_PIN_MCK  GPIO_NUM_2
#define I2S_PIN_BCK  GPIO_NUM_48
#define I2S_PIN_WS   GPIO_NUM_38
#define I2S_PIN_DOUT GPIO_NUM_47
#define I2S_PIN_DIN  GPIO_NUM_39
#define AMP_EN_PIN   GPIO_NUM_9
#define ES7210_I2C_ADDR 0x40

bool audio_manager_init(void);
void audio_manager_deinit(void);
void audio_manager_amp_enable(bool enable);
bool audio_read_frame(int16_t *buffer);
bool audio_write_frame(const int16_t *buffer);
void audio_set_playback_volume(uint8_t percent);
void audio_set_capture_gain(uint8_t percent);

#endif
