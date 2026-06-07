#include "pet_sprites.h"
#include <Esp.h>
#include <string.h>

const SpriteTemplate SPECIES_TEMPLATES[PET_SPECIES_COUNT] = {
    // DUCK (1 eye)
    {
        "            \n"
        "    __      \n"
        "  <(%c )___  \n"
        "   (  ._>   \n"
        "    `--´    ",
        "            \n"
        "    __      \n"
        "  <(%c )___  \n"
        "   (  ._>   \n"
        "    `--´~   ",
        "            \n"
        "    __      \n"
        "  <(%c )___  \n"
        "   (  .__>  \n"
        "    `--´    ",
        1
    },
    // GOOSE (1 eye)
    {
        "            \n"
        "     (%c>    \n"
        "     ||     \n"
        "   _(__)_   \n"
        "    ^^^^    ",
        "            \n"
        "    (%c>     \n"
        "     ||     \n"
        "   _(__)_   \n"
        "    ^^^^    ",
        "            \n"
        "     (%c>>   \n"
        "     ||     \n"
        "   _(__)_   \n"
        "    ^^^^    ",
        1
    },
    // BLOB (2 eyes)
    {
        "            \n"
        "   .----.   \n"
        "  ( %c  %c )  \n"
        "  (      )  \n"
        "   `----´   ",
        "            \n"
        "  .------.  \n"
        " (  %c  %c  ) \n"
        " (        ) \n"
        "  `------´  ",
        "            \n"
        "    .--.    \n"
        "   (%c  %c)   \n"
        "   (    )   \n"
        "    `--´    ",
        2
    },
    // CAT (2 eyes)
    {
        "            \n"
        "   /\_/\    \n"
        "  ( %c   %c)  \n"
        "  (  ω  )   \n"
        "  (\")_(\")   ",
        "            \n"
        "   /\_/\    \n"
        "  ( %c   %c)  \n"
        "  (  ω  )   \n"
        "  (\")_(\")~  ",
        "            \n"
        "   /\-/\    \n"
        "  ( %c   %c)  \n"
        "  (  ω  )   \n"
        "  (\")_(\")   ",
        2
    },
    // DRAGON (2 eyes)
    {
        "            \n"
        "  /^\  /^\  \n"
        " <  %c  %c  > \n"
        " (   ~~   ) \n"
        "  `-vvvv-´  ",
        "            \n"
        "  /^\  /^\  \n"
        " <  %c  %c  > \n"
        " (        ) \n"
        "  `-vvvv-´  ",
        "   ~    ~   \n"
        "  /^\  /^\  \n"
        " <  %c  %c  > \n"
        " (   ~~   ) \n"
        "  `-vvvv-´  ",
        2
    },
    // OCTOPUS (2 eyes)
    {
        "            \n"
        "   .----.   \n"
        "  ( %c  %c )  \n"
        "  (______)  \n"
        "  /\/\/\/\  ",
        "            \n"
        "   .----.   \n"
        "  ( %c  %c )  \n"
        "  (______)  \n"
        "  \/\/\/\/  ",
        "     o      \n"
        "   .----.   \n"
        "  ( %c  %c )  \n"
        "  (______)  \n"
        "  /\/\/\/\  ",
        2
    },
    // OWL (2 eyes)
    {
        "            \n"
        "   /\  /\   \n"
        "  ((%c)(%c))  \n"
        "  (  ><  )  \n"
        "   `----´   ",
        "            \n"
        "   /\  /\   \n"
        "  ((%c)(%c))  \n"
        "  (  ><  )  \n"
        "   .----.   ",
        "            \n"
        "   /\  /\   \n"
        "  ((%c)(-))  \n"
        "  (  ><  )  \n"
        "   `----´   ",
        2
    },
    // PENGUIN (2 eyes)
    {
        "            \n"
        "  .---.     \n"
        "  (%c>%c)     \n"
        " /(   )\\    \n"
        "  `---´     ",
        "            \n"
        "  .---.     \n"
        "  (%c>%c)     \n"
        " |(   )|    \n"
        "  `---´     ",
        "  .---.     \n"
        "  (%c>%c)     \n"
        " /(   )\\    \n"
        "  `---´     \n"
        "   ~ ~      ",
        2
    },
    // TURTLE (2 eyes)
    {
        "            \n"
        "   _,--._   \n"
        "  ( %c  %c )  \n"
        " /[______]\\ \n"
        "  ``    ``  ",
        "            \n"
        "   _,--._   \n"
        "  ( %c  %c )  \n"
        " /[______]\\ \n"
        "   ``  ``   ",
        "            \n"
        "   _,--._   \n"
        "  ( %c  %c )  \n"
        " /[======]\\ \n"
        "  ``    ``  ",
        2
    },
    // SNAIL (1 eye)
    {
        "            \n"
        " %c    .--.  \n"
        "  \\  ( @ )  \n"
        "   \\_`--´   \n"
        "  ~~~~~~~   ",
        "            \n"
        "  %c   .--.  \n"
        "  |  ( @ )  \n"
        "   \\_`--´   \n"
        "  ~~~~~~~   ",
        "            \n"
        " %c    .--.  \n"
        "  \\  ( @  ) \n"
        "   \\_`--´   \n"
        "   ~~~~~~   ",
        1
    },
    // GHOST (2 eyes)
    {
        "            \n"
        "   .----.   \n"
        "  / %c  %c \\  \n"
        "  |      |  \n"
        "  ~`~``~`~  ",
        "            \n"
        "   .----.   \n"
        "  / %c  %c \\  \n"
        "  |      |  \n"
        "  `~`~~`~`  ",
        "    ~  ~    \n"
        "   .----.   \n"
        "  / %c  %c \\  \n"
        "  |      |  \n"
        "  ~~`~~`~~  ",
        2
    },
    // AXOLOTL (2 eyes)
    {
        "            \n"
        "}~(______)~{\n"
        "}~(%c .. %c)~{\n"
        "  ( .--. )  \n"
        "  (_/  \\_)  ",
        "            \n"
        "~}(______){~\n"
        "~}(%c .. %c){~\n"
        "  ( .--. )  \n"
        "  (_/  \\_)  ",
        "            \n"
        "}~(______)~{\n"
        "}~(%c .. %c)~{\n"
        "  (  --  )  \n"
        "  ~_/  \\_~  ",
        2
    },
    // CAPYBARA (2 eyes)
    {
        "            \n"
        "  n______n  \n"
        " ( %c    %c ) \n"
        " (   oo   ) \n"
        "  `------´  ",
        "            \n"
        "  n______n  \n"
        " ( %c    %c ) \n"
        " (   Oo   ) \n"
        "  `------´  ",
        "    ~  ~    \n"
        "  u______n  \n"
        " ( %c    %c ) \n"
        " (   oo   ) \n"
        "  `------´  ",
        2
    },
    // CACTUS (2 eyes)
    {
        "            \n"
        " n  ____  n \n"
        " | |%c  %c| | \n"
        " |_|    |_| \n"
        "   |    |   ",
        "            \n"
        "    ____    \n"
        " n |%c  %c| n \n"
        " |_|    |_| \n"
        "   |    |   ",
        " n        n \n"
        " |  ____  | \n"
        " | |%c  %c| | \n"
        " |_|    |_| \n"
        "   |    |   ",
        2
    },
    // ROBOT (2 eyes)
    {
        "            \n"
        "   .[||].   \n"
        "  [ %c  %c ]  \n"
        "  [ ==== ]  \n"
        "  `------´  ",
        "            \n"
        "   .[||].   \n"
        "  [ %c  %c ]  \n"
        "  [ -==- ]  \n"
        "  `------´  ",
        "     *      \n"
        "   .[||].   \n"
        "  [ %c  %c ]  \n"
        "  [ ==== ]  \n"
        "  `------´  ",
        2
    },
    // RABBIT (2 eyes)
    {
        "            \n"
        "   (\\__/)   \n"
        "  ( %c  %c )  \n"
        " =(  ..  )= \n"
        "  (\")__(\")  ",
        "            \n"
        "   (|__/)   \n"
        "  ( %c  %c )  \n"
        " =(  ..  )= \n"
        "  (\")__(\")  ",
        "            \n"
        "   (\\__/)   \n"
        "  ( %c  %c )  \n"
        " =( .  . )= \n"
        "  (\")__(\")  ",
        2
    },
    // MUSHROOM (2 eyes)
    {
        "            \n"
        " .-o-OO-o-. \n"
        "(__________)\n"
        "   |%c  %c|   \n"
        "   |____|   ",
        "            \n"
        " .-O-oo-O-. \n"
        "(__________)\n"
        "   |%c  %c|   \n"
        "   |____|   ",
        "   . o  .   \n"
        " .-o-OO-o-. \n"
        "(__________)\n"
        "   |%c  %c|   \n"
        "   |____|   ",
        2
    },
    // CHONK (2 eyes)
    {
        "            \n"
        "  /\    /\  \n"
        " ( %c    %c ) \n"
        " (   ..   ) \n"
        "  `------´  ",
        "            \n"
        "  /\    /|  \n"
        " ( %c    %c ) \n"
        " (   ..   ) \n"
        "  `------´  ",
        "            \n"
        "  /\    /\  \n"
        " ( %c    %c ) \n"
        " (   ..   ) \n"
        "  `------´~ ",
        2
    },
};

