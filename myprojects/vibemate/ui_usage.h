#ifndef UI_USAGE_H
#define UI_USAGE_H

#include <lvgl.h>
#include "kimi_api.h"

void ui_usage_create(lv_obj_t *parent_tile);
void ui_usage_update(const kimi_usage_t *data);

#endif
