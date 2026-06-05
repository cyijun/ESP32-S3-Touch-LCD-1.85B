#ifndef VOICE_NETWORK_H
#define VOICE_NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define VOICE_DISCOVERY_PORT    3721
#define VOICE_TCP_PORT          3722
#define DISCOVERY_INTERVAL_MS   2000
#define HEARTBEAT_INTERVAL_MS   3000
#define HEARTBEAT_TIMEOUT_MS    10000

#define FRAME_AUDIO_UPLINK      0x01
#define FRAME_AUDIO_DOWNLINK    0x02
#define FRAME_CONTROL           0x03
#define FRAME_MAGIC_0           0x56
#define FRAME_MAGIC_1           0x4D

enum voice_state_t {
    VOICE_IDLE = 0,
    VOICE_DISCOVERING,
    VOICE_CONNECTING,
    VOICE_CONNECTED,
    VOICE_TRANSMITTING,
};

enum voice_mode_t {
    VOICE_MODE_PTT = 0,
    VOICE_MODE_DUPLEX,
};

void voice_network_init(void);
void voice_network_update(void);

voice_state_t voice_get_state(void);
voice_mode_t voice_get_mode(void);
const char* voice_get_host_ip(void);
const char* voice_get_host_name(void);

void voice_set_mode(voice_mode_t mode);
void voice_start_discovery(void);
void voice_connect_to_host(const char *ip, uint16_t port);
void voice_disconnect(void);
void voice_ptt_set(bool pressed);

bool voice_send_audio_frame(const int8_t *pcm_data, size_t len);
size_t voice_recv_audio_frame(uint8_t *pcm_buffer, size_t buf_len);
bool voice_send_control(const char *json_str);

#endif
