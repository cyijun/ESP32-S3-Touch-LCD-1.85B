#ifndef PET_SPRITES_H
#define PET_SPRITES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DUCK = 0,
    GOOSE,
    BLOB,
    CAT,
    DRAGON,
    OCTOPUS,
    OWL,
    PENGUIN,
    TURTLE,
    SNAIL,
    GHOST,
    AXOLOTL,
    CAPYBARA,
    CACTUS,
    ROBOT,
    RABBIT,
    MUSHROOM,
    CHONK,
    PET_SPECIES_COUNT
} PetSpecies;

typedef enum {
    EYE_DOT = 0,     // '·'
    EYE_STAR,        // '✦'
    EYE_X,           // '×'
    EYE_CIRCLE,      // '◉'
    EYE_AT,          // '@'
    EYE_DEG,         // '°'
    EYE_COUNT
} PetEye;

typedef enum {
    HAT_NONE = 0,
    HAT_CROWN,
    HAT_TOPHAT,
    HAT_PROPELLER,
    HAT_HALO,
    HAT_WIZARD,
    HAT_BEANIE,
    HAT_TINYDUCK,
    HAT_COUNT
} PetHat;

typedef enum {
    COLOR_CYAN = 0,    // 0x3DD9D0
    COLOR_PINK,        // 0xFF6B9D
    COLOR_GOLD,        // 0xFFD166
    COLOR_PURPLE,      // 0xC85EFF
    COLOR_GREEN,       // 0x5EE7A0
    COLOR_BLUE,        // 0x6B8CFF
    COLOR_ORANGE,      // 0xFF9F43
    COLOR_WHITE,       // 0xE8E8ED
    COLOR_COUNT
} PetColor;

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
    PetHat hat;
    PetColor color;
    PetRarity rarity;
    uint8_t shiny;
    uint8_t stats[PET_STAT_COUNT];
    uint8_t hunger;
    uint8_t joy;
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
extern const char* EYE_STRINGS[EYE_COUNT];
extern const char* HAT_NAMES[HAT_COUNT];
extern const char* HAT_LINES[HAT_COUNT];
extern const char* COLOR_NAMES[COLOR_COUNT];
extern const uint32_t COLOR_HEX[COLOR_COUNT];
extern const char* STAT_LABELS[PET_STAT_COUNT];
extern const char** NAME_POOLS[PET_SPECIES_COUNT];
extern const int NAME_POOL_SIZES[PET_SPECIES_COUNT];

extern PetData g_pet;
void pet_generate(void);
void pet_reset_stats(void);
uint32_t pet_rng_next(void);
uint32_t pet_rng_range(uint32_t max);

#ifdef __cplusplus
}
#endif

#endif
