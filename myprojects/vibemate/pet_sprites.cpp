#include "pet_sprites.h"
#include <Esp.h>
#include <string.h>

const SpriteTemplate SPECIES_TEMPLATES[PET_SPECIES_COUNT] = {
    // PET_CAT
    {
        "  /\\_/\\  \n ( %c   %c )\n (   -   )\n (\")_(\") ",
        "  /\\_/\\  \n ( %c   %c )\n (   -   )\n (\")_(\")~",
        "  /\\-/-\\  \n ( %c   %c )\n (   -   )\n (\")_(\") ",
        2
    },
    // PET_ROBOT
    {
        "  .[||].  \n [ %c   %c ]\n [ ==== ]\n `------´",
        "  .[||].  \n [ %c   %c ]\n [ -==- ]\n `------´",
        "     *    \n  .[||].  \n [ %c   %c ]\n [ ==== ]\n `------´",
        2
    },
    // PET_GHOST
    {
        "  .----.  \n / %c   %c \\ \n |      | \n ~`~``~`~ ",
        "  .----.  \n / %c   %c \\ \n |      | \n `~`~~`~` ",
        "    ~  ~  \n  .----.  \n / %c   %c \\ \n |      | \n ~~`~~`~~ ",
        2
    },
    // PET_BLOB
    {
        "  .----.  \n ( %c   %c  )\n (      ) \n  `----´  ",
        " .------. \n(  %c   %c  )\n(        )\n `------´ ",
        "    .--.  \n   (%c  %c)   \n   (    )  \n    `--´   ",
        2
    },
    // PET_DUCK
    {
        "    __    \n  <(%c )___ \n   (  ._-> \n    `--´   ",
        "    __    \n  <(%c )___ \n   (  ._-> \n    `--´~  ",
        "    __    \n  <(%c )___ \n   (  .__> \n    `--´   ",
        1
    },
};

const char* SPECIES_NAMES[PET_SPECIES_COUNT] = {
    "Cat", "Robot", "Ghost", "Blob", "Duck"
};

const char* RARITY_NAMES[RARITY_COUNT] = {
    "Common", "Uncommon", "Rare", "Epic", "Legendary"
};

const char* RARITY_STARS[RARITY_COUNT] = {
    "*", "**", "***", "****", "*****"
};

const char EYE_CHARS[EYE_COUNT] = {
    'o', 'O', '.', '*', 'x', '-'
};

const char* STAT_LABELS[PET_STAT_COUNT] = {
    "DEBUG", "PATIENCE", "CHAOS", "WISDOM", "SNARK"
};

// Name pools: 8 names per species
static const char* NAME_POOL_CAT[]     = {"Whiskers", "Mittens", "Luna", "Simba", "Nala", "Oliver", "Milo", "Kitty"};
static const char* NAME_POOL_ROBOT[]   = {"Byte", "Pixel", "Spark", "Bolt", "Chip", "Turing", "Ada", "Unit-01"};
static const char* NAME_POOL_GHOST[]   = {"Boo", "Casper", "Phantom", "Specter", "Wisp", "Shade", "Spirit", "Echo"};
static const char* NAME_POOL_BLOB[]    = {"Gloop", "Bloop", "Squish", "Gelatin", "Ooze", "Slime", "Pudding", "Jelly"};
static const char* NAME_POOL_DUCK[]    = {"Quackers", "Daffy", "Waddles", "Webster", "Puddles", "Ducky", "Bill", "Howard"};

const char** NAME_POOLS[PET_SPECIES_COUNT] = {
    NAME_POOL_CAT,
    NAME_POOL_ROBOT,
    NAME_POOL_GHOST,
    NAME_POOL_BLOB,
    NAME_POOL_DUCK,
};

const int NAME_POOL_SIZES[PET_SPECIES_COUNT] = {
    8, 8, 8, 8, 8
};

// ========== Pet Generation ==========
PetData g_pet;

static uint32_t s_rng_state;

uint32_t pet_rng_next(void)
{
    s_rng_state = s_rng_state * 1103515245u + 12345u;
    return s_rng_state;
}

uint32_t pet_rng_range(uint32_t max)
{
    return pet_rng_next() % max;
}

void pet_generate(void)
{
    uint64_t mac = ESP.getEfuseMac();
    s_rng_state = (uint32_t)(mac ^ (mac >> 32));

    // Rarity (weighted)
    uint32_t r = pet_rng_range(100);
    if (r < 60) g_pet.rarity = RARITY_COMMON;
    else if (r < 85) g_pet.rarity = RARITY_UNCOMMON;
    else if (r < 95) g_pet.rarity = RARITY_RARE;
    else if (r < 99) g_pet.rarity = RARITY_EPIC;
    else g_pet.rarity = RARITY_LEGENDARY;

    g_pet.species = (PetSpecies)pet_rng_range(PET_SPECIES_COUNT);
    g_pet.eye = (PetEye)pet_rng_range(EYE_COUNT);

    // Stats: one peak, one dump, rest scattered
    uint8_t peak = (uint8_t)pet_rng_range(PET_STAT_COUNT);
    uint8_t dump = (uint8_t)pet_rng_range(PET_STAT_COUNT);
    while (dump == peak) dump = (uint8_t)pet_rng_range(PET_STAT_COUNT);

    uint8_t floor = 5 + g_pet.rarity * 10;
    for (int i = 0; i < PET_STAT_COUNT; i++) {
        if (i == peak) {
            uint32_t v = floor + 50 + pet_rng_range(30);
            g_pet.stats[i] = (uint8_t)(v > 100 ? 100 : v);
        } else if (i == dump) {
            uint32_t v = floor - 10 + pet_rng_range(15);
            g_pet.stats[i] = (uint8_t)(v < 1 ? 1 : v);
        } else {
            g_pet.stats[i] = (uint8_t)(floor + pet_rng_range(40));
        }
    }

    // Name from pool
    int pool_size = NAME_POOL_SIZES[g_pet.species];
    const char* chosen = NAME_POOLS[g_pet.species][pet_rng_range(pool_size)];
    strncpy(g_pet.name, chosen, sizeof(g_pet.name) - 1);
    g_pet.name[sizeof(g_pet.name) - 1] = '\0';
}
