#ifndef UI_PET_H
#define UI_PET_H
#include <lvgl.h>
#include "debug_trace.h"
void ui_pet_create(lv_obj_t *parent_tile);
void ui_pet_update(void);
void ui_pet_delete(void);
void ui_pet_pause_anim(void);
void ui_pet_resume_anim(void);
#endif
