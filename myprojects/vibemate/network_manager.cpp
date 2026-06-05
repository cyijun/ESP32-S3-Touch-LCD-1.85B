#include "network_manager.h"
#include "config.h"

void network_init(void) {
    Serial.println("[Network] Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[Network] Connected, IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[Network] Failed to connect within 30 seconds.");
    }
}

void network_check(void) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Network] Disconnected, attempting reconnect...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("[Network] Reconnected, IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("[Network] Reconnect failed.");
        }
    }
}

bool network_is_connected(void) {
    return WiFi.status() == WL_CONNECTED;
}

String network_get_ip(void) {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "Disconnected";
}

int network_get_rssi(void) {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}
