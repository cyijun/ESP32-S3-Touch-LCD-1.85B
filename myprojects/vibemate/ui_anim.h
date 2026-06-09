#ifndef UI_ANIM_H
#define UI_ANIM_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_ANIM_MAX_SLOTS 32

// Register a timer slot so the manager can track it.
// Call once per slot (typically right after declaring the static variable).
void ui_anim_register(lv_timer_t **slot);

// Start a looping timer. The slot will be managed for pause/resume/stop_all.
void ui_anim_start_loop(lv_timer_t **slot, lv_timer_cb_t cb, uint32_t period_ms);

// Start a one-shot timer. It auto-deletes after firing, and the manager
// clears the slot to NULL automatically.
void ui_anim_start_once(lv_timer_t **slot, lv_timer_cb_t cb, uint32_t period_ms);

// Safely stop a single animation: delete the timer and set *slot = NULL.
void ui_anim_stop(lv_timer_t **slot);

// Pause all registered loop timers.
void ui_anim_pause_all(void);

// Resume all registered loop timers.
void ui_anim_resume_all(void);

// Stop all registered timers (loop and one-shot).
void ui_anim_stop_all(void);

// Clear the registry. Does NOT delete timers — call ui_anim_stop_all() first
// if you need them stopped.
void ui_anim_reset(void);

#ifdef __cplusplus
}
#endif

#endif
