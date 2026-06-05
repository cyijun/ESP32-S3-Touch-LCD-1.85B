#include "voice_network.h"
#include "network_manager.h"
#include <ArduinoJson.h>
#include <WiFiClient.h>

static WiFiUDP udp;
static WiFiClient tcp_client;

static voice_state_t voice_state = VOICE_IDLE;
static voice_mode_t voice_mode = VOICE_MODE_PTT;

static char host_ip[16] = {0};
static char host_name[32] = {0};
static uint16_t host_port = VOICE_TCP_PORT;

static unsigned long last_discovery_ms = 0;
static unsigned long last_heartbeat_ms = 0;
static unsigned long last_pong_ms = 0;

static bool host_found = false;

static void send_discovery_broadcast(void)
{
    if (!network_is_connected()) return;

    StaticJsonDocument<128> doc;
    doc["type"] = "discover";
    doc["device"] = "vibemate";
    doc["version"] = 1;

    char buf[128];
    size_t len = serializeJson(doc, buf, sizeof(buf));

    udp.beginPacket(IPAddress(255, 255, 255, 255), VOICE_DISCOVERY_PORT);
    udp.write((const uint8_t *)buf, len);
    udp.endPacket();
}

static void process_udp_inbound(void)
{
    int packetSize = udp.parsePacket();
    if (packetSize <= 0) return;

    char buf[256];
    int len = udp.read(buf, sizeof(buf) - 1);
    if (len <= 0) return;
    buf[len] = '\0';

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, buf);
    if (err) return;

    const char *type = doc["type"];
    if (!type || strcmp(type, "announce") != 0) return;

    const char *ip = doc["ip"];
    const char *name = doc["name"];
    uint16_t port = doc["port"] | VOICE_TCP_PORT;

    if (!ip) {
        IPAddress remote = udp.remoteIP();
        snprintf(host_ip, sizeof(host_ip), "%d.%d.%d.%d",
                 remote[0], remote[1], remote[2], remote[3]);
    } else {
        strlcpy(host_ip, ip, sizeof(host_ip));
    }

    if (name) {
        strlcpy(host_name, name, sizeof(host_name));
    } else {
        host_name[0] = '\0';
    }

    host_port = port;
    host_found = true;
}

static void send_frame(uint8_t type, const uint8_t *payload, uint16_t payload_len)
{
    if (!tcp_client.connected()) return;

    uint8_t header[5];
    header[0] = FRAME_MAGIC_0;
    header[1] = FRAME_MAGIC_1;
    header[2] = type;
    header[3] = (payload_len >> 8) & 0xFF;
    header[4] = payload_len & 0xFF;

    tcp_client.write(header, 5);
    if (payload_len > 0 && payload) {
        tcp_client.write(payload, payload_len);
    }
}

static void send_heartbeat(void)
{
    StaticJsonDocument<64> doc;
    doc["cmd"] = "ping";
    char buf[64];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    send_frame(FRAME_CONTROL, (const uint8_t *)buf, (uint16_t)len);
}

static void handle_control_json(const char *json_str)
{
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, json_str);
    if (err) return;

    const char *cmd = doc["cmd"];
    if (!cmd) return;

    if (strcmp(cmd, "pong") == 0 || strcmp(cmd, "hello_ack") == 0) {
        last_pong_ms = millis();
    }
}

void voice_network_init(void)
{
    udp.begin(VOICE_DISCOVERY_PORT);
    voice_state = VOICE_IDLE;
    host_found = false;
    host_ip[0] = '\0';
    host_name[0] = '\0';
    last_discovery_ms = 0;
    last_heartbeat_ms = 0;
    last_pong_ms = 0;
}

void voice_network_update(void)
{
    if (!network_is_connected()) {
        if (tcp_client.connected()) {
            tcp_client.stop();
        }
        if (voice_state != VOICE_IDLE) {
            voice_state = VOICE_IDLE;
        }
        return;
    }

    process_udp_inbound();

    unsigned long now = millis();

    switch (voice_state) {
        case VOICE_IDLE:
            break;

        case VOICE_DISCOVERING: {
            if (now - last_discovery_ms >= DISCOVERY_INTERVAL_MS) {
                send_discovery_broadcast();
                last_discovery_ms = now;
            }
            if (host_found) {
                voice_connect_to_host(host_ip, host_port);
            }
            break;
        }

        case VOICE_CONNECTING:
            if (tcp_client.connected()) {
                voice_state = VOICE_CONNECTED;
                last_heartbeat_ms = now;
                last_pong_ms = now;
            } else if (now - last_discovery_ms >= 10000) {
                // connection attempt timed out, go back to discovering
                voice_state = VOICE_DISCOVERING;
                last_discovery_ms = now;
            }
            break;

        case VOICE_CONNECTED:
        case VOICE_TRANSMITTING: {
            if (!tcp_client.connected()) {
                voice_disconnect();
                voice_start_discovery();
                break;
            }

            if (now - last_heartbeat_ms >= HEARTBEAT_INTERVAL_MS) {
                send_heartbeat();
                last_heartbeat_ms = now;
            }

            if (now - last_pong_ms >= HEARTBEAT_TIMEOUT_MS) {
                // heartbeat timeout
                tcp_client.stop();
                voice_state = VOICE_IDLE;
                host_found = false;
                voice_start_discovery();
            }
            break;
        }
    }
}

voice_state_t voice_get_state(void)
{
    return voice_state;
}

voice_mode_t voice_get_mode(void)
{
    return voice_mode;
}

