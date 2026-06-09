#include "ui_anim.h"

typedef struct {
    lv_timer_t **slot;
    bool is_loop;
} UiAnimEntry;

// Oneshot wrapper state: maps a timer pointer to the user's original callback.
// We can't allocate dynamically, so use a small fixed-size pool.
#define UI_ANIM_ONESHOT_MAX 16

typedef struct {
    lv_timer_t *timer;
    lv_timer_cb_t user_cb;
    lv_timer_t **slot;
} UiAnimOneshot;

static UiAnimEntry s_registry[UI_ANIM_MAX_SLOTS];
static int s_count = 0;

static UiAnimOneshot s_oneshots[UI_ANIM_ONESHOT_MAX];

static int s_find_index(lv_timer_t **slot)
{
    for (int i = 0; i < s_count; i++) {
        if (s_registry[i].slot == slot) {
            return i;
        }
    }
    return -1;
}

static void s_remove_at(int idx)
{
    if (idx < 0 || idx >= s_count) {
        return;
    }
    for (int i = idx; i < s_count - 1; i++) {
        s_registry[i] = s_registry[i + 1];
    }
    s_count--;
}

static int s_oneshot_find(lv_timer_t *timer)
{
    for (int i = 0; i < UI_ANIM_ONESHOT_MAX; i++) {
        if (s_oneshots[i].timer == timer) {
            return i;
        }
    }
    return -1;
}

static int s_oneshot_alloc(void)
{
    for (int i = 0; i < UI_ANIM_ONESHOT_MAX; i++) {
        if (s_oneshots[i].timer == NULL) {
            return i;
        }
    }
    return -1;
}

static void s_oneshot_free_by_timer(lv_timer_t *timer)
{
    int idx = s_oneshot_find(timer);
    if (idx >= 0) {
        s_oneshots[idx].timer = NULL;
        s_oneshots[idx].user_cb = NULL;
        s_oneshots[idx].slot = NULL;
    }
}

// Wrapper callback for one-shot timers.
// Calls the user's callback, then clears the slot and cleans up.
static void s_oneshot_wrapper_cb(lv_timer_t *t)
{
    int idx = s_oneshot_find(t);
    if (idx < 0) {
        // orphaned wrapper; just delete ourselves
        lv_timer_del(t);
        return;
    }

    lv_timer_cb_t user_cb = s_oneshots[idx].user_cb;
    lv_timer_t **slot = s_oneshots[idx].slot;

    // Free the oneshot record before calling user cb,
    // in case the user cb tries to restart a timer on the same slot.
    s_oneshots[idx].timer = NULL;
    s_oneshots[idx].user_cb = NULL;
    s_oneshots[idx].slot = NULL;

    if (slot) {
        *slot = NULL;
    }

    if (user_cb) {
        user_cb(t);
    }

    lv_timer_del(t);
}

void ui_anim_register(lv_timer_t **slot)
{
    if (!slot) {
        return;
    }
    if (s_find_index(slot) >= 0) {
        return;
    }
    if (s_count >= UI_ANIM_MAX_SLOTS) {
        return;
    }
    s_registry[s_count].slot = slot;
    s_registry[s_count].is_loop = false;
    s_count++;
}

void ui_anim_start_loop(lv_timer_t **slot, lv_timer_cb_t cb, uint32_t period_ms)
{
    if (!slot || !cb) {
        return;
    }
    ui_anim_stop(slot);
    *slot = lv_timer_create(cb, period_ms, NULL);
    int idx = s_find_index(slot);
    if (idx >= 0) {
        s_registry[idx].is_loop = true;
    } else if (s_count < UI_ANIM_MAX_SLOTS) {
        s_registry[s_count].slot = slot;
        s_registry[s_count].is_loop = true;
        s_count++;
    }
}

void ui_anim_start_once(lv_timer_t **slot, lv_timer_cb_t cb, uint32_t period_ms)
{
    if (!slot || !cb) {
        return;
    }
    ui_anim_stop(slot);

    int os_idx = s_oneshot_alloc();
    if (os_idx < 0) {
        // oneshot pool exhausted; fall back to creating the timer directly
        // without auto-cleanup wrapper. The caller must manage it.
        *slot = lv_timer_create(cb, period_ms, NULL);
        return;
    }

    *slot = lv_timer_create(s_oneshot_wrapper_cb, period_ms, NULL);
    s_oneshots[os_idx].timer = *slot;
    s_oneshots[os_idx].user_cb = cb;
    s_oneshots[os_idx].slot = slot;

    int idx = s_find_index(slot);
    if (idx >= 0) {
        s_registry[idx].is_loop = false;
    } else if (s_count < UI_ANIM_MAX_SLOTS) {
        s_registry[s_count].slot = slot;
        s_registry[s_count].is_loop = false;
        s_count++;
    }
}

void ui_anim_stop(lv_timer_t **slot)
{
    if (!slot) {
        return;
    }
    if (*slot) {
        // If this is a wrapped oneshot, free its record first
        s_oneshot_free_by_timer(*slot);
        lv_timer_del(*slot);
        *slot = NULL;
    }
}

void ui_anim_pause_all(void)
{
    for (int i = 0; i < s_count; i++) {
        if (s_registry[i].is_loop && s_registry[i].slot && *s_registry[i].slot) {
            lv_timer_pause(*s_registry[i].slot);
        }
    }
}

void ui_anim_resume_all(void)
{
    for (int i = 0; i < s_count; i++) {
        if (s_registry[i].is_loop && s_registry[i].slot && *s_registry[i].slot) {
            lv_timer_resume(*s_registry[i].slot);
        }
    }
}

void ui_anim_stop_all(void)
{
    for (int i = 0; i < s_count; i++) {
        if (s_registry[i].slot && *s_registry[i].slot) {
            s_oneshot_free_by_timer(*s_registry[i].slot);
            lv_timer_del(*s_registry[i].slot);
            *s_registry[i].slot = NULL;
        }
    }
}

void ui_anim_reset(void)
{
    s_count = 0;
    for (int i = 0; i < UI_ANIM_ONESHOT_MAX; i++) {
        s_oneshots[i].timer = NULL;
        s_oneshots[i].user_cb = NULL;
        s_oneshots[i].slot = NULL;
    }
}