const char* SPECIES_NAMES[PET_SPECIES_COUNT] = {
    "Duck", "Goose", "Blob", "Cat", "Dragon",
    "Octopus", "Owl", "Penguin", "Turtle", "Snail",
    "Ghost", "Axolotl", "Capybara", "Cactus", "Robot",
    "Rabbit", "Mushroom", "Chonk"
};

const char* RARITY_NAMES[RARITY_COUNT] = {
    "Common", "Uncommon", "Rare", "Epic", "Legendary"
};

const char* RARITY_STARS[RARITY_COUNT] = {
    "*", "**", "***", "****", "*****"
};

const char* EYE_STRINGS[EYE_COUNT] = {
    "·",   // U+00B7 -> UTF-8: \xC2\xB7
    "*",   // star
    "×",   // U+00D7 -> UTF-8: \xC3\x97
    "◉",   // U+25C9 -> UTF-8: \xE2\x97\x89
    "@",   // ASCII
    "°",   // U+00B0 -> UTF-8: \xC2\xB0
};

const char* HAT_NAMES[HAT_COUNT] = {
    "None", "Crown", "Top Hat", "Propeller", "Halo", "Wizard", "Beanie", "Tiny Duck"
};

const char* HAT_LINES[HAT_COUNT] = {
    "",
    "   \\^^^/    ",
    "   [___]    ",
    "    -+-     ",
    "   (   )    ",
    "    /^\\     ",
    "   (___)    ",
    "    ,>      ",
};