const char* voice_get_host_ip(void)
{
    return host_ip;
}

const char* voice_get_host_name(void)
{
    return host_name;
}

void voice_set_mode(voice_mode_t mode)
{
    voice_mode = mode;
}

void voice_start_discovery(void)
{
    voice_state = VOICE_DISCOVERING;
    host_found = false;
    host_ip[0] = '\0';
    host_name[0] = '\0';
    last_discovery_ms = 0;
    last_heartbeat_ms = 0;
    last_pong_ms = 0;
    if (tcp_client.connected()) {
        tcp_client.stop();
    }
}

void voice_connect_to_host(const char *ip, uint16_t port)
{
    if (!ip || !network_is_connected()) return;

    voice_state = VOICE_CONNECTING;
    strlcpy(host_ip, ip, sizeof(host_ip));
    host_port = port;

    tcp_client.stop();
    tcp_client.connect(ip, port);

    // send hello control frame with current mode
    StaticJsonDocument<128> doc;
    doc["cmd"] = "hello";
    doc["version"] = 1;
    doc["mode"] = (voice_mode == VOICE_MODE_PTT) ? "ptt" : "duplex";
    char buf[128];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    send_frame(FRAME_CONTROL, (const uint8_t *)buf, (uint16_t)len);

    last_discovery_ms = millis();
}

void voice_disconnect(void)
{
    if (tcp_client.connected()) {
        tcp_client.stop();
    }
    voice_state = VOICE_IDLE;
    host_found = false;
}

void voice_ptt_set(bool pressed)
{
    if (voice_mode != VOICE_MODE_PTT) return;
    if (voice_state != VOICE_CONNECTED && voice_state != VOICE_TRANSMITTING) return;
    if (!tcp_client.connected()) return;

    StaticJsonDocument<64> doc;
    doc["cmd"] = "ptt";
    doc["state"] = pressed ? "pressed" : "released";
    char buf[64];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    send_frame(FRAME_CONTROL, (const uint8_t *)buf, (uint16_t)len);

    voice_state = pressed ? VOICE_TRANSMITTING : VOICE_CONNECTED;
}

bool voice_send_audio_frame(const int8_t *pcm_data, size_t len)
{
    if (!tcp_client.connected()) return false;
    if (voice_state != VOICE_CONNECTED && voice_state != VOICE_TRANSMITTING) return false;
    if (len > 65535) return false;

    send_frame(FRAME_AUDIO_UPLINK, (const uint8_t *)pcm_data, (uint16_t)len);
    return true;
}

size_t voice_recv_audio_frame(uint8_t *pcm_buffer, size_t buf_len)
{
    if (!tcp_client.connected()) return 0;
    if (tcp_client.available() < 5) return 0;

    uint8_t header[5];
    tcp_client.read(header, 5);

    if (header[0] != FRAME_MAGIC_0 || header[1] != FRAME_MAGIC_1) {
        // desync: drain one byte and return 0 for now
        return 0;
    }

    uint8_t frame_type = header[2];
    uint16_t payload_len = ((uint16_t)header[3] << 8) | header[4];

    if (frame_type == FRAME_AUDIO_DOWNLINK) {
        if (payload_len == 0) return 0;
        size_t to_read = (payload_len > buf_len) ? buf_len : payload_len;

        unsigned long start = millis();
        while ((size_t)tcp_client.available() < payload_len) {
            if (millis() - start > 100) return 0;
            delay(1);
        }

        size_t read_len = tcp_client.read(pcm_buffer, to_read);
        // drain any remaining payload bytes not fitting in buffer
        if (payload_len > to_read) {
            uint8_t drain[64];
            size_t remaining = payload_len - to_read;
            while (remaining > 0) {
                size_t chunk = (remaining > sizeof(drain)) ? sizeof(drain) : remaining;
                size_t drained = tcp_client.read(drain, chunk);
                if (drained == 0) break;
                remaining -= drained;
            }
        }
        return read_len;
    }

    if (frame_type == FRAME_CONTROL) {
        if (payload_len == 0) return 0;
        unsigned long start = millis();
        while ((size_t)tcp_client.available() < payload_len) {
            if (millis() - start > 100) return 0;
            delay(1);
        }

        char *json_buf = (char *)malloc(payload_len + 1);
        if (!json_buf) {
            // drain payload
            uint8_t drain[64];
            size_t remaining = payload_len;
            while (remaining > 0) {
                size_t chunk = (remaining > sizeof(drain)) ? sizeof(drain) : remaining;
                size_t drained = tcp_client.read(drain, chunk);
                if (drained == 0) break;
                remaining -= drained;
            }
            return 0;
        }

        tcp_client.read((uint8_t *)json_buf, payload_len);
        json_buf[payload_len] = '\0';
        handle_control_json(json_buf);
        free(json_buf);
        return 0;
    }

    // unknown frame type: drain payload
    if (payload_len > 0) {
        uint8_t drain[64];
        size_t remaining = payload_len;
        while (remaining > 0) {
            size_t chunk = (remaining > sizeof(drain)) ? sizeof(drain) : remaining;
            size_t drained = tcp_client.read(drain, chunk);
            if (drained == 0) break;
            remaining -= drained;
        }
    }
    return 0;
}

bool voice_send_control(const char *json_str)
{
    if (!tcp_client.connected()) return false;
    if (!json_str) return false;

    size_t len = strlen(json_str);
    if (len > 65535) return false;

    send_frame(FRAME_CONTROL, (const uint8_t *)json_str, (uint16_t)len);
    return true;
}
