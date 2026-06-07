#include "kimi_api.h"
#include "config.h"
#include "debug_trace.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

kimi_usage_t g_kimi_data;
bool g_ui_needs_update = false;

static volatile bool s_request_busy = false;
static TaskHandle_t s_api_task_handle = NULL;

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

static void s_do_http_request(kimi_usage_t *out)
{
    memset(out, 0, sizeof(kimi_usage_t));
    out->api_ok = false;

    if (WiFi.status() != WL_CONNECTED) {
        out->last_error = "WiFi not connected";
        return;
    }

    TRACE_KIMI("HTTP begin");
    HTTPClient http;
    http.setTimeout(10000);
    http.begin("https://api.kimi.com/coding/v1/usages");
    http.addHeader("Authorization", String("Bearer ") + KIMI_API_KEY);
    http.addHeader("Accept", "application/json");

    int httpCode = http.GET();
    TRACE_KIMI("HTTP code=%d", httpCode);

    if (httpCode < 0) {
        out->last_error = "Network error: " + String(http.errorToString(httpCode).c_str());
        http.end();
        return;
    }

    if (httpCode == 401 || httpCode == 403) {
        out->last_error = "Invalid API Key";
        http.end();
        return;
    }

    if (httpCode >= 500) {
        out->last_error = "Server error: HTTP " + String(httpCode);
        http.end();
        return;
    }

    if (httpCode != 200) {
        out->last_error = "HTTP error: " + String(httpCode);
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
        out->last_error = "JSON parse error: " + String(err.c_str());
        return;
    }

    bool found_window = false;
    if (doc.containsKey("limits") && doc["limits"].is<JsonArray>()) {
        JsonArray limits = doc["limits"];
        for (JsonObject item : limits) {
            if (item.containsKey("detail")) {
                JsonObject detail = item["detail"];
                out->window_limit = parse_f64(detail["limit"]);
                out->window_remaining = parse_f64(detail["remaining"]);
                out->window_used = max(out->window_limit - out->window_remaining, 0.0f);
                if (out->window_limit > 0.0f) {
                    out->window_pct = (out->window_used / out->window_limit) * 100.0f;
                } else {
                    out->window_pct = 0.0f;
                }
                out->window_reset_time = extract_reset_time(detail["resetTime"]);
                found_window = true;
                break;
            }
        }
    }

    bool found_week = false;
    if (doc.containsKey("usage")) {
        JsonObject usage = doc["usage"];
        out->week_limit = parse_f64(usage["limit"]);
        out->week_remaining = parse_f64(usage["remaining"]);
        out->week_used = max(out->week_limit - out->week_remaining, 0.0f);
        if (out->week_limit > 0.0f) {
            out->week_pct = (out->week_used / out->week_limit) * 100.0f;
        } else {
            out->week_pct = 0.0f;
        }
        out->week_reset_time = extract_reset_time(usage["resetTime"]);
        found_week = true;
    }

    if (!found_window && !found_week) {
        out->last_error = "JSON parse error: expected fields not found";
        return;
    }

    out->api_ok = true;
    out->last_error = "";
    out->last_update_ms = millis();
}

static void s_api_task(void *arg)
{
    (void)arg;
    TRACE_KIMI("API task start");

    kimi_usage_t result;
    s_do_http_request(&result);

    g_kimi_data = result;
    g_ui_needs_update = true;
    TRACE_KIMI("API task done ok=%d", g_kimi_data.api_ok);

    s_request_busy = false;
    s_api_task_handle = NULL;
    vTaskDelete(NULL);
}

void kimi_api_refresh_now(void) {
    if (s_request_busy) {
        TRACE_KIMI("refresh skipped, busy");
        return;
    }

    s_request_busy = true;
    BaseType_t rc = xTaskCreate(s_api_task, "kimi_api", 12288, NULL, 1, &s_api_task_handle);
    if (rc != pdPASS) {
        TRACE_KIMI("xTaskCreate failed rc=%d", rc);
        s_request_busy = false;
        s_api_task_handle = NULL;
        g_kimi_data.api_ok = false;
        g_kimi_data.last_error = "Task create failed";
        g_ui_needs_update = true;
    }
}

void kimi_api_timer_cb(lv_timer_t *timer) {
    (void)timer;
    if (WiFi.status() == WL_CONNECTED) {
        kimi_api_refresh_now();
    }
}