const char* COLOR_NAMES[COLOR_COUNT] = {
    "Cyan", "Pink", "Gold", "Purple", "Green", "Blue", "Orange", "White"
};

const uint32_t COLOR_HEX[COLOR_COUNT] = {
    0x3DD9D0, 0xFF6B9D, 0xFFD166, 0xC85EFF, 0x5EE7A0, 0x6B8CFF, 0xFF9F43, 0xE8E8ED
};

const char* STAT_LABELS[PET_STAT_COUNT] = {
    "DEBUG", "PATIENCE", "CHAOS", "WISDOM", "SNARK"
};

// Name pools: 8 names per species
static const char* NAME_POOL_DUCK[]      = {"Quackers", "Daffy", "Waddles", "Webster", "Puddles", "Ducky", "Bill", "Howard"};
static const char* NAME_POOL_GOOSE[]     = {"Honk", "Goose", "Ryan", "Maverick", "Gander", "Feathers", "Webby", "Silly"};
static const char* NAME_POOL_BLOB[]      = {"Gloop", "Bloop", "Squish", "Gelatin", "Ooze", "Slime", "Pudding", "Jelly"};
static const char* NAME_POOL_CAT[]       = {"Whiskers", "Mittens", "Luna", "Simba", "Nala", "Oliver", "Milo", "Kitty"};
static const char* NAME_POOL_DRAGON[]    = {"Smaug", "Toothless", "Spyro", "Drogon", "Mushu", "Falkor", "Norbert", "Draco"};
static const char* NAME_POOL_OCTOPUS[]   = {"Inky", "Squidward", "Octo", "Kraken", "Tentacle", "Otto", "Eight", "Sucker"};
static const char* NAME_POOL_OWL[]       = {"Hoot", "Hedwig", "Owlbert", "Archimedes", "Errol", "Pigwidgeon", "Whoo", "Bubo"};
static const char* NAME_POOL_PENGUIN[]   = {"Pingu", "Skipper", "Waddles", "Chilly", "Frosty", "Iceberg", "Tux", "Pebble"};
static const char* NAME_POOL_TURTLE[]    = {"Squirt", "Crush", "Shelly", "Franklin", "Yertle", "Donatello", "Leonardo", "Raphael"};
static const char* NAME_POOL_SNAIL[]     = {"Gary", "Turbo", "Shellby", "Escargot", "Slowpoke", "Slimer", "Gastropod", "Trail"};
static const char* NAME_POOL_GHOST[]     = {"Boo", "Casper", "Phantom", "Specter", "Wisp", "Shade", "Spirit", "Echo"};
static const char* NAME_POOL_AXOLOTL[]   = {"Wooper", "Lotl", "Gills", "Mexican", "Pinkie", "Axel", "Woop", "Salamander"};
static const char* NAME_POOL_CAPYBARA[]  = {"Capy", "Rodney", "Hydro", "Barry", "Pablo", "Coconut", "Churro", "Waffles"};
static const char* NAME_POOL_CACTUS[]    = {"Spike", "Prickles", "Cactuar", "Saguaro", "Needles", "Poky", "Thorny", "Desert"};
static const char* NAME_POOL_ROBOT[]     = {"Byte", "Pixel", "Spark", "Bolt", "Chip", "Turing", "Ada", "Unit-01"};
static const char* NAME_POOL_RABBIT[]    = {"Bugs", "Thumper", "Bunny", "Clover", "Hops", "Cotton", "Fluffy", "Peter"};
static const char* NAME_POOL_MUSHROOM[]  = {"Shroom", "Toad", "Fungus", "Morel", "Portobello", "Chanterelle", "Truffle", "Puff"};
static const char* NAME_POOL_CHONK[]     = {"Chonky", "Biggie", "Chunk", "Thicc", "Round", "Orb", "Dumpling", "Boulder"};

