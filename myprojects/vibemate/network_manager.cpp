#include "network_manager.h"
#include "config.h"
#include "rtc_bsp.h"
#include <time.h>

static unsigned long last_reconnect_attempt = 0;
static const unsigned long RECONNECT_INTERVAL_MS = 5000;

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
        last_reconnect_attempt = millis();
    }
}

void network_check(void) {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    unsigned long now = millis();
    if (now - last_reconnect_attempt < RECONNECT_INTERVAL_MS) {
        return;
    }
    last_reconnect_attempt = now;

    Serial.println("[Network] Disconnected, attempting reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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

bool network_sync_ntp_to_rtc(void) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NTP] WiFi not connected, cannot sync");
        return false;
    }

    Serial.println("[NTP] Starting NTP sync...");
    configTime(8 * 3600, 0, "pool.ntp.org");

    struct tm timeinfo;
    int retries = 0;
    while (!getLocalTime(&timeinfo) && retries < 30) {
        delay(500);
        retries++;
        Serial.print(".");
    }
    Serial.println();

    if (!getLocalTime(&timeinfo)) {
        Serial.println("[NTP] Failed to get time from NTP server");
        return false;
    }

    int year = timeinfo.tm_year + 1900;
    int month = timeinfo.tm_mon + 1;
    int day = timeinfo.tm_mday;
    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    i2c_rtc_setTime(year, month, day, hour, minute, second);

    Serial.printf("[NTP] RTC set to %04d-%02d-%02d %02d:%02d:%02d (UTC+8)\n",
                  year, month, day, hour, minute, second);
    return true;
}
