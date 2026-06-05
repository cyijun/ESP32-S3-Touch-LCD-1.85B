#include "kimi_api.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

kimi_usage_t g_kimi_data;
bool g_ui_needs_update = false;

static float parse_f64(const JsonVariant &value) {
    if (value.is<float>()) {
        return value.as<float>();
    }
    if (value.is<int>()) {
        return static_cast<float>(value.as<int>());
    }
    if (value.is<long>()) {
        return static_cast<float>(value.as<long>());
    }
    if (value.is<const char*>()) {
        return atof(value.as<const char*>());
    }
    return 0.0f;
}

static String timestamp_to_iso8601(unsigned long long ms) {
    time_t sec = static_cast<time_t>(ms / 1000ULL);
    struct tm *tm_info = gmtime(&sec);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm_info);
    return String(buf);
}

static String extract_reset_time(const JsonVariant &value) {
    if (value.is<const char*>()) {
        return String(value.as<const char*>());
    }
    if (value.is<int>() || value.is<long>() || value.is<unsigned long>() || value.is<unsigned long long>()) {
        unsigned long long ts = value.as<unsigned long long>();
        if (ts < 1000000000000ULL) {
            ts *= 1000ULL;
        }
        return timestamp_to_iso8601(ts);
    }
    return "";
}

void kimi_api_init(void) {
    memset(&g_kimi_data, 0, sizeof(g_kimi_data));
    g_kimi_data.api_ok = false;
    g_kimi_data.last_error = "";
    g_kimi_data.last_update_ms = 0;
    g_ui_needs_update = false;
}

void kimi_api_refresh_now(void) {
    if (WiFi.status() != WL_CONNECTED) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "WiFi not connected";
        g_ui_needs_update = true;
        return;
    }

    HTTPClient http;
    http.setTimeout(10000);
    http.begin("https://api.kimi.com/coding/v1/usages");
    http.addHeader("Authorization", String("Bearer ") + KIMI_API_KEY);
    http.addHeader("Accept", "application/json");

    int httpCode = http.GET();

    if (httpCode < 0) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "Network error: " + String(http.errorToString(httpCode).c_str());
        g_ui_needs_update = true;
        http.end();
        return;
    }

    if (httpCode == 401 || httpCode == 403) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "Invalid API Key";
        g_ui_needs_update = true;
        http.end();
        return;
    }

    if (httpCode >= 500) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "Server error: HTTP " + String(httpCode);
        g_ui_needs_update = true;
        http.end();
        return;
    }

    if (httpCode != 200) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "HTTP error: " + String(httpCode);
        g_ui_needs_update = true;
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "JSON parse error: " + String(err.c_str());
        g_ui_needs_update = true;
        return;
    }

    // Parse 5-hour window data from limits array
    bool found_window = false;
    if (doc.containsKey("limits") && doc["limits"].is<JsonArray>()) {
        JsonArray limits = doc["limits"];
        for (JsonObject item : limits) {
            if (item.containsKey("detail")) {
                JsonObject detail = item["detail"];
                g_kimi_data.window_limit = parse_f64(detail["limit"]);
                g_kimi_data.window_remaining = parse_f64(detail["remaining"]);
                g_kimi_data.window_used = max(g_kimi_data.window_limit - g_kimi_data.window_remaining, 0.0f);
                if (g_kimi_data.window_limit > 0.0f) {
                    g_kimi_data.window_pct = (g_kimi_data.window_used / g_kimi_data.window_limit) * 100.0f;
                } else {
                    g_kimi_data.window_pct = 0.0f;
                }
                g_kimi_data.window_reset_time = extract_reset_time(detail["resetTime"]);
                found_window = true;
                break;
            }
        }
    }

    // Parse weekly limit data from usage object
    bool found_week = false;
    if (doc.containsKey("usage")) {
        JsonObject usage = doc["usage"];
        g_kimi_data.week_limit = parse_f64(usage["limit"]);
        g_kimi_data.week_remaining = parse_f64(usage["remaining"]);
        g_kimi_data.week_used = max(g_kimi_data.week_limit - g_kimi_data.week_remaining, 0.0f);
        if (g_kimi_data.week_limit > 0.0f) {
            g_kimi_data.week_pct = (g_kimi_data.week_used / g_kimi_data.week_limit) * 100.0f;
        } else {
            g_kimi_data.week_pct = 0.0f;
        }
        g_kimi_data.week_reset_time = extract_reset_time(usage["resetTime"]);
        found_week = true;
    }

    if (!found_window && !found_week) {
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "JSON parse error: expected fields not found";
        g_ui_needs_update = true;
        return;
    }

    g_kimi_data.api_ok = true;
    g_kimi_data.last_error = "";
    g_kimi_data.last_update_ms = millis();
    g_ui_needs_update = true;
}

void kimi_api_timer_cb(lv_timer_t *timer) {
    (void)timer;
    if (WiFi.status() == WL_CONNECTED) {
        kimi_api_refresh_now();
    }
}
