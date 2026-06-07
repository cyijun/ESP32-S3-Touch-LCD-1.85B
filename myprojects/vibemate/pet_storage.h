#ifndef PET_STORAGE_H
#define PET_STORAGE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool pet_load(void);
void pet_save(void);
bool pet_has_saved(void);

#ifdef __cplusplus
}
#endif

#endif
