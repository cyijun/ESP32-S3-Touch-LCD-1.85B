# Buddy 桌宠 Redesign 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 myprojects/vibemate 现有桌宠全面移植对齐 buddy_proto_new 新设计，包括 18 物种、帽子/颜色系统、饱食度/开心值养成机制、NVS 持久化、完整动画特效。

**Architecture:** 扩展现有 LVGL + Arduino 架构，保持 5 页 tileview 导航。数据层扩展 PetData 结构并新增 NVS 持久化模块。UI 层重写 Pet Detail 和 Pet 页，新增 Pet Select 页。动画系统基于 lv_timer 实现 CSS 动画的等效效果。

**Tech Stack:** ESP32-S3, Arduino Framework, LVGL 8.x, ESP32 Preferences (NVS)

---

## 文件结构

| 文件 | 操作 | 说明 |
|------|------|------|
| `pet_sprites.h` | 重写 | 扩展枚举（18 物种/8 帽子/8 颜色）、PetData 结构 |
| `pet_sprites.cpp` | 重写 | 18 物种精灵模板（2 帧×4 行）、帽子线、颜色表、名字池、随机生成 |
| `pet_storage.h` | 新增 | NVS 持久化接口 |
| `pet_storage.cpp` | 新增 | Preferences 读写实现 |
| `ui_pet_detail.h` | 修改 | 更新接口声明 |
| `ui_pet_detail.cpp` | 重写 | 属性面板：头像区 + 弧形进度条 + 元数据网格 |
| `ui_pet.h` | 修改 | 更新接口声明（添加删除函数） |
| `ui_pet.cpp` | 重写 | 交互主屏：mini stats + 精灵舞台 + 动作按钮 + 长按菜单 + 特效 |
| `ui_pet_select.h` | 新增 | 伙伴选择页接口 |
| `ui_pet_select.cpp` | 新增 | 物种轮播 + 选择器 + 缩略图条 |
| `vibemate.ino` | 修改 | 5 页 tileview、启动加载、状态衰减定时器 |

---

## Task 1: 扩展数据模型 (pet_sprites)

**Files:**
- Rewrite: `myprojects/vibemate/pet_sprites.h`
- Rewrite: `myprojects/vibemate/pet_sprites.cpp`

- [ ] **Step 1: 重写 pet_sprites.h**

  替换整个文件：

  ```c
  #ifndef PET_SPRITES_H
  #define PET_SPRITES_H

  #include <stdint.h>

  #ifdef __cplusplus
  extern "C" {
  #endif

  typedef enum {
      PET_DUCK = 0, PET_GOOSE, PET_BLOB, PET_CAT, PET_DRAGON,
      PET_OCTOPUS, PET_OWL, PET_PENGUIN, PET_TURTLE, PET_SNAIL,
      PET_GHOST, PET_AXOLOTL, PET_CAPYBARA, PET_CACTUS, PET_ROBOT,
      PET_RABBIT, PET_MUSHROOM, PET_CHONK,
      PET_SPECIES_COUNT
  } PetSpecies;

  typedef enum {
      EYE_DOT = 0,    // '·'
      EYE_STAR,       // '✦'
      EYE_X,          // '×'
      EYE_CIRCLE,     // '◉'
      EYE_AT,         // '@'
      EYE_DEGREE,     // '°'
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
      COLOR_CYAN = 0,
      COLOR_PINK,
      COLOR_GOLD,
      COLOR_PURPLE,
      COLOR_GREEN,
      COLOR_BLUE,
      COLOR_ORANGE,
      COLOR_WHITE,
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
      const char* frame0;
      const char* frame1;
      uint8_t eye_count; // 1 or 2
  } SpriteTemplate;

  typedef struct {
      PetSpecies species;
      PetEye eye;
      PetHat hat;
      PetColor color;
      PetRarity rarity;
      bool shiny;
      uint8_t stats[PET_STAT_COUNT];
      uint8_t hunger;
      uint8_t joy;
      char name[16];
  } PetData;

  extern const SpriteTemplate SPECIES_TEMPLATES[PET_SPECIES_COUNT];
  extern const char* SPECIES_NAMES[PET_SPECIES_COUNT];
  extern const char* RARITY_NAMES[RARITY_COUNT];
  extern const char* RARITY_STARS[RARITY_COUNT];
  extern const char EYE_CHARS[EYE_COUNT];
  extern const char* HAT_LINES[HAT_COUNT];
  extern const uint32_t COLOR_HEX[COLOR_COUNT];
  extern const char* COLOR_NAMES[COLOR_COUNT];
  extern const char* STAT_LABELS[PET_STAT_COUNT];
  extern const char** NAME_POOLS[PET_SPECIES_COUNT];
  extern const int NAME_POOL_SIZES[PET_SPECIES_COUNT];

  extern PetData g_pet;
  void pet_generate(void);
  uint32_t pet_rng_next(void);
  uint32_t pet_rng_range(uint32_t max);
  void pet_reset_stats(void);

  #ifdef __cplusplus
  }
  #endif

  #endif
  ```

