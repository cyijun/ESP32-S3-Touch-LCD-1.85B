#ifndef PET_SPRITES_H
#define PET_SPRITES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PET_CAT = 0,
    PET_ROBOT,
    PET_GHOST,
    PET_BLOB,
    PET_DUCK,
    PET_SPECIES_COUNT
} PetSpecies;

typedef enum {
    EYE_O = 0,    // 'o'
    EYE_BIG,      // 'O'
    EYE_DOT,      // '.'
    EYE_STAR,     // '*'
    EYE_X,        // 'x'
    EYE_DASH,     // '-'
    EYE_COUNT
} PetEye;

typedef enum {
    RARITY_COMMON = 0,
    RARITY_UNCOMMON,
    RARITY_RARE,
    RARITY_EPIC,
    RARITY_LEGENDARY,
    RARITY_COUNT
} PetRarity;

#define PET_STAT_COUNT 5
typedef enum {
    STAT_DEBUG = 0,
    STAT_PATIENCE,
    STAT_CHAOS,
    STAT_WISDOM,
    STAT_SNARK,
} PetStat;

typedef struct {
    PetSpecies species;
    PetEye eye;
    PetRarity rarity;
    uint8_t stats[PET_STAT_COUNT];
    char name[16];
} PetData;

typedef struct {
    const char* frame0;
    const char* frame1;
    const char* frame2;
    uint8_t eye_count; // 1 or 2
} SpriteTemplate;

extern const SpriteTemplate SPECIES_TEMPLATES[PET_SPECIES_COUNT];
extern const char* SPECIES_NAMES[PET_SPECIES_COUNT];
extern const char* RARITY_NAMES[RARITY_COUNT];
extern const char* RARITY_STARS[RARITY_COUNT];
extern const char EYE_CHARS[EYE_COUNT];
extern const char* STAT_LABELS[PET_STAT_COUNT];
extern const char** NAME_POOLS[PET_SPECIES_COUNT];
extern const int NAME_POOL_SIZES[PET_SPECIES_COUNT];

extern PetData g_pet;
void pet_generate(void);
uint32_t pet_rng_next(void);
uint32_t pet_rng_range(uint32_t max);

#ifdef __cplusplus
}
#endif

#endif