const char** NAME_POOLS[PET_SPECIES_COUNT] = {
    NAME_POOL_DUCK,
    NAME_POOL_GOOSE,
    NAME_POOL_BLOB,
    NAME_POOL_CAT,
    NAME_POOL_DRAGON,
    NAME_POOL_OCTOPUS,
    NAME_POOL_OWL,
    NAME_POOL_PENGUIN,
    NAME_POOL_TURTLE,
    NAME_POOL_SNAIL,
    NAME_POOL_GHOST,
    NAME_POOL_AXOLOTL,
    NAME_POOL_CAPYBARA,
    NAME_POOL_CACTUS,
    NAME_POOL_ROBOT,
    NAME_POOL_RABBIT,
    NAME_POOL_MUSHROOM,
    NAME_POOL_CHONK,
};

const int NAME_POOL_SIZES[PET_SPECIES_COUNT] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
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
    g_pet.species = (PetSpecies)pet_rng_range(PET_SPECIES_COUNT);
    pet_reset_stats();

    // Name from pool
    int pool_size = NAME_POOL_SIZES[g_pet.species];
    const char* chosen = NAME_POOLS[g_pet.species][pet_rng_range(pool_size)];
    strncpy(g_pet.name, chosen, sizeof(g_pet.name) - 1);
    g_pet.name[sizeof(g_pet.name) - 1] = '\0';
}

void pet_reset_stats(void)
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

    g_pet.eye = (PetEye)pet_rng_range(EYE_COUNT);
    g_pet.hat = (PetHat)pet_rng_range(HAT_COUNT);
    g_pet.color = (PetColor)pet_rng_range(COLOR_COUNT);
    g_pet.shiny = (pet_rng_range(100) < 5) ? 1 : 0;
    g_pet.hunger = 80;
    g_pet.joy = 70;

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
}