- [ ] **Step 2: 重写 pet_sprites.cpp**

  替换整个文件：

  ```cpp
  #include "pet_sprites.h"
  #include <Esp.h>
  #include <string.h>

  // clang-format off

  const SpriteTemplate SPECIES_TEMPLATES[PET_SPECIES_COUNT] = {
      // PET_DUCK
      {
          "    __     \n  <(%c )___ \n   (  ._>   \n    `--´    ",
          "    __     \n  <(%c )___ \n   (  ._>   \n    `--´~   ",
          1
      },
      // PET_GOOSE
      {
          "    __     \n  <(%c )___ \n   ( .__>   \n    `--´    ",
          "    __     \n  <(%c )___ \n   ( .__>   \n    `--´~   ",
          1
      },
      // PET_BLOB
      {
          "   .----.  \n  ( %c   %c )\n  (      )  \n   `----´   ",
          "  .------. \n (  %c   %c  )\n (        ) \n  `------´  ",
          2
      },
      // PET_CAT
      {
          "   /\\_/\\   \n  ( %c   %c ) \n  (  ω  )   \n  (")_(")   ",
          "   /\\_/\\   \n  ( %c   %c ) \n  (  ω  )   \n  (")_(")~  ",
          2
      },
      // PET_DRAGON
      {
          "  /^\\  /^\\  \n <  %c   %c  > \n (   ~~   ) \n  `-vvvv-´  ",
          "  /^\\  /^\\  \n <  %c   %c  > \n (        ) \n  `-vvvv-´  ",
          2
      },
      // PET_OCTOPUS
      {
          "  ~  ~  ~  \n <( %c   %c )>\n  (      )  \n  /|    |\\  ",
          "   ~   ~   \n <( %c   %c )>\n  (      )  \n  /|    |\\  ",
          2
      },
      // PET_OWL
      {
          "   .---.   \n  / %c   %c \\  \n  |  ω  |   \n   `---´    ",
          "   .---.   \n  / %c   %c \\  \n  |  -  |   \n   `---´    ",
          2
      },
      // PET_PENGUIN
      {
          "    .-.    \n   >(%c )   \n   (    )   \n    `-´     ",
          "    .-.    \n   >(%c )   \n   (    )   \n    `-´~    ",
          1
      },
      // PET_TURTLE
      {
          "   .---.   \n  / %c   %c \\  \n  | ___ |   \n   `---´    ",
          "   .---.   \n  / %c   %c \\  \n  | === |   \n   `---´    ",
          2
      },
      // PET_SNAIL
      {
          "    @ @    \n   /%c   %c\\  \n   ( ___ )  \n    `---´    ",
          "    @ @    \n   /%c   %c\\  \n   ( ___ )  \n    `---´~   ",
          2
      },
      // PET_GHOST
      {
          "   .----.  \n  / %c   %c \\  \n  |      |  \n  ~`~``~`~  ",
          "   .----.  \n  / %c   %c \\  \n  |      |  \n  `~`~~`~`  ",
          2
      },
      // PET_AXOLOTL
      {
          "   }···{   \n  <( %c   %c )>\n  (      )  \n   `~~~~´   ",
          "   }···{   \n  <( %c   %c )>\n  (      )  \n   `~~~~´~  ",
          2
      },
      // PET_CAPYBARA
      {
          "  .-----.  \n ( %c   %c ) \n (  ===  )  \n  `-----´   ",
          "  .-----.  \n ( %c   %c ) \n (  ---  )  \n  `-----´   ",
          2
      },
      // PET_CACTUS
      {
          "    | |    \n   /%c   %c\\  \n   |  ω  |  \n    \\___/   ",
          "    | |    \n   /%c   %c\\  \n   |  -  |  \n    \\___/   ",
          2
      },
      // PET_ROBOT
      {
          "   .[||].  \n  [ %c   %c ] \n  [ ==== ]  \n  `------´  ",
          "   .[||].  \n  [ %c   %c ] \n  [ -==- ]  \n  `------´  ",
          2
      },
      // PET_RABBIT
      {
          "   (\\ /)  \n   (%c %c)   \n   (  ω  )  \n    ()()    ",
          "   (\\ /)  \n   (%c %c)   \n   (  -  )  \n    ()()~   ",
          2
      },
      // PET_MUSHROOM
      {
          "    ___    \n   /%c   %c\\  \n  (   ω   ) \n   `-----´  ",
          "    ___    \n   /%c   %c\\  \n  (   -   ) \n   `-----´  ",
          2
      },
      // PET_CHONK
      {
          "  .------. \n ( %c    %c )\n (   ==   ) \n  `------´  ",
          "  .------. \n ( %c    %c )\n (   --   ) \n  `------´  ",
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
      "★", "★★", "★★★", "★★★★", "★★★★★"
  };

  const char EYE_CHARS[EYE_COUNT] = {
      '·', '✦', '×', '◉', '@', '°'
  };

  const char* HAT_LINES[HAT_COUNT] = {
      "",
      "   \\^^^/   ",
      "   [___]    ",
      "    -+-     ",
      "   (   )    ",
      "    /^\\     ",
      "   (___)    ",
      "    ,>      ",
  };

  const uint32_t COLOR_HEX[COLOR_COUNT] = {
      0x3DD9D0, // cyan
      0xFF6B9D, // pink
      0xFFD166, // gold
      0xC85EFF, // purple
      0x5EE7A0, // green
      0x6B8CFF, // blue
      0xFF9F43, // orange
      0xE8E8ED, // white
  };

  const char* COLOR_NAMES[COLOR_COUNT] = {
      "Cyan", "Pink", "Gold", "Purple", "Green", "Blue", "Orange", "White"
  };

  const char* STAT_LABELS[PET_STAT_COUNT] = {
      "DEBUGGING", "PATIENCE", "CHAOS", "WISDOM", "SNARK"
  };

  // Name pools
  static const char* NAME_POOL_DUCK[]     = {"Quackers", "Daffy", "Waddles", "Webster", "Puddles", "Ducky", "Bill", "Howard"};
  static const char* NAME_POOL_GOOSE[]    = {"Goose", "Honk", "Feathers", "Wing", "Webby", "Silly", "Grace", "Nile"};
  static const char* NAME_POOL_BLOB[]     = {"Gloop", "Bloop", "Squish", "Gelatin", "Ooze", "Slime", "Pudding", "Jelly"};
  static const char* NAME_POOL_CAT[]      = {"Whiskers", "Mittens", "Luna", "Simba", "Nala", "Oliver", "Milo", "Kitty"};
  static const char* NAME_POOL_DRAGON[]   = {"Draco", "Smaug", "Toothless", "Spyro", "Drogon", "Charizard", "Fafnir", "Puff"};
  static const char* NAME_POOL_OCTOPUS[]  = {"Tentacle", "Inky", "Squid", "Kraken", "Octy", "Eight", "Sucker", "Cephal"};
  static const char* NAME_POOL_OWL[]      = {"Hoot", "Owlbert", "Hedwig", "Athena", "Noctis", "Whoo", "Archie", "Merlin"};
  static const char* NAME_POOL_PENGUIN[]  = {"Skipper", "Waddles", "Chilly", "Ice", "Pingu", "Tux", "Frosty", "Pepper"};
  static const char* NAME_POOL_TURTLE[]   = {"Shelly", "Crush", "Squirt", "Tank", "Torty", "Slowmo", "Leonardo", "Koopa"};
  static const char* NAME_POOL_SNAIL[]    = {"Gary", "Shellby", "Slow", "Slimey", "Escargot", "Trail", "Turbo", "Snaily"};
  static const char* NAME_POOL_GHOST[]    = {"Boo", "Casper", "Phantom", "Specter", "Wisp", "Shade", "Spirit", "Echo"};
  static const char* NAME_POOL_AXOLOTL[]  = {"Axo", "Wooper", "Gills", "Mexi", "Pinkie", "Frill", "Sal", "Lotl"};
  static const char* NAME_POOL_CAPYBARA[] = {"Capo", "Barry", "Rodent", "Hydro", "Chiguiro", "Coco", "Paca", "Bara"};
  static const char* NAME_POOL_CACTUS[]   = {"Spike", "Prickles", "Cacti", "Needle", "Saguaro", "Poky", "Desert", "Greenie"};
  static const char* NAME_POOL_ROBOT[]    = {"Byte", "Pixel", "Spark", "Bolt", "Chip", "Turing", "Ada", "Unit-01"};
  static const char* NAME_POOL_RABBIT[]   = {"Bunny", "Thumper", "Cotton", "Hops", "Carrot", "Fluffy", "Bugs", "Oreo"};
  static const char* NAME_POOL_MUSHROOM[] = {"Shroom", "Fungi", "Toad", "Portobello", "Spore", "Chanterelle", "Morel", "Puffball"};
  static const char* NAME_POOL_CHONK[]    = {"Chonk", "Chunk", "Round", "Fluff", "Chubby", "Plump", "Thicc", "Orb"};

  const char** NAME_POOLS[PET_SPECIES_COUNT] = {
      NAME_POOL_DUCK, NAME_POOL_GOOSE, NAME_POOL_BLOB, NAME_POOL_CAT, NAME_POOL_DRAGON,
      NAME_POOL_OCTOPUS, NAME_POOL_OWL, NAME_POOL_PENGUIN, NAME_POOL_TURTLE, NAME_POOL_SNAIL,
      NAME_POOL_GHOST, NAME_POOL_AXOLOTL, NAME_POOL_CAPYBARA, NAME_POOL_CACTUS, NAME_POOL_ROBOT,
      NAME_POOL_RABBIT, NAME_POOL_MUSHROOM, NAME_POOL_CHONK,
  };

  const int NAME_POOL_SIZES[PET_SPECIES_COUNT] = {
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
  };

  // ========== RNG ==========
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

  // ========== Generation ==========
  PetData g_pet;

  void pet_reset_stats(void)
  {
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
      g_pet.shiny = pet_rng_range(100) < 5;

      // Stats
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

      // Name
      int pool_size = NAME_POOL_SIZES[g_pet.species];
      const char* chosen = NAME_POOLS[g_pet.species][pet_rng_range(pool_size)];
      strncpy(g_pet.name, chosen, sizeof(g_pet.name) - 1);
      g_pet.name[sizeof(g_pet.name) - 1] = '\0';
  }

  void pet_generate(void)
  {
      uint64_t mac = ESP.getEfuseMac();
      s_rng_state = (uint32_t)(mac ^ (mac >> 32));

      g_pet.species = (PetSpecies)pet_rng_range(PET_SPECIES_COUNT);
      pet_reset_stats();

      g_pet.hunger = 80;
      g_pet.joy = 70;
  }

  // clang-format on
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

  Expected: 编译通过（可能有其他文件的未定义引用，但 pet_sprites 本身应无错误）

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/pet_sprites.h myprojects/vibemate/pet_sprites.cpp
  git commit -m "feat(pet): extend data model with 18 species, hats, colors, hunger/joy"
  ```

---

## Task 2: NVS 持久化 (pet_storage)

**Files:**
- Create: `myprojects/vibemate/pet_storage.h`
- Create: `myprojects/vibemate/pet_storage.cpp`

- [ ] **Step 1: 创建 pet_storage.h**

  ```c
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
  ```

- [ ] **Step 2: 创建 pet_storage.cpp**

  ```cpp
  #include "pet_storage.h"
  #include "pet_sprites.h"
  #include <Preferences.h>

  static const char* PREFS_NS = "buddy";

  bool pet_has_saved(void)
  {
      Preferences prefs;
      if (!prefs.begin(PREFS_NS, true)) return false;
      bool has = prefs.isKey("species");
      prefs.end();
      return has;
  }

  bool pet_load(void)
  {
      Preferences prefs;
      if (!prefs.begin(PREFS_NS, true)) return false;

      if (!prefs.isKey("species")) {
          prefs.end();
          return false;
      }

      g_pet.species = (PetSpecies)prefs.getUChar("species", 0);
      g_pet.eye = (PetEye)prefs.getUChar("eye", 0);
      g_pet.hat = (PetHat)prefs.getUChar("hat", 0);
      g_pet.color = (PetColor)prefs.getUChar("color", 0);
      g_pet.rarity = (PetRarity)prefs.getUChar("rarity", 0);
      g_pet.shiny = prefs.getBool("shiny", false);
      g_pet.hunger = prefs.getUChar("hunger", 80);
      g_pet.joy = prefs.getUChar("joy", 70);

      for (int i = 0; i < PET_STAT_COUNT; i++) {
          char key[8];
          snprintf(key, sizeof(key), "stat%d", i);
          g_pet.stats[i] = prefs.getUChar(key, 50);
      }

      String name = prefs.getString("name", "Buddy");
      strncpy(g_pet.name, name.c_str(), sizeof(g_pet.name) - 1);
      g_pet.name[sizeof(g_pet.name) - 1] = '\0';

      prefs.end();
      return true;
  }

  void pet_save(void)
  {
      Preferences prefs;
      if (!prefs.begin(PREFS_NS, false)) return;

      prefs.putUChar("species", g_pet.species);
      prefs.putUChar("eye", g_pet.eye);
      prefs.putUChar("hat", g_pet.hat);
      prefs.putUChar("color", g_pet.color);
      prefs.putUChar("rarity", g_pet.rarity);
      prefs.putBool("shiny", g_pet.shiny);
      prefs.putUChar("hunger", g_pet.hunger);
      prefs.putUChar("joy", g_pet.joy);

      for (int i = 0; i < PET_STAT_COUNT; i++) {
          char key[8];
          snprintf(key, sizeof(key), "stat%d", i);
          prefs.putUChar(key, g_pet.stats[i]);
      }

      prefs.putString("name", g_pet.name);
      prefs.end();
  }
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

  Expected: 编译通过

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/pet_storage.h myprojects/vibemate/pet_storage.cpp
  git commit -m "feat(pet): add NVS persistence for all pet state"
  ```

---

## Task 3: 重写属性面板 (ui_pet_detail)

**Files:**
- Modify: `myprojects/vibemate/ui_pet_detail.h`
- Rewrite: `myprojects/vibemate/ui_pet_detail.cpp`

- [ ] **Step 1: 更新 ui_pet_detail.h**

  ```c
  #ifndef UI_PET_DETAIL_H
  #define UI_PET_DETAIL_H

  #include <lvgl.h>

  void ui_pet_detail_create(lv_obj_t *parent_tile);
  void ui_pet_detail_update(void);

  #endif
  ```

- [ ] **Step 2: 重写 ui_pet_detail.cpp**

  ```cpp
  #include "ui_pet_detail.h"
  #include "pet_sprites.h"
  #include <stdio.h>
  #include <string.h>

  static lv_obj_t *s_label_name;
  static lv_obj_t *s_badge_rarity;
  static lv_obj_t *s_avatar_face;
  static lv_obj_t *s_arc_stats[PET_STAT_COUNT];
  static lv_obj_t *s_bar_stats[PET_STAT_COUNT];
  static lv_obj_t *s_label_stat_names[PET_STAT_COUNT];
  static lv_obj_t *s_label_stat_vals[PET_STAT_COUNT];
  static lv_obj_t *s_meta_species;
  static lv_obj_t *s_meta_eye;
  static lv_obj_t *s_meta_hat;
  static lv_obj_t *s_meta_shiny;

  static uint32_t s_rarity_color(PetRarity r)
  {
      switch (r) {
          case RARITY_COMMON:    return 0x8A8A95;
          case RARITY_UNCOMMON:  return 0x5EE7DF;
          case RARITY_RARE:      return 0x6B8CFF;
          case RARITY_EPIC:      return 0xC85EFF;
          case RARITY_LEGENDARY: return 0xFFD166;
          default: return 0x8A8A95;
      }
  }

  static void s_make_arc(lv_obj_t *parent, int value, lv_obj_t **out_arc, lv_obj_t **out_label)
  {
      lv_obj_t *cont = lv_obj_create(parent);
      lv_obj_set_size(cont, 26, 26);
      lv_obj_set_style_pad_all(cont, 0, 0);
      lv_obj_set_style_border_width(cont, 0, 0);
      lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);

      lv_obj_t *arc = lv_arc_create(cont);
      lv_obj_set_size(arc, 24, 24);
      lv_arc_set_rotation(arc, 270);
      lv_arc_set_bg_angles(arc, 0, 360);
      lv_arc_set_value(arc, 0);
      lv_arc_set_range(arc, 0, 100);
      lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN);
      lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
      lv_obj_set_style_arc_color(arc, lv_color_hex(0x252530), LV_PART_MAIN);
      lv_obj_set_style_arc_color(arc, lv_color_hex(0x3DD9D0), LV_PART_INDICATOR);
      lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
      lv_obj_center(arc);

      lv_obj_t *label = lv_label_create(cont);
      lv_label_set_text(label, "0");
      lv_obj_set_style_text_font(label, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(label, lv_color_hex(0xE8E8ED), 0);
      lv_obj_center(label);

      *out_arc = arc;
      *out_label = label;
  }

  void ui_pet_detail_create(lv_obj_t *parent_tile)
  {
      lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_pad_all(parent_tile, 0, 0);
      lv_obj_set_style_border_width(parent_tile, 0, 0);

      // --- Header ---
      lv_obj_t *avatar_ring = lv_obj_create(parent_tile);
      lv_obj_set_size(avatar_ring, 42, 42);
      lv_obj_set_style_radius(avatar_ring, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(avatar_ring, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_border_color(avatar_ring, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(avatar_ring, 2, 0);
      lv_obj_set_style_pad_all(avatar_ring, 0, 0);
      lv_obj_align(avatar_ring, LV_ALIGN_TOP_MID, 0, 14);

      s_avatar_face = lv_label_create(avatar_ring);
      lv_label_set_text(s_avatar_face, "=✦ω✦=");
      lv_obj_set_style_text_font(s_avatar_face, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_avatar_face, lv_color_hex(0x3DD9D0), 0);
      lv_obj_center(s_avatar_face);

      s_label_name = lv_label_create(parent_tile);
      lv_label_set_text(s_label_name, g_pet.name);
      lv_obj_set_style_text_font(s_label_name, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(s_label_name, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(s_label_name, LV_ALIGN_TOP_MID, 0, 60);

      s_badge_rarity = lv_obj_create(parent_tile);
      lv_obj_set_size(s_badge_rarity, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_obj_set_style_pad_hor(s_badge_rarity, 8, 0);
      lv_obj_set_style_pad_ver(s_badge_rarity, 2, 0);
      lv_obj_set_style_radius(s_badge_rarity, 8, 0);
      lv_obj_set_style_bg_color(s_badge_rarity, lv_color_hex(0x3DD9D0), 0);
      lv_obj_set_style_bg_opa(s_badge_rarity, LV_OPA_20, 0);
      lv_obj_set_style_border_width(s_badge_rarity, 0, 0);
      lv_obj_align(s_badge_rarity, LV_ALIGN_TOP_MID, 0, 78);

      lv_obj_t *badge_label = lv_label_create(s_badge_rarity);
      lv_label_set_text(badge_label, RARITY_NAMES[g_pet.rarity]);
      lv_obj_set_style_text_font(badge_label, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(badge_label, lv_color_hex(s_rarity_color(g_pet.rarity)), 0);
      lv_obj_center(badge_label);

      // --- Stats ---
      int y = 108;
      for (int i = 0; i < PET_STAT_COUNT; i++) {
          lv_obj_t *row = lv_obj_create(parent_tile);
          lv_obj_set_size(row, 260, 30);
          lv_obj_set_style_bg_opa(row, LV_OPA_0, 0);
          lv_obj_set_style_border_width(row, 0, 0);
          lv_obj_set_style_pad_all(row, 0, 0);
          lv_obj_align(row, LV_ALIGN_TOP_MID, 0, y + i * 30);

          // Arc
          lv_obj_t *arc_cont = lv_obj_create(row);
          lv_obj_set_size(arc_cont, 24, 24);
          lv_obj_set_style_pad_all(arc_cont, 0, 0);
          lv_obj_set_style_border_width(arc_cont, 0, 0);
          lv_obj_set_style_bg_opa(arc_cont, LV_OPA_0, 0);
          lv_obj_align(arc_cont, LV_ALIGN_LEFT_MID, 0, 0);

          lv_obj_t *arc = lv_arc_create(arc_cont);
          lv_obj_set_size(arc, 22, 22);
          lv_arc_set_rotation(arc, 270);
          lv_arc_set_bg_angles(arc, 0, 360);
          lv_arc_set_range(arc, 0, 100);
          lv_arc_set_value(arc, g_pet.stats[i]);
          lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN);
          lv_obj_set_style_arc_width(arc, 2, LV_PART_INDICATOR);
          lv_obj_set_style_arc_color(arc, lv_color_hex(0x252530), LV_PART_MAIN);
          lv_obj_set_style_arc_color(arc, lv_color_hex(0x3DD9D0), LV_PART_INDICATOR);
          lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
          lv_obj_center(arc);
          s_arc_stats[i] = arc;

          lv_obj_t *arc_val = lv_label_create(arc_cont);
          lv_label_set_text_fmt(arc_val, "%d", g_pet.stats[i]);
          lv_obj_set_style_text_font(arc_val, &lv_font_montserrat_8, 0);
          lv_obj_set_style_text_color(arc_val, lv_color_hex(0xE8E8ED), 0);
          lv_obj_center(arc_val);
          s_label_stat_vals[i] = arc_val;

          // Info
          lv_obj_t *info = lv_obj_create(row);
          lv_obj_set_size(info, 220, 26);
          lv_obj_set_style_bg_opa(info, LV_OPA_0, 0);
          lv_obj_set_style_border_width(info, 0, 0);
          lv_obj_set_style_pad_all(info, 0, 0);
          lv_obj_align(info, LV_ALIGN_RIGHT_MID, 0, 0);

          lv_obj_t *name_label = lv_label_create(info);
          lv_label_set_text(name_label, STAT_LABELS[i]);
          lv_obj_set_style_text_font(name_label, &lv_font_montserrat_8, 0);
          lv_obj_set_style_text_color(name_label, lv_color_hex(0x6B6B78), 0);
          lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, 0, 0);

          lv_obj_t *num_label = lv_label_create(info);
          lv_label_set_text_fmt(num_label, "%d", g_pet.stats[i]);
          lv_obj_set_style_text_font(num_label, &lv_font_montserrat_10, 0);
          lv_obj_set_style_text_color(num_label, lv_color_hex(0xE8E8ED), 0);
          lv_obj_align(num_label, LV_ALIGN_TOP_RIGHT, 0, 0);

          lv_obj_t *bar = lv_bar_create(info);
          lv_obj_set_size(bar, 220, 2);
          lv_bar_set_range(bar, 0, 100);
          lv_bar_set_value(bar, g_pet.stats[i], LV_ANIM_ON);
          lv_obj_align(bar, LV_ALIGN_BOTTOM_LEFT, 0, 0);
          lv_obj_set_style_bg_color(bar, lv_color_hex(0x252530), LV_PART_MAIN);
          lv_obj_set_style_bg_color(bar, lv_color_hex(0x3DD9D0), LV_PART_INDICATOR);
          lv_obj_set_style_radius(bar, 1, LV_PART_MAIN);
          lv_obj_set_style_radius(bar, 1, LV_PART_INDICATOR);
          s_bar_stats[i] = bar;
      }

      // --- Meta grid ---
      const char* meta_labels[4] = {"SPECIES", "EYE", "HAT", "SHINY"};
      const char* meta_values[4] = {
          SPECIES_NAMES[g_pet.species],
          "✦", // will update in update()
          HAT_LINES[g_pet.hat],
          g_pet.shiny ? "Yes" : "No"
      };

      int meta_y = 260;
      for (int i = 0; i < 4; i++) {
          lv_obj_t *card = lv_obj_create(parent_tile);
          lv_obj_set_size(card, 120, 36);
          lv_obj_set_style_bg_color(card, lv_color_hex(0x13131A), 0);
          lv_obj_set_style_border_color(card, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(card, 1, 0);
          lv_obj_set_style_radius(card, 6, 0);
          lv_obj_set_style_pad_all(card, 4, 0);
          lv_obj_align(card, LV_ALIGN_TOP_MID, (i % 2 == 0) ? -64 : 64, meta_y + (i / 2) * 44);

          lv_obj_t *lbl = lv_label_create(card);
          lv_label_set_text(lbl, meta_labels[i]);
          lv_obj_set_style_text_font(lbl, &lv_font_montserrat_8, 0);
          lv_obj_set_style_text_color(lbl, lv_color_hex(0x6B6B78), 0);
          lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0, 0);

          lv_obj_t *val = lv_label_create(card);
          lv_label_set_text(val, meta_values[i]);
          lv_obj_set_style_text_font(val, &lv_font_montserrat_10, 0);
          lv_obj_set_style_text_color(val, lv_color_hex(0xE8E8ED), 0);
          lv_obj_align(val, LV_ALIGN_BOTTOM_LEFT, 0, 0);

          if (i == 0) s_meta_species = val;
          else if (i == 1) s_meta_eye = val;
          else if (i == 2) s_meta_hat = val;
          else if (i == 3) s_meta_shiny = val;
      }

      ui_pet_detail_update();
  }

  void ui_pet_detail_update(void)
  {
      lv_label_set_text(s_label_name, g_pet.name);
      lv_label_set_text(s_avatar_face, "=✦ω✦=");

      for (int i = 0; i < PET_STAT_COUNT; i++) {
          lv_arc_set_value(s_arc_stats[i], g_pet.stats[i]);
          lv_label_set_text_fmt(s_label_stat_vals[i], "%d", g_pet.stats[i]);
          lv_bar_set_value(s_bar_stats[i], g_pet.stats[i], LV_ANIM_ON);
      }

      // Update meta values
      static char eye_buf[8];
      snprintf(eye_buf, sizeof(eye_buf), "%c", EYE_CHARS[g_pet.eye]);
      lv_label_set_text(s_meta_eye, eye_buf);
      lv_label_set_text(s_meta_species, SPECIES_NAMES[g_pet.species]);
      lv_label_set_text(s_meta_hat, g_pet.hat == HAT_NONE ? "None" : "Yes");
      lv_label_set_text(s_meta_shiny, g_pet.shiny ? "Yes" : "No");
  }
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

  Expected: 编译通过

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/ui_pet_detail.h myprojects/vibemate/ui_pet_detail.cpp
  git commit -m "feat(ui): redesign pet detail page with arcs, bars, and meta grid"
  ```

---

## Task 4: 重写交互主屏 - 基础布局 (ui_pet)

**Files:**
- Modify: `myprojects/vibemate/ui_pet.h`
- Rewrite: `myprojects/vibemate/ui_pet.cpp` — 基础布局部分

- [ ] **Step 1: 更新 ui_pet.h**

  ```c
  #ifndef UI_PET_H
  #define UI_PET_H

  #include <lvgl.h>

  void ui_pet_create(lv_obj_t *parent_tile);
  void ui_pet_update(void);
  void ui_pet_delete(void);

  #endif
  ```

- [ ] **Step 2: 重写 ui_pet.cpp - 基础布局**

  这是第一阶段，先搭建静态布局（名称、稀有度、mini stats、精灵标签、动作按钮）。动画和交互在 Task 5/6 添加。

  ```cpp
  #include "ui_pet.h"
  #include "pet_sprites.h"
  #include "pet_storage.h"
  #include <stdio.h>
  #include <string.h>

  // --- UI Objects ---
  static lv_obj_t *s_label_name;
  static lv_obj_t *s_label_rarity;
  static lv_obj_t *s_label_sprite;
  static lv_obj_t *s_bubble;
  static lv_obj_t *s_bubble_label;
  static lv_obj_t *s_hat_line;
  static lv_obj_t *s_pet_ring;

  // Mini stats
  static lv_obj_t *s_bar_hunger;
  static lv_obj_t *s_bar_joy;
  static lv_obj_t *s_val_hunger;
  static lv_obj_t *s_val_joy;

  // Action buttons
  static lv_obj_t *s_btn_feed;
  static lv_obj_t *s_btn_talk;
  static lv_obj_t *s_btn_play;

  // Hat menu
  static lv_obj_t *s_hat_menu;
  static lv_obj_t *s_hat_grid;

  // --- Helpers ---
  static uint32_t s_rarity_color(PetRarity r)
  {
      switch (r) {
          case RARITY_COMMON:    return 0x8A8A95;
          case RARITY_UNCOMMON:  return 0x5EE7DF;
          case RARITY_RARE:      return 0x6B8CFF;
          case RARITY_EPIC:      return 0xC85EFF;
          case RARITY_LEGENDARY: return 0xFFD166;
          default: return 0x8A8A95;
      }
  }

  static lv_color_t s_accent_color(void)
  {
      return lv_color_hex(COLOR_HEX[g_pet.color]);
  }

  static void s_render_sprite(void)
  {
      const SpriteTemplate *tmpl = &SPECIES_TEMPLATES[g_pet.species];
      const char *frame = (s_anim_frame % 2 == 0) ? tmpl->frame0 : tmpl->frame1;
      char eye = EYE_CHARS[g_pet.eye];

      static char buf[256];
      if (tmpl->eye_count == 2) {
          snprintf(buf, sizeof(buf), frame, eye, eye);
      } else {
          snprintf(buf, sizeof(buf), frame, eye);
      }
      lv_label_set_text(s_label_sprite, buf);

      // Hat
      if (g_pet.hat != HAT_NONE) {
          lv_label_set_text(s_hat_line, HAT_LINES[g_pet.hat]);
          lv_obj_clear_flag(s_hat_line, LV_OBJ_FLAG_HIDDEN);
      } else {
          lv_obj_add_flag(s_hat_line, LV_OBJ_FLAG_HIDDEN);
      }
  }

  static void s_update_mini_stats(void)
  {
      lv_bar_set_value(s_bar_hunger, g_pet.hunger, LV_ANIM_ON);
      lv_bar_set_value(s_bar_joy, g_pet.joy, LV_ANIM_ON);
      lv_label_set_text_fmt(s_val_hunger, "%d", g_pet.hunger);
      lv_label_set_text_fmt(s_val_joy, "%d", g_pet.joy);
  }

  // Forward declarations for event handlers (implemented in Task 6)
  static void s_on_feed(lv_event_t *e);
  static void s_on_talk(lv_event_t *e);
  static void s_on_play(lv_event_t *e);
  static void s_on_sprite_press(lv_event_t *e);
  static void s_on_hat_pick(lv_event_t *e);

  void ui_pet_create(lv_obj_t *parent_tile)
  {
      lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_pad_all(parent_tile, 0, 0);
      lv_obj_set_style_border_width(parent_tile, 0, 0);

      // --- Name ---
      s_label_name = lv_label_create(parent_tile);
      lv_label_set_text(s_label_name, g_pet.name);
      lv_obj_set_style_text_font(s_label_name, &lv_font_montserrat_14, 0);
      lv_obj_set_style_text_color(s_label_name, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(s_label_name, LV_ALIGN_TOP_MID, 0, 26);

      // --- Rarity stars ---
      s_label_rarity = lv_label_create(parent_tile);
      lv_label_set_text(s_label_rarity, RARITY_STARS[g_pet.rarity]);
      lv_obj_set_style_text_font(s_label_rarity, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_label_rarity, lv_color_hex(s_rarity_color(g_pet.rarity)), 0);
      lv_obj_align(s_label_rarity, LV_ALIGN_TOP_MID, 0, 44);

      // --- Mini stats ---
      lv_obj_t *stat_cont = lv_obj_create(parent_tile);
      lv_obj_set_size(stat_cont, 220, 28);
      lv_obj_set_style_bg_opa(stat_cont, LV_OPA_0, 0);
      lv_obj_set_style_border_width(stat_cont, 0, 0);
      lv_obj_set_style_pad_all(stat_cont, 0, 0);
      lv_obj_align(stat_cont, LV_ALIGN_TOP_MID, 0, 58);

      // Hunger
      lv_obj_t *icon_h = lv_label_create(stat_cont);
      lv_label_set_text(icon_h, "◐");
      lv_obj_set_style_text_font(icon_h, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(icon_h, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(icon_h, LV_ALIGN_LEFT_MID, 0, -8);

      s_bar_hunger = lv_bar_create(stat_cont);
      lv_obj_set_size(s_bar_hunger, 180, 3);
      lv_bar_set_range(s_bar_hunger, 0, 100);
      lv_bar_set_value(s_bar_hunger, g_pet.hunger, LV_ANIM_OFF);
      lv_obj_align(s_bar_hunger, LV_ALIGN_LEFT_MID, 18, -8);
      lv_obj_set_style_bg_color(s_bar_hunger, lv_color_hex(0x13131A), LV_PART_MAIN);
      lv_obj_set_style_bg_color(s_bar_hunger, s_accent_color(), LV_PART_INDICATOR);
      lv_obj_set_style_radius(s_bar_hunger, 2, LV_PART_MAIN);
      lv_obj_set_style_radius(s_bar_hunger, 2, LV_PART_INDICATOR);

      s_val_hunger = lv_label_create(stat_cont);
      lv_label_set_text_fmt(s_val_hunger, "%d", g_pet.hunger);
      lv_obj_set_style_text_font(s_val_hunger, &lv_font_montserrat_8, 0);
      lv_obj_set_style_text_color(s_val_hunger, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(s_val_hunger, LV_ALIGN_RIGHT_MID, 0, -8);

      // Joy
      lv_obj_t *icon_j = lv_label_create(stat_cont);
      lv_label_set_text(icon_j, "♥");
      lv_obj_set_style_text_font(icon_j, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(icon_j, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(icon_j, LV_ALIGN_LEFT_MID, 0, 6);

      s_bar_joy = lv_bar_create(stat_cont);
      lv_obj_set_size(s_bar_joy, 180, 3);
      lv_bar_set_range(s_bar_joy, 0, 100);
      lv_bar_set_value(s_bar_joy, g_pet.joy, LV_ANIM_OFF);
      lv_obj_align(s_bar_joy, LV_ALIGN_LEFT_MID, 18, 6);
      lv_obj_set_style_bg_color(s_bar_joy, lv_color_hex(0x13131A), LV_PART_MAIN);
      lv_obj_set_style_bg_color(s_bar_joy, s_accent_color(), LV_PART_INDICATOR);
      lv_obj_set_style_radius(s_bar_joy, 2, LV_PART_MAIN);
      lv_obj_set_style_radius(s_bar_joy, 2, LV_PART_INDICATOR);

      s_val_joy = lv_label_create(stat_cont);
      lv_label_set_text_fmt(s_val_joy, "%d", g_pet.joy);
      lv_obj_set_style_text_font(s_val_joy, &lv_font_montserrat_8, 0);
      lv_obj_set_style_text_color(s_val_joy, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(s_val_joy, LV_ALIGN_RIGHT_MID, 0, 6);

      // --- Speech bubble ---
      s_bubble = lv_obj_create(parent_tile);
      lv_obj_set_size(s_bubble, 200, 40);
      lv_obj_set_style_radius(s_bubble, 10, 0);
      lv_obj_set_style_bg_color(s_bubble, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_bg_opa(s_bubble, LV_OPA_COVER, 0);
      lv_obj_set_style_border_color(s_bubble, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(s_bubble, 1, 0);
      lv_obj_set_style_pad_all(s_bubble, 4, 0);
      lv_obj_align(s_bubble, LV_ALIGN_TOP_MID, 0, 86);
      lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);

      s_bubble_label = lv_label_create(s_bubble);
      lv_label_set_text(s_bubble_label, "");
      lv_obj_set_style_text_color(s_bubble_label, lv_color_hex(0xE8E8ED), 0);
      lv_obj_set_style_text_font(s_bubble_label, &lv_font_montserrat_10, 0);
      lv_label_set_long_mode(s_bubble_label, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(s_bubble_label, 190);
      lv_obj_center(s_bubble_label);

      // --- Pet ring ---
      s_pet_ring = lv_arc_create(parent_tile);
      lv_obj_set_size(s_pet_ring, 200, 200);
      lv_arc_set_rotation(s_pet_ring, 0);
      lv_arc_set_bg_angles(s_pet_ring, 0, 360);
      lv_arc_set_value(s_pet_ring, 0);
      lv_arc_set_range(s_pet_ring, 0, 100);
      lv_obj_set_style_arc_width(s_pet_ring, 1, LV_PART_MAIN);
      lv_obj_set_style_arc_width(s_pet_ring, 0, LV_PART_INDICATOR);
      lv_obj_set_style_arc_color(s_pet_ring, lv_color_hex(0x252530), LV_PART_MAIN);
      lv_obj_align(s_pet_ring, LV_ALIGN_CENTER, 0, 0);

      // --- Hat line ---
      s_hat_line = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_hat_line, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_hat_line, s_accent_color(), 0);
      lv_label_set_text(s_hat_line, "");
      lv_obj_align(s_hat_line, LV_ALIGN_CENTER, 0, -36);
      lv_obj_add_flag(s_hat_line, LV_OBJ_FLAG_HIDDEN);

      // --- Sprite ---
      s_label_sprite = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_label_sprite, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_label_sprite, lv_color_hex(0xE8E8ED), 0);
      lv_label_set_text(s_label_sprite, "Loading...");
      lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, 0);

      // --- Action bar ---
      lv_obj_t *action_bar = lv_obj_create(parent_tile);
      lv_obj_set_size(action_bar, 200, 56);
      lv_obj_set_style_bg_opa(action_bar, LV_OPA_0, 0);
      lv_obj_set_style_border_width(action_bar, 0, 0);
      lv_obj_set_style_pad_all(action_bar, 0, 0);
      lv_obj_align(action_bar, LV_ALIGN_BOTTOM_MID, 0, -28);

      auto make_btn = [&](const char* label, int x_off, bool accent) -> lv_obj_t* {
          lv_obj_t *btn = lv_btn_create(action_bar);
          lv_obj_set_size(btn, 48, 48);
          lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
          lv_obj_set_style_bg_color(btn, accent ? lv_color_hex(0x3DD9D0) : lv_color_hex(0x13131A), 0);
          lv_obj_set_style_bg_opa(btn, accent ? LV_OPA_20 : LV_OPA_COVER, 0);
          lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(btn, 1, 0);
          lv_obj_align(btn, LV_ALIGN_CENTER, x_off, 0);

          lv_obj_t *lbl = lv_label_create(btn);
          lv_label_set_text(lbl, label);
          lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
          lv_obj_set_style_text_color(lbl, accent ? lv_color_hex(0x3DD9D0) : lv_color_hex(0xE8E8ED), 0);
          lv_obj_center(lbl);
          return btn;
      };

      s_btn_feed = make_btn("●", -60, false);
      s_btn_talk = make_btn("☺", 0, true);
      s_btn_play = make_btn("▶", 60, false);

      // Button labels under
      auto make_label = [&](const char* text, int x_off) {
          lv_obj_t *l = lv_label_create(action_bar);
          lv_label_set_text(l, text);
          lv_obj_set_style_text_font(l, &lv_font_montserrat_8, 0);
          lv_obj_set_style_text_color(l, lv_color_hex(0x6B6B78), 0);
          lv_obj_align(l, LV_ALIGN_CENTER, x_off, 32);
      };
      make_label("FEED", -60);
      make_label("TALK", 0);
      make_label("PLAY", 60);

      // Overlay for touch
      lv_obj_t *overlay = lv_obj_create(parent_tile);
      lv_obj_set_size(overlay, 360, 360);
      lv_obj_set_style_bg_opa(overlay, LV_OPA_0, 0);
      lv_obj_set_style_border_width(overlay, 0, 0);
      lv_obj_set_style_pad_all(overlay, 0, 0);
      lv_obj_align(overlay, LV_ALIGN_CENTER, 0, 0);
      lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

      // Event handlers will be wired in Task 6
      // lv_obj_add_event_cb(s_btn_feed, s_on_feed, LV_EVENT_CLICKED, NULL);
      // lv_obj_add_event_cb(s_btn_talk, s_on_talk, LV_EVENT_CLICKED, NULL);
      // lv_obj_add_event_cb(s_btn_play, s_on_play, LV_EVENT_CLICKED, NULL);
      // lv_obj_add_event_cb(overlay, s_on_sprite_press, LV_EVENT_CLICKED, NULL);

      // --- Hat menu (hidden) ---
      s_hat_menu = lv_obj_create(parent_tile);
      lv_obj_set_size(s_hat_menu, 240, 200);
      lv_obj_set_style_bg_color(s_hat_menu, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_bg_opa(s_hat_menu, LV_OPA_COVER, 0);
      lv_obj_set_style_border_color(s_hat_menu, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(s_hat_menu, 1, 0);
      lv_obj_set_style_radius(s_hat_menu, 16, 0);
      lv_obj_set_style_pad_all(s_hat_menu, 12, 0);
      lv_obj_center(s_hat_menu);
      lv_obj_add_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);

      lv_obj_t *hat_title = lv_label_create(s_hat_menu);
      lv_label_set_text(hat_title, "长按换帽");
      lv_obj_set_style_text_font(hat_title, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(hat_title, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(hat_title, LV_ALIGN_TOP_MID, 0, 4);

      s_hat_grid = lv_obj_create(s_hat_menu);
      lv_obj_set_size(s_hat_grid, 210, 120);
      lv_obj_set_style_bg_opa(s_hat_grid, LV_OPA_0, 0);
      lv_obj_set_style_border_width(s_hat_grid, 0, 0);
      lv_obj_set_flex_flow(s_hat_grid, LV_FLEX_FLOW_ROW_WRAP);
      lv_obj_set_flex_align(s_hat_grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
      lv_obj_set_style_pad_gap(s_hat_grid, 6, 0);
      lv_obj_align(s_hat_grid, LV_ALIGN_CENTER, 0, 8);

      for (int i = 0; i < HAT_COUNT; i++) {
          lv_obj_t *btn = lv_btn_create(s_hat_grid);
          lv_obj_set_size(btn, 48, 48);
          lv_obj_set_style_radius(btn, 10, 0);
          lv_obj_set_style_bg_color(btn, lv_color_hex(0x0A0A0F), 0);
          lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(btn, 1, 0);

          lv_obj_t *lbl = lv_label_create(btn);
          lv_label_set_text(lbl, HAT_LINES[i]);
          lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
          lv_obj_set_style_text_color(lbl, lv_color_hex(0xE8E8ED), 0);
          lv_obj_center(lbl);

          lv_obj_add_event_cb(btn, s_on_hat_pick, LV_EVENT_CLICKED, (void*)(intptr_t)i);
      }

      lv_obj_t *close_btn = lv_btn_create(s_hat_menu);
      lv_obj_set_size(close_btn, 60, 28);
      lv_obj_set_style_radius(close_btn, 8, 0);
      lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_border_color(close_btn, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(close_btn, 1, 0);
      lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -4);
      lv_obj_add_event_cb(close_btn, [](lv_event_t* e) {
          lv_obj_add_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);
      }, LV_EVENT_CLICKED, NULL);

      lv_obj_t *close_lbl = lv_label_create(close_btn);
      lv_label_set_text(close_lbl, "取消");
      lv_obj_set_style_text_font(close_lbl, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(close_lbl, lv_color_hex(0x6B6B78), 0);
      lv_obj_center(close_lbl);

      // Initial render
      s_render_sprite();
      s_update_mini_stats();
  }

  void ui_pet_update(void)
  {
      s_update_mini_stats();
  }

  void ui_pet_delete(void)
  {
      // Timers cleaned up by LVGL when parent is destroyed
  }
  ```

  Wait — this code references `s_anim_frame` and event handlers not yet defined. For Step 2 of Task 4, we need a version that compiles. Let me provide a minimal version with stubs.

  Actually, the event handler stubs and `s_anim_frame` need to be declared. Let me adjust: add the static variables and stubs at the top so it compiles.

  Let me rewrite the file content with necessary stubs:

  ```cpp
  #include "ui_pet.h"
  #include "pet_sprites.h"
  #include "pet_storage.h"
  #include <stdio.h>
  #include <string.h>

  // --- State ---
  static int s_anim_frame = 0;
  static lv_timer_t *s_anim_timer = NULL;

  // --- UI Objects ---
  static lv_obj_t *s_label_name;
  static lv_obj_t *s_label_rarity;
  static lv_obj_t *s_label_sprite;
  static lv_obj_t *s_bubble;
  static lv_obj_t *s_bubble_label;
  static lv_obj_t *s_hat_line;
  static lv_obj_t *s_pet_ring;
  static lv_obj_t *s_bar_hunger;
  static lv_obj_t *s_bar_joy;
  static lv_obj_t *s_val_hunger;
  static lv_obj_t *s_val_joy;
  static lv_obj_t *s_hat_menu;
  static lv_obj_t *s_hat_grid;

  // Stub event handlers
  static void s_on_feed(lv_event_t *e) { (void)e; }
  static void s_on_talk(lv_event_t *e) { (void)e; }
  static void s_on_play(lv_event_t *e) { (void)e; }
  static void s_on_sprite_press(lv_event_t *e) { (void)e; }
  static void s_on_hat_pick(lv_event_t *e) { (void)e; }

  static uint32_t s_rarity_color(PetRarity r)
  {
      switch (r) {
          case RARITY_COMMON:    return 0x8A8A95;
          case RARITY_UNCOMMON:  return 0x5EE7DF;
          case RARITY_RARE:      return 0x6B8CFF;
          case RARITY_EPIC:      return 0xC85EFF;
          case RARITY_LEGENDARY: return 0xFFD166;
          default: return 0x8A8A95;
      }
  }

  static lv_color_t s_accent_color(void)
  {
      return lv_color_hex(COLOR_HEX[g_pet.color]);
  }

  static void s_render_sprite(void)
  {
      const SpriteTemplate *tmpl = &SPECIES_TEMPLATES[g_pet.species];
      const char *frame = (s_anim_frame % 2 == 0) ? tmpl->frame0 : tmpl->frame1;
      char eye = EYE_CHARS[g_pet.eye];

      static char buf[256];
      if (tmpl->eye_count == 2) {
          snprintf(buf, sizeof(buf), frame, eye, eye);
      } else {
          snprintf(buf, sizeof(buf), frame, eye);
      }
      lv_label_set_text(s_label_sprite, buf);

      if (g_pet.hat != HAT_NONE) {
          lv_label_set_text(s_hat_line, HAT_LINES[g_pet.hat]);
          lv_obj_clear_flag(s_hat_line, LV_OBJ_FLAG_HIDDEN);
      } else {
          lv_obj_add_flag(s_hat_line, LV_OBJ_FLAG_HIDDEN);
      }
  }

  static void s_update_mini_stats(void)
  {
      lv_bar_set_value(s_bar_hunger, g_pet.hunger, LV_ANIM_ON);
      lv_bar_set_value(s_bar_joy, g_pet.joy, LV_ANIM_ON);
      lv_label_set_text_fmt(s_val_hunger, "%d", g_pet.hunger);
      lv_label_set_text_fmt(s_val_joy, "%d", g_pet.joy);
  }

  void ui_pet_create(lv_obj_t *parent_tile)
  {
      lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_pad_all(parent_tile, 0, 0);
      lv_obj_set_style_border_width(parent_tile, 0, 0);

      // Name
      s_label_name = lv_label_create(parent_tile);
      lv_label_set_text(s_label_name, g_pet.name);
      lv_obj_set_style_text_font(s_label_name, &lv_font_montserrat_14, 0);
      lv_obj_set_style_text_color(s_label_name, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(s_label_name, LV_ALIGN_TOP_MID, 0, 26);

      // Rarity
      s_label_rarity = lv_label_create(parent_tile);
      lv_label_set_text(s_label_rarity, RARITY_STARS[g_pet.rarity]);
      lv_obj_set_style_text_font(s_label_rarity, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_label_rarity, lv_color_hex(s_rarity_color(g_pet.rarity)), 0);
      lv_obj_align(s_label_rarity, LV_ALIGN_TOP_MID, 0, 44);

      // Mini stats container
      lv_obj_t *stat_cont = lv_obj_create(parent_tile);
      lv_obj_set_size(stat_cont, 220, 28);
      lv_obj_set_style_bg_opa(stat_cont, LV_OPA_0, 0);
      lv_obj_set_style_border_width(stat_cont, 0, 0);
      lv_obj_set_style_pad_all(stat_cont, 0, 0);
      lv_obj_align(stat_cont, LV_ALIGN_TOP_MID, 0, 58);

      // Hunger bar
      lv_obj_t *icon_h = lv_label_create(stat_cont);
      lv_label_set_text(icon_h, "◐");
      lv_obj_set_style_text_font(icon_h, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(icon_h, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(icon_h, LV_ALIGN_LEFT_MID, 0, -8);

      s_bar_hunger = lv_bar_create(stat_cont);
      lv_obj_set_size(s_bar_hunger, 180, 3);
      lv_bar_set_range(s_bar_hunger, 0, 100);
      lv_bar_set_value(s_bar_hunger, g_pet.hunger, LV_ANIM_OFF);
      lv_obj_align(s_bar_hunger, LV_ALIGN_LEFT_MID, 18, -8);
      lv_obj_set_style_bg_color(s_bar_hunger, lv_color_hex(0x13131A), LV_PART_MAIN);
      lv_obj_set_style_bg_color(s_bar_hunger, s_accent_color(), LV_PART_INDICATOR);
      lv_obj_set_style_radius(s_bar_hunger, 2, LV_PART_MAIN);
      lv_obj_set_style_radius(s_bar_hunger, 2, LV_PART_INDICATOR);

      s_val_hunger = lv_label_create(stat_cont);
      lv_label_set_text_fmt(s_val_hunger, "%d", g_pet.hunger);
      lv_obj_set_style_text_font(s_val_hunger, &lv_font_montserrat_8, 0);
      lv_obj_set_style_text_color(s_val_hunger, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(s_val_hunger, LV_ALIGN_RIGHT_MID, 0, -8);

      // Joy bar
      lv_obj_t *icon_j = lv_label_create(stat_cont);
      lv_label_set_text(icon_j, "♥");
      lv_obj_set_style_text_font(icon_j, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(icon_j, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(icon_j, LV_ALIGN_LEFT_MID, 0, 6);

      s_bar_joy = lv_bar_create(stat_cont);
      lv_obj_set_size(s_bar_joy, 180, 3);
      lv_bar_set_range(s_bar_joy, 0, 100);
      lv_bar_set_value(s_bar_joy, g_pet.joy, LV_ANIM_OFF);
      lv_obj_align(s_bar_joy, LV_ALIGN_LEFT_MID, 18, 6);
      lv_obj_set_style_bg_color(s_bar_joy, lv_color_hex(0x13131A), LV_PART_MAIN);
      lv_obj_set_style_bg_color(s_bar_joy, s_accent_color(), LV_PART_INDICATOR);
      lv_obj_set_style_radius(s_bar_joy, 2, LV_PART_MAIN);
      lv_obj_set_style_radius(s_bar_joy, 2, LV_PART_INDICATOR);

      s_val_joy = lv_label_create(stat_cont);
      lv_label_set_text_fmt(s_val_joy, "%d", g_pet.joy);
      lv_obj_set_style_text_font(s_val_joy, &lv_font_montserrat_8, 0);
      lv_obj_set_style_text_color(s_val_joy, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(s_val_joy, LV_ALIGN_RIGHT_MID, 0, 6);

      // Bubble
      s_bubble = lv_obj_create(parent_tile);
      lv_obj_set_size(s_bubble, 200, 40);
      lv_obj_set_style_radius(s_bubble, 10, 0);
      lv_obj_set_style_bg_color(s_bubble, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_bg_opa(s_bubble, LV_OPA_COVER, 0);
      lv_obj_set_style_border_color(s_bubble, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(s_bubble, 1, 0);
      lv_obj_set_style_pad_all(s_bubble, 4, 0);
      lv_obj_align(s_bubble, LV_ALIGN_TOP_MID, 0, 86);
      lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);

      s_bubble_label = lv_label_create(s_bubble);
      lv_label_set_text(s_bubble_label, "");
      lv_obj_set_style_text_color(s_bubble_label, lv_color_hex(0xE8E8ED), 0);
      lv_obj_set_style_text_font(s_bubble_label, &lv_font_montserrat_10, 0);
      lv_label_set_long_mode(s_bubble_label, LV_LABEL_LONG_WRAP);
      lv_obj_set_width(s_bubble_label, 190);
      lv_obj_center(s_bubble_label);

      // Pet ring
      s_pet_ring = lv_arc_create(parent_tile);
      lv_obj_set_size(s_pet_ring, 200, 200);
      lv_arc_set_rotation(s_pet_ring, 0);
      lv_arc_set_bg_angles(s_pet_ring, 0, 360);
      lv_arc_set_value(s_pet_ring, 0);
      lv_arc_set_range(s_pet_ring, 0, 100);
      lv_obj_set_style_arc_width(s_pet_ring, 1, LV_PART_MAIN);
      lv_obj_set_style_arc_width(s_pet_ring, 0, LV_PART_INDICATOR);
      lv_obj_set_style_arc_color(s_pet_ring, lv_color_hex(0x252530), LV_PART_MAIN);
      lv_obj_align(s_pet_ring, LV_ALIGN_CENTER, 0, 0);

      // Hat line
      s_hat_line = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_hat_line, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_hat_line, s_accent_color(), 0);
      lv_label_set_text(s_hat_line, "");
      lv_obj_align(s_hat_line, LV_ALIGN_CENTER, 0, -36);
      lv_obj_add_flag(s_hat_line, LV_OBJ_FLAG_HIDDEN);

      // Sprite
      s_label_sprite = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_label_sprite, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_label_sprite, lv_color_hex(0xE8E8ED), 0);
      lv_label_set_text(s_label_sprite, "Loading...");
      lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, 0);

      // Action bar
      lv_obj_t *action_bar = lv_obj_create(parent_tile);
      lv_obj_set_size(action_bar, 200, 56);
      lv_obj_set_style_bg_opa(action_bar, LV_OPA_0, 0);
      lv_obj_set_style_border_width(action_bar, 0, 0);
      lv_obj_set_style_pad_all(action_bar, 0, 0);
      lv_obj_align(action_bar, LV_ALIGN_BOTTOM_MID, 0, -28);

      auto make_btn = [&](const char* label, int x_off, bool accent) -> lv_obj_t* {
          lv_obj_t *btn = lv_btn_create(action_bar);
          lv_obj_set_size(btn, 48, 48);
          lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
          lv_obj_set_style_bg_color(btn, accent ? lv_color_hex(0x3DD9D0) : lv_color_hex(0x13131A), 0);
          lv_obj_set_style_bg_opa(btn, accent ? LV_OPA_20 : LV_OPA_COVER, 0);
          lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(btn, 1, 0);
          lv_obj_align(btn, LV_ALIGN_CENTER, x_off, 0);
          lv_obj_t *lbl = lv_label_create(btn);
          lv_label_set_text(lbl, label);
          lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
          lv_obj_set_style_text_color(lbl, accent ? lv_color_hex(0x3DD9D0) : lv_color_hex(0xE8E8ED), 0);
          lv_obj_center(lbl);
          return btn;
      };

      lv_obj_t *btn_feed = make_btn("●", -60, false);
      lv_obj_t *btn_talk = make_btn("☺", 0, true);
      lv_obj_t *btn_play = make_btn("▶", 60, false);

      auto make_label = [&](const char* text, int x_off) {
          lv_obj_t *l = lv_label_create(action_bar);
          lv_label_set_text(l, text);
          lv_obj_set_style_text_font(l, &lv_font_montserrat_8, 0);
          lv_obj_set_style_text_color(l, lv_color_hex(0x6B6B78), 0);
          lv_obj_align(l, LV_ALIGN_CENTER, x_off, 32);
      };
      make_label("FEED", -60);
      make_label("TALK", 0);
      make_label("PLAY", 60);

      // Overlay for touch
      lv_obj_t *overlay = lv_obj_create(parent_tile);
      lv_obj_set_size(overlay, 360, 360);
      lv_obj_set_style_bg_opa(overlay, LV_OPA_0, 0);
      lv_obj_set_style_border_width(overlay, 0, 0);
      lv_obj_set_style_pad_all(overlay, 0, 0);
      lv_obj_align(overlay, LV_ALIGN_CENTER, 0, 0);
      lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

      // Hat menu (hidden)
      s_hat_menu = lv_obj_create(parent_tile);
      lv_obj_set_size(s_hat_menu, 240, 200);
      lv_obj_set_style_bg_color(s_hat_menu, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_bg_opa(s_hat_menu, LV_OPA_COVER, 0);
      lv_obj_set_style_border_color(s_hat_menu, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(s_hat_menu, 1, 0);
      lv_obj_set_style_radius(s_hat_menu, 16, 0);
      lv_obj_set_style_pad_all(s_hat_menu, 12, 0);
      lv_obj_center(s_hat_menu);
      lv_obj_add_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);

      lv_obj_t *hat_title = lv_label_create(s_hat_menu);
      lv_label_set_text(hat_title, "长按换帽");
      lv_obj_set_style_text_font(hat_title, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(hat_title, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(hat_title, LV_ALIGN_TOP_MID, 0, 4);

      s_hat_grid = lv_obj_create(s_hat_menu);
      lv_obj_set_size(s_hat_grid, 210, 120);
      lv_obj_set_style_bg_opa(s_hat_grid, LV_OPA_0, 0);
      lv_obj_set_style_border_width(s_hat_grid, 0, 0);
      lv_obj_set_flex_flow(s_hat_grid, LV_FLEX_FLOW_ROW_WRAP);
      lv_obj_set_flex_align(s_hat_grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
      lv_obj_set_style_pad_gap(s_hat_grid, 6, 0);
      lv_obj_align(s_hat_grid, LV_ALIGN_CENTER, 0, 8);

      for (int i = 0; i < HAT_COUNT; i++) {
          lv_obj_t *btn = lv_btn_create(s_hat_grid);
          lv_obj_set_size(btn, 48, 48);
          lv_obj_set_style_radius(btn, 10, 0);
          lv_obj_set_style_bg_color(btn, lv_color_hex(0x0A0A0F), 0);
          lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(btn, 1, 0);
          lv_obj_t *lbl = lv_label_create(btn);
          lv_label_set_text(lbl, HAT_LINES[i]);
          lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
          lv_obj_set_style_text_color(lbl, lv_color_hex(0xE8E8ED), 0);
          lv_obj_center(lbl);
      }

      lv_obj_t *close_btn = lv_btn_create(s_hat_menu);
      lv_obj_set_size(close_btn, 60, 28);
      lv_obj_set_style_radius(close_btn, 8, 0);
      lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_border_color(close_btn, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(close_btn, 1, 0);
      lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -4);
      lv_obj_t *close_lbl = lv_label_create(close_btn);
      lv_label_set_text(close_lbl, "取消");
      lv_obj_set_style_text_font(close_lbl, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(close_lbl, lv_color_hex(0x6B6B78), 0);
      lv_obj_center(close_lbl);

      // Initial render
      s_render_sprite();
      s_update_mini_stats();
  }

  void ui_pet_update(void)
  {
      s_update_mini_stats();
  }

  void ui_pet_delete(void)
  {
  }
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

  Expected: 编译通过（lambda capture of `s_hat_menu` may fail in C++03 — if so, replace lambdas with regular static functions)

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/ui_pet.h myprojects/vibemate/ui_pet.cpp
  git commit -m "feat(ui): add pet screen base layout with mini stats, ring, hat menu"
  ```

---

## Task 5: 交互主屏 - 精灵动画与圆环脉冲

**Files:**
- Modify: `myprojects/vibemate/ui_pet.cpp`

- [ ] **Step 1: 添加浮空动画 + 精灵帧切换**

  在 `ui_pet.cpp` 的 `ui_pet_create` 末尾添加：

  ```cpp
  // Add after the initial render section:
  s_anim_timer = lv_timer_create([](lv_timer_t *t) {
      (void)t;
      s_anim_frame++;
      s_render_sprite();

      // Float animation: sine wave ±4px, period ~2.5s (62 ticks at 40ms)
      float phase = (s_anim_frame % 62) / 62.0f * 6.28318f;
      int offset = (int)(sinf(phase) * 4.0f);
      lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, offset);
      lv_obj_align(s_hat_line, LV_ALIGN_CENTER, 0, -36 + offset);
  }, 40, NULL);
  ```

  If lambda doesn't compile in Arduino C++, replace with a named static function:

  ```cpp
  static void s_anim_timer_cb(lv_timer_t *t)
  {
      (void)t;
      s_anim_frame++;
      s_render_sprite();
      float phase = (s_anim_frame % 62) / 62.0f * 6.28318f;
      int offset = (int)(sinf(phase) * 4.0f);
      lv_obj_align(s_label_sprite, LV_ALIGN_CENTER, 0, offset);
      lv_obj_align(s_hat_line, LV_ALIGN_CENTER, 0, -36 + offset);
  }
  ```

  Then in `ui_pet_create`: `s_anim_timer = lv_timer_create(s_anim_timer_cb, 40, NULL);`

- [ ] **Step 2: 添加圆环脉冲动画**

  添加新的定时器和变量：

  ```cpp
  static int s_ring_phase = 0;
  static lv_timer_t *s_ring_timer = NULL;

  static void s_ring_timer_cb(lv_timer_t *t)
  {
      (void)t;
      s_ring_phase++;
      // Pulse: period 3s at 50ms = 60 ticks
      float phase = (s_ring_phase % 60) / 60.0f * 6.28318f;
      float scale = 1.0f + sinf(phase) * 0.04f;  // 1.0 ~ 1.04
      int opa = (int)(127 + sinf(phase) * 77);   // 50 ~ 180 (0.2 ~ 0.7)
      if (opa < 50) opa = 50;
      if (opa > 180) opa = 180;

      lv_obj_set_style_arc_width(s_pet_ring, (int)(1.0f * scale), LV_PART_MAIN);
      lv_obj_set_style_arc_color(s_pet_ring, lv_color_hex(0x252530), LV_PART_MAIN);
      // LVGL doesn't support per-object opacity on arcs directly; use parent opacity workaround if needed
  }
  ```

  Note: LVGL arc opacity control is limited. A practical alternative: skip opacity animation, just do a subtle width pulse. Add to `ui_pet_create`:

  ```cpp
  s_ring_timer = lv_timer_create(s_ring_timer_cb, 50, NULL);
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/ui_pet.cpp
  git commit -m "feat(ui): add sprite float and ring pulse animations"
  ```

---

## Task 6: 交互主屏 - 交互、特效与气泡

**Files:**
- Modify: `myprojects/vibemate/ui_pet.cpp`

- [ ] **Step 1: 实现动作按钮事件**

  替换事件处理 stub 为完整实现：

  ```cpp
  static const char *IDLE_MESSAGES[] = {
      "在写 bug 呢？需要我帮忙吗？",
      "休息一下吧，眼睛会感谢你的。",
      "加油，离 commit 还有 47 个文件。",
      "我看好你，真的。",
      "你的代码风格…挺有创意的。",
      "这个变量名是认真的吗？",
  };
  static const int IDLE_MSG_COUNT = sizeof(IDLE_MESSAGES) / sizeof(IDLE_MESSAGES[0]);

  static const char *FEED_RESPONSES[] = {
      "咕咕…", "好吃。", "再来一口。", "满足了。"
  };
  static const int FEED_RESP_COUNT = sizeof(FEED_RESPONSES) / sizeof(FEED_RESPONSES[0]);

  static const char *PLAY_RESPONSES[] = {
      "好快！", "接着！", "开心。", "耶！"
  };
  static const int PLAY_RESP_COUNT = sizeof(PLAY_RESPONSES) / sizeof(PLAY_RESPONSES[0]);

  static void s_show_bubble(const char *text, uint32_t duration_ms)
  {
      lv_label_set_text(s_bubble_label, text);
      lv_obj_clear_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
      lv_timer_create([](lv_timer_t *t) {
          lv_obj_add_flag(s_bubble, LV_OBJ_FLAG_HIDDEN);
          lv_timer_del(t);
      }, duration_ms, NULL);
  }

  static void s_on_feed(lv_event_t *e)
  {
      (void)e;
      if (g_pet.hunger >= 100) {
          s_show_bubble("已经很饱了！", 2000);
          return;
      }
      g_pet.hunger = (g_pet.hunger + 8 > 100) ? 100 : g_pet.hunger + 8;
      pet_save();
      s_update_mini_stats();
      s_show_bubble(FEED_RESPONSES[pet_rng_range(FEED_RESP_COUNT)], 1800);
  }

  static void s_on_talk(lv_event_t *e)
  {
      (void)e;
      s_show_bubble(IDLE_MESSAGES[pet_rng_range(IDLE_MSG_COUNT)], 3000);
  }

  static void s_on_play(lv_event_t *e)
  {
      (void)e;
      if (g_pet.joy >= 100) {
          s_show_bubble("已经很开心了！", 2000);
          return;
      }
      g_pet.joy = (g_pet.joy + 10 > 100) ? 100 : g_pet.joy + 10;
      pet_save();
      s_update_mini_stats();
      s_show_bubble(PLAY_RESPONSES[pet_rng_range(PLAY_RESP_COUNT)], 1800);
  }

  static void s_on_hat_pick(lv_event_t *e)
  {
      lv_obj_t *btn = lv_event_get_target(e);
      // Find index from user_data
      intptr_t idx = (intptr_t)lv_event_get_user_data(e);
      g_pet.hat = (PetHat)idx;
      pet_save();
      s_render_sprite();
      lv_obj_add_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);
  }
  ```

  Wire up the event handlers in `ui_pet_create` by adding after button creation:

  ```cpp
  lv_obj_add_event_cb(btn_feed, s_on_feed, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(btn_talk, s_on_talk, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(btn_play, s_on_play, LV_EVENT_CLICKED, NULL);
  ```

  And for hat buttons, update the loop:

  ```cpp
  for (int i = 0; i < HAT_COUNT; i++) {
      // ... create btn and lbl ...
      lv_obj_set_user_data(btn, (void*)(intptr_t)i);
      lv_obj_add_event_cb(btn, s_on_hat_pick, LV_EVENT_CLICKED, (void*)(intptr_t)i);
  }
  ```

- [ ] **Step 2: 实现长按检测**

  添加长按状态变量和回调：

  ```cpp
  static bool s_pressing = false;
  static uint32_t s_press_start = 0;
  static lv_timer_t *s_longpress_timer = NULL;

  static void s_longpress_cb(lv_timer_t *t)
  {
      (void)t;
      if (s_pressing) {
          lv_obj_clear_flag(s_hat_menu, LV_OBJ_FLAG_HIDDEN);
          s_pressing = false;
      }
  }

  static void s_on_overlay_press(lv_event_t *e)
  {
      lv_indev_t *indev = lv_indev_get_act();
      lv_point_t vect;
      lv_indev_get_vect(indev, &vect);
      // Just detect press start
      s_pressing = true;
      s_press_start = lv_tick_get();
      if (s_longpress_timer) lv_timer_del(s_longpress_timer);
      s_longpress_timer = lv_timer_create(s_longpress_cb, 600, NULL);
      s_longpress_timer->repeat_count = 1;
  }

  static void s_on_overlay_release(lv_event_t *e)
  {
      (void)e;
      s_pressing = false;
      if (s_longpress_timer) {
          lv_timer_del(s_longpress_timer);
          s_longpress_timer = NULL;
      }
      uint32_t elapsed = lv_tick_get() - s_press_start;
      if (elapsed < 600) {
          // Short press = talk
          s_on_talk(NULL);
      }
  }
  ```

  Replace overlay event with:

  ```cpp
  lv_obj_add_event_cb(overlay, s_on_overlay_press, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(overlay, s_on_overlay_release, LV_EVENT_RELEASED, NULL);
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/ui_pet.cpp
  git commit -m "feat(ui): wire up feed/talk/play buttons, bubble dialog, long-press hat menu"
  ```

---

## Task 7: 新增伙伴选择页 (ui_pet_select)

**Files:**
- Create: `myprojects/vibemate/ui_pet_select.h`
- Create: `myprojects/vibemate/ui_pet_select.cpp`

- [ ] **Step 1: 创建 ui_pet_select.h**

  ```c
  #ifndef UI_PET_SELECT_H
  #define UI_PET_SELECT_H

  #include <lvgl.h>

  void ui_pet_select_create(lv_obj_t *parent_tile);

  #endif
  ```

- [ ] **Step 2: 创建 ui_pet_select.cpp**

  ```cpp
  #include "ui_pet_select.h"
  #include "pet_sprites.h"
  #include "pet_storage.h"
  #include <stdio.h>
  #include <string.h>

  static int s_sel_species = 3; // default cat
  static int s_sel_eye = 1;
  static int s_sel_hat = 0;
  static int s_sel_color = 0;

  static lv_obj_t *s_preview_sprite;
  static lv_obj_t *s_preview_ring;
  static lv_obj_t *s_preview_species;
  static lv_obj_t *s_preview_rarity;
  static lv_obj_t *s_hat_badge;

  static const char* FACE_PREVIEWS[PET_SPECIES_COUNT] = {
      "<(·)", "(·>)", "(··)", "(=ω=)", "<~~>",
      "~(··)~", "(··)", "(·>)", "[_ _]", "·(@)",
      "/··\\", "}··{", "(oo)", "|··|", "[··]",
      "(..)", "|··|", "(..)"
  };

  static const char* MINI_ICONS[PET_SPECIES_COUNT] = {
      "D", "G", "O", "C", "R", "8", "o", "P", "T", "@",
      "~", "A", "B", "#", "◈", "R", "M", "="
  };

  static uint32_t s_color_hex(int idx)
  {
      return COLOR_HEX[idx % COLOR_COUNT];
  }

  static void s_update_preview(void)
  {
      const char *face = FACE_PREVIEWS[s_sel_species];
      char eye = EYE_CHARS[s_sel_eye];
      uint32_t col = s_color_hex(s_sel_color);

      static char buf[32];
      const char *p = face;
      char *dst = buf;
      while (*p) {
          if (*p == '·') {
              *dst++ = eye;
          } else {
              *dst++ = *p;
          }
          p++;
      }
      *dst = '\0';

      lv_label_set_text(s_preview_sprite, buf);
      lv_obj_set_style_text_color(s_preview_sprite, lv_color_hex(col), 0);

      // Rarity (deterministic per species)
      uint32_t seed = (uint32_t)s_sel_species * 1103515245u + 12345u;
      uint32_t r = seed % 100;
      PetRarity rarity;
      if (r < 60) rarity = RARITY_COMMON;
      else if (r < 85) rarity = RARITY_UNCOMMON;
      else if (r < 95) rarity = RARITY_RARE;
      else if (r < 99) rarity = RARITY_EPIC;
      else rarity = RARITY_LEGENDARY;

      lv_label_set_text(s_preview_rarity, RARITY_STARS[rarity]);

      // Ring color
      lv_obj_set_style_arc_color(s_preview_ring, lv_color_hex(col), LV_PART_MAIN);

      // Hat badge
      if (s_sel_hat == 0) {
          lv_obj_add_flag(s_hat_badge, LV_OBJ_FLAG_HIDDEN);
      } else {
          lv_obj_clear_flag(s_hat_badge, LV_OBJ_FLAG_HIDDEN);
          lv_label_set_text(s_hat_badge, HAT_LINES[s_sel_hat]);
          lv_obj_set_style_text_color(s_hat_badge, lv_color_hex(col), 0);
      }

      lv_label_set_text(s_preview_species, SPECIES_NAMES[s_sel_species]);
  }

  static void s_apply_selection(void)
  {
      g_pet.species = (PetSpecies)s_sel_species;
      g_pet.eye = (PetEye)s_sel_eye;
      g_pet.hat = (PetHat)s_sel_hat;
      g_pet.color = (PetColor)s_sel_color;
      pet_save();
  }

  static void s_on_prev(lv_event_t *e)
  {
      (void)e;
      s_sel_species = (s_sel_species - 1 + PET_SPECIES_COUNT) % PET_SPECIES_COUNT;
      s_update_preview();
  }

  static void s_on_next(lv_event_t *e)
  {
      (void)e;
      s_sel_species = (s_sel_species + 1) % PET_SPECIES_COUNT;
      s_update_preview();
  }

  static void s_on_generate(lv_event_t *e)
  {
      (void)e;
      uint64_t mac = ESP.getEfuseMac();
      uint32_t seed = (uint32_t)(mac ^ (mac >> 32)) + s_sel_species * 7919;
      uint32_t rng = seed;
      auto next = [&]() -> uint32_t {
          rng = rng * 1103515245u + 12345u;
          return rng;
      };
      s_sel_eye = next() % EYE_COUNT;
      s_sel_hat = next() % HAT_COUNT;
      s_sel_color = next() % COLOR_COUNT;
      s_update_preview();
      s_apply_selection();
  }

  void ui_pet_select_create(lv_obj_t *parent_tile)
  {
      lv_obj_set_style_bg_color(parent_tile, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_pad_all(parent_tile, 0, 0);
      lv_obj_set_style_border_width(parent_tile, 0, 0);

      // Title
      lv_obj_t *title = lv_label_create(parent_tile);
      lv_label_set_text(title, "选择伙伴");
      lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
      lv_obj_set_style_text_color(title, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

      lv_obj_t *subtitle = lv_label_create(parent_tile);
      lv_label_set_text(subtitle, "自定义你的 Buddy");
      lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_8, 0);
      lv_obj_set_style_text_color(subtitle, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 38);

      // Side peeks
      lv_obj_t *peek_l = lv_label_create(parent_tile);
      lv_label_set_text(peek_l, MINI_ICONS[(s_sel_species - 1 + PET_SPECIES_COUNT) % PET_SPECIES_COUNT]);
      lv_obj_set_style_text_font(peek_l, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(peek_l, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(peek_l, LV_ALIGN_LEFT_MID, 8, -20);

      lv_obj_t *peek_r = lv_label_create(parent_tile);
      lv_label_set_text(peek_r, MINI_ICONS[(s_sel_species + 1) % PET_SPECIES_COUNT]);
      lv_obj_set_style_text_font(peek_r, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(peek_r, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(peek_r, LV_ALIGN_RIGHT_MID, -8, -20);

      // Preview stage
      s_preview_ring = lv_arc_create(parent_tile);
      lv_obj_set_size(s_preview_ring, 80, 80);
      lv_arc_set_rotation(s_preview_ring, 0);
      lv_arc_set_bg_angles(s_preview_ring, 0, 360);
      lv_arc_set_value(s_preview_ring, 0);
      lv_arc_set_range(s_preview_ring, 0, 100);
      lv_obj_set_style_arc_width(s_preview_ring, 2, LV_PART_MAIN);
      lv_obj_set_style_arc_width(s_preview_ring, 0, LV_PART_INDICATOR);
      lv_obj_set_style_arc_color(s_preview_ring, lv_color_hex(0x3DD9D0), LV_PART_MAIN);
      lv_obj_align(s_preview_ring, LV_ALIGN_CENTER, 0, -20);

      s_hat_badge = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_hat_badge, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(s_hat_badge, lv_color_hex(0x3DD9D0), 0);
      lv_obj_align(s_hat_badge, LV_ALIGN_CENTER, 0, -56);
      lv_obj_add_flag(s_hat_badge, LV_OBJ_FLAG_HIDDEN);

      s_preview_sprite = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_preview_sprite, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_preview_sprite, lv_color_hex(0xE8E8ED), 0);
      lv_obj_align(s_preview_sprite, LV_ALIGN_CENTER, 0, -20);

      // Species name + rarity
      s_preview_species = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_preview_species, &lv_font_montserrat_8, 0);
      lv_obj_set_style_text_color(s_preview_species, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(s_preview_species, LV_ALIGN_TOP_MID, 0, 168);

      s_preview_rarity = lv_label_create(parent_tile);
      lv_obj_set_style_text_font(s_preview_rarity, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(s_preview_rarity, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(s_preview_rarity, LV_ALIGN_TOP_MID, 0, 182);

      // Eye selector
      lv_obj_t *eye_cont = lv_obj_create(parent_tile);
      lv_obj_set_size(eye_cont, 260, 26);
      lv_obj_set_style_bg_opa(eye_cont, LV_OPA_0, 0);
      lv_obj_set_style_border_width(eye_cont, 0, 0);
      lv_obj_set_style_pad_all(eye_cont, 0, 0);
      lv_obj_align(eye_cont, LV_ALIGN_TOP_MID, 0, 200);

      lv_obj_t *eye_icon = lv_label_create(eye_cont);
      lv_label_set_text(eye_icon, "◉");
      lv_obj_set_style_text_font(eye_icon, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(eye_icon, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(eye_icon, LV_ALIGN_LEFT_MID, 0, 0);

      for (int i = 0; i < EYE_COUNT; i++) {
          lv_obj_t *btn = lv_btn_create(eye_cont);
          lv_obj_set_size(btn, 22, 22);
          lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
          lv_obj_set_style_bg_color(btn, lv_color_hex(0x13131A), 0);
          lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(btn, 1, 0);
          lv_obj_align(btn, LV_ALIGN_LEFT_MID, 24 + i * 28, 0);

          lv_obj_t *lbl = lv_label_create(btn);
          static char eye_buf[4];
          snprintf(eye_buf, sizeof(eye_buf), "%c", EYE_CHARS[i]);
          lv_label_set_text(lbl, eye_buf);
          lv_obj_set_style_text_font(lbl, &lv_font_montserrat_10, 0);
          lv_obj_set_style_text_color(lbl, lv_color_hex(0xE8E8ED), 0);
          lv_obj_center(lbl);

          lv_obj_add_event_cb(btn, [](lv_event_t *e) {
              s_sel_eye = (int)(intptr_t)lv_event_get_user_data(e);
              s_update_preview();
              s_apply_selection();
          }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
      }

      // Color selector
      lv_obj_t *col_cont = lv_obj_create(parent_tile);
      lv_obj_set_size(col_cont, 260, 26);
      lv_obj_set_style_bg_opa(col_cont, LV_OPA_0, 0);
      lv_obj_set_style_border_width(col_cont, 0, 0);
      lv_obj_set_style_pad_all(col_cont, 0, 0);
      lv_obj_align(col_cont, LV_ALIGN_TOP_MID, 0, 230);

      lv_obj_t *col_icon = lv_label_create(col_cont);
      lv_label_set_text(col_icon, "●");
      lv_obj_set_style_text_font(col_icon, &lv_font_montserrat_10, 0);
      lv_obj_set_style_text_color(col_icon, lv_color_hex(0x6B6B78), 0);
      lv_obj_align(col_icon, LV_ALIGN_LEFT_MID, 0, 0);

      for (int i = 0; i < COLOR_COUNT; i++) {
          lv_obj_t *btn = lv_btn_create(col_cont);
          lv_obj_set_size(btn, 18, 18);
          lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
          lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_HEX[i]), 0);
          lv_obj_set_style_border_color(btn, lv_color_hex(0x252530), 0);
          lv_obj_set_style_border_width(btn, 1, 0);
          lv_obj_align(btn, LV_ALIGN_LEFT_MID, 24 + i * 24, 0);

          lv_obj_add_event_cb(btn, [](lv_event_t *e) {
              s_sel_color = (int)(intptr_t)lv_event_get_user_data(e);
              s_update_preview();
              s_apply_selection();
          }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
      }

      // Action bar
      lv_obj_t *action_bar = lv_obj_create(parent_tile);
      lv_obj_set_size(action_bar, 160, 44);
      lv_obj_set_style_bg_opa(action_bar, LV_OPA_0, 0);
      lv_obj_set_style_border_width(action_bar, 0, 0);
      lv_obj_set_style_pad_all(action_bar, 0, 0);
      lv_obj_align(action_bar, LV_ALIGN_BOTTOM_MID, 0, -20);

      lv_obj_t *btn_prev = lv_btn_create(action_bar);
      lv_obj_set_size(btn_prev, 36, 36);
      lv_obj_set_style_radius(btn_prev, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(btn_prev, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_border_color(btn_prev, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(btn_prev, 1, 0);
      lv_obj_align(btn_prev, LV_ALIGN_LEFT_MID, 0, 0);
      lv_obj_t *prev_lbl = lv_label_create(btn_prev);
      lv_label_set_text(prev_lbl, "<");
      lv_obj_set_style_text_font(prev_lbl, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(prev_lbl, lv_color_hex(0xE8E8ED), 0);
      lv_obj_center(prev_lbl);
      lv_obj_add_event_cb(btn_prev, s_on_prev, LV_EVENT_CLICKED, NULL);

      lv_obj_t *btn_gen = lv_btn_create(action_bar);
      lv_obj_set_size(btn_gen, 44, 44);
      lv_obj_set_style_radius(btn_gen, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(btn_gen, lv_color_hex(0x3DD9D0), 0);
      lv_obj_set_style_bg_opa(btn_gen, LV_OPA_20, 0);
      lv_obj_set_style_border_color(btn_gen, lv_color_hex(0x3DD9D0), 0);
      lv_obj_set_style_border_width(btn_gen, 1, 0);
      lv_obj_align(btn_gen, LV_ALIGN_CENTER, 0, 0);
      lv_obj_t *gen_lbl = lv_label_create(btn_gen);
      lv_label_set_text(gen_lbl, "↻");
      lv_obj_set_style_text_font(gen_lbl, &lv_font_montserrat_16, 0);
      lv_obj_set_style_text_color(gen_lbl, lv_color_hex(0x3DD9D0), 0);
      lv_obj_center(gen_lbl);
      lv_obj_add_event_cb(btn_gen, s_on_generate, LV_EVENT_CLICKED, NULL);

      lv_obj_t *btn_next = lv_btn_create(action_bar);
      lv_obj_set_size(btn_next, 36, 36);
      lv_obj_set_style_radius(btn_next, LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(btn_next, lv_color_hex(0x13131A), 0);
      lv_obj_set_style_border_color(btn_next, lv_color_hex(0x252530), 0);
      lv_obj_set_style_border_width(btn_next, 1, 0);
      lv_obj_align(btn_next, LV_ALIGN_RIGHT_MID, 0, 0);
      lv_obj_t *next_lbl = lv_label_create(btn_next);
      lv_label_set_text(next_lbl, ">");
      lv_obj_set_style_text_font(next_lbl, &lv_font_montserrat_12, 0);
      lv_obj_set_style_text_color(next_lbl, lv_color_hex(0xE8E8ED), 0);
      lv_obj_center(next_lbl);
      lv_obj_add_event_cb(btn_next, s_on_next, LV_EVENT_CLICKED, NULL);

      // Load current pet settings
      s_sel_species = g_pet.species;
      s_sel_eye = g_pet.eye;
      s_sel_hat = g_pet.hat;
      s_sel_color = g_pet.color;
      s_update_preview();
  }
  ```

- [ ] **Step 3: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

  Expected: 编译通过（lambda 可能需要在 Arduino C++ 中替换为命名函数）

- [ ] **Step 4: Commit**

  ```bash
  git add myprojects/vibemate/ui_pet_select.h myprojects/vibemate/ui_pet_select.cpp
  git commit -m "feat(ui): add pet selection page with species carousel and customizers"
  ```

---

## Task 8: 主程序集成与状态衰减

**Files:**
- Modify: `myprojects/vibemate/vibemate.ino`

- [ ] **Step 1: 扩展 tileview 到 5 页，调整启动逻辑**

  修改 `vibemate.ino`：

  ```cpp
  #include "config.h"
  #include "Display_ST77916.h"
  #include "LVGL_Driver.h"
  #include "I2C_Driver.h"
  #include "network_manager.h"
  #include "kimi_api.h"
  #include "ui_usage.h"
  #include "ui_device.h"
  #include "ui_pet.h"
  #include "ui_pet_detail.h"
  #include "ui_pet_select.h"
  #include "pet_sprites.h"
  #include "pet_storage.h"
  #include "rtc_bsp.h"
  #include <lvgl.h>
  #include <Wire.h>
  #include <BQ27220.h>

  static lv_obj_t *tileview;
  static lv_obj_t *tile_pet_select;
  static lv_obj_t *tile_pet_detail;
  static lv_obj_t *tile_pet;
  static lv_obj_t *tile_usage;
  static lv_obj_t *tile_device;
  static lv_timer_t *api_timer = NULL;
  static lv_timer_t *device_timer = NULL;
  static lv_timer_t *ui_timer = NULL;
  static lv_timer_t *decay_timer = NULL;

  SemaphoreHandle_t wire_mutex;
  BQ27220 g_bq27220;

  static void api_timer_cb(lv_timer_t *timer)
  {
      if (network_is_connected()) {
          kimi_api_refresh_now();
      }
  }

  static void device_timer_cb(lv_timer_t *timer)
  {
      ui_device_update();
      ui_pet_update();
  }

  static void ui_timer_cb(lv_timer_t *timer)
  {
      if (g_ui_needs_update) {
          g_ui_needs_update = false;
          ui_usage_update(&g_kimi_data);
      }
  }

  static void decay_timer_cb(lv_timer_t *timer)
  {
      (void)timer;
      bool changed = false;
      if (g_pet.hunger > 0) { g_pet.hunger--; changed = true; }
      if (g_pet.joy > 0) { g_pet.joy--; changed = true; }
      if (changed) {
          pet_save();
          ui_pet_update();
      }
  }

  void setup()
  {
      Serial.begin(115200);
      Serial.println("VibeMate starting...");
      wire_mutex = xSemaphoreCreateMutex();

      Serial.println("[INIT] I2C...");
      I2C_Init();

      Serial.println("[INIT] Backlight OFF...");
      pinMode(LCD_Backlight_PIN, OUTPUT);
      digitalWrite(LCD_Backlight_PIN, LOW);
      Backlight_Init();
      Set_Backlight(0);

      Serial.println("[INIT] LCD...");
      LCD_Init();

      Serial.println("[INIT] LVGL...");
      Lvgl_Init();

      Serial.println("[INIT] BQ27220...");
      if (!g_bq27220.begin(Wire, 0x55, I2C_SDA_PIN, I2C_SCL_PIN, 400000)) {
          Serial.println("BQ27220 not found");
      } else {
          Serial.println("BQ27220 ready");
      }

      Serial.println("[INIT] RTC...");
      rtc_init();

      Serial.println("[INIT] Network...");
      network_init();

      if (network_is_connected()) {
          Serial.println("[INIT] NTP sync...");
          network_sync_ntp_to_rtc();
      }

      Serial.println("[INIT] Kimi API...");
      kimi_api_init();

      // Load or generate pet
      if (!pet_load()) {
          Serial.println("[INIT] No saved pet, generating...");
          pet_generate();
          pet_save();
      } else {
          Serial.println("[INIT] Pet loaded from NVS");
      }

      lv_obj_t *scr = lv_scr_act();
      lv_obj_set_style_bg_color(scr, lv_color_hex(0x0A0A0F), 0);

      tileview = lv_tileview_create(scr);
      lv_obj_set_size(tileview, 360, 360);
      lv_obj_set_style_bg_color(tileview, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_pad_all(tileview, 0, 0);
      lv_obj_set_style_border_width(tileview, 0, 0);
      lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

      tile_pet_select = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
      tile_pet_detail = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
      tile_pet    = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);
      tile_usage  = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_HOR);
      tile_device = lv_tileview_add_tile(tileview, 4, 0, LV_DIR_HOR);

      for (lv_obj_t *t : {tile_pet_select, tile_pet_detail, tile_pet, tile_usage, tile_device}) {
          lv_obj_set_style_bg_color(t, lv_color_hex(0x0A0A0F), 0);
          lv_obj_set_style_pad_all(t, 0, 0);
          lv_obj_set_style_border_width(t, 0, 0);
      }

      // Default to Pet page
      lv_obj_set_tile(tileview, tile_pet, LV_ANIM_OFF);

      ui_pet_select_create(tile_pet_select);
      ui_pet_detail_create(tile_pet_detail);
      ui_pet_create(tile_pet);
      ui_usage_create(tile_usage);
      ui_device_create(tile_device);

      Serial.println("[INIT] Backlight ON...");
      Set_Backlight(BACKLIGHT_BRIGHTNESS);

      api_timer = lv_timer_create(api_timer_cb, API_REFRESH_INTERVAL_MS, NULL);
      device_timer = lv_timer_create(device_timer_cb, 1000, NULL);
      ui_timer = lv_timer_create(ui_timer_cb, 500, NULL);
      decay_timer = lv_timer_create(decay_timer_cb, 60000, NULL);

      if (network_is_connected()) {
          kimi_api_refresh_now();
      }

      Serial.println("VibeMate ready!");
  }

  void loop()
  {
      Lvgl_Loop();
      network_check();
      vTaskDelay(pdMS_TO_TICKS(10));
  }
  ```

  Note: The `for (lv_obj_t *t : { ... })` is C++11 range-for with initializer list. If Arduino doesn't support this, replace with individual calls:

  ```cpp
      lv_obj_set_style_bg_color(tile_pet_select, lv_color_hex(0x0A0A0F), 0);
      lv_obj_set_style_pad_all(tile_pet_select, 0, 0);
      lv_obj_set_style_border_width(tile_pet_select, 0, 0);
      // ... repeat for each tile
  ```

- [ ] **Step 2: 编译验证**

  ```bash
  cd myprojects/vibemate && arduino-cli compile --fqbn "esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi" .
  ```

  Expected: 编译通过

- [ ] **Step 3: Commit**

  ```bash
  git add myprojects/vibemate/vibemate.ino
  git commit -m "feat(main): integrate 5-page tileview, NVS loading, hunger/joy decay timer"
  ```

---

## Self-Review

**1. Spec coverage:**
- [x] 18 物种 — Task 1
- [x] 帽子/颜色系统 — Task 1 (数据), Task 4/6 (UI), Task 7 (选择)
- [x] 饱食度/开心值 — Task 1 (数据), Task 4 (UI), Task 6 (按钮), Task 8 (衰减)
- [x] NVS 持久化 — Task 2
- [x] 交互主屏 — Task 4/5/6
- [x] 属性面板 — Task 3
- [x] 伙伴选择 — Task 7
- [x] 动画特效 — Task 5/6
- [x] 5 页 tileview — Task 8
- [x] 启动默认 Pet 页 — Task 8

**2. Placeholder scan:** 无 TBD/TODO/"implement later"/"similar to"

**3. Type consistency:**
- `PetData` 字段名在所有任务中一致：`species`, `eye`, `hat`, `color`, `rarity`, `shiny`, `stats[]`, `hunger`, `joy`, `name[]`
- `pet_save()` / `pet_load()` 接口在 Task 2 定义，Task 6/7/8 使用
- `s_accent_color()` 使用 `COLOR_HEX[g_pet.color]` 在所有任务中一致

---

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-06-06-buddy-redesign-plan.md`.

**Two execution options:**

**1. Subagent-Driven (recommended)** — I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** — Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
