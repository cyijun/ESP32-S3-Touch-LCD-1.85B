#include "pet_manager.h"
#include "pet_storage.h"
#include "ui_pet.h"
#include "ui_pet_detail.h"
#include "debug_trace.h"

// pet_manager 封装全局 g_pet 的访问，所有修改自动持久化并更新 UI。
// 方案 A：直接操作全局 g_pet（定义在 pet_sprites.cpp），最小化改动。

// ========== Read-only access ==========
const PetData* pet_get(void)
{
    return &g_pet;
}

// ========== Feed ==========
void pet_feed(uint8_t amount)
{
    TRACE_STORAGE_ENTER();
    if (g_pet.hunger >= 100) {
        TRACE_STORAGE("feed skipped, already full");
        TRACE_STORAGE_EXIT();
        return;
    }
    g_pet.hunger += amount;
    if (g_pet.hunger > 100) g_pet.hunger = 100;
    pet_save();
    ui_pet_update();
    TRACE_STORAGE("feed amount=%d hunger=%d", amount, g_pet.hunger);
    TRACE_STORAGE_EXIT();
}

// ========== Play ==========
void pet_play(uint8_t amount)
{
    TRACE_STORAGE_ENTER();
    if (g_pet.joy >= 100) {
        TRACE_STORAGE("play skipped, already max joy");
        TRACE_STORAGE_EXIT();
        return;
    }
    g_pet.joy += amount;
    if (g_pet.joy > 100) g_pet.joy = 100;
    pet_save();
    ui_pet_update();
    TRACE_STORAGE("play amount=%d joy=%d", amount, g_pet.joy);
    TRACE_STORAGE_EXIT();
}

// ========== Set hat (from pet page long-press menu) ==========
void pet_set_hat(PetHat hat)
{
    TRACE_STORAGE_ENTER();
    g_pet.hat = hat;
    pet_save();
    ui_pet_update();
    TRACE_STORAGE("hat=%d", (int)hat);
    TRACE_STORAGE_EXIT();
}

// ========== Set appearance (from pet_select page) ==========
void pet_set_appearance(PetSpecies species, PetEye eye, PetColor color)
{
    TRACE_STORAGE_ENTER();
    g_pet.species = species;
    g_pet.eye = eye;
    g_pet.color = color;
    pet_save();
    ui_pet_detail_update();
    ui_pet_update();
    TRACE_STORAGE("species=%d eye=%d color=%d", (int)species, (int)eye, (int)color);
    TRACE_STORAGE_EXIT();
}

// ========== Set hat (from pet_select page) ==========
void pet_set_hat_select(PetHat hat)
{
    TRACE_STORAGE_ENTER();
    g_pet.hat = hat;
    pet_save();
    ui_pet_detail_update();
    ui_pet_update();
    TRACE_STORAGE("hat=%d", (int)hat);
    TRACE_STORAGE_EXIT();
}

// ========== Generate new pet (random) ==========
void pet_generate_new(void)
{
    TRACE_STORAGE_ENTER();
    pet_generate();
    pet_save();
    ui_pet_detail_update();
    ui_pet_update();
    TRACE_STORAGE("generated new pet");
    TRACE_STORAGE_EXIT();
}

// ========== Reroll stats (keep appearance, reroll stats/rarity) ==========
void pet_reroll_stats(void)
{
    TRACE_STORAGE_ENTER();
    ::pet_reset_stats();  // call the one in pet_sprites.cpp
    pet_save();
    ui_pet_detail_update();
    ui_pet_update();
    TRACE_STORAGE("reroll stats");
    TRACE_STORAGE_EXIT();
}

// ========== Decay (called by timer) ==========
bool pet_decay(void)
{
    bool changed = false;
    if (g_pet.hunger > 0) { g_pet.hunger--; changed = true; }
    if (g_pet.joy > 0) { g_pet.joy--; changed = true; }
    if (changed) {
        TRACE_STORAGE("decay hunger=%d joy=%d", g_pet.hunger, g_pet.joy);
        pet_save();
        ui_pet_update();
    }
    return changed;
}

// ========== Init (load from NVS or generate) ==========
void pet_manager_init(void)
{
    TRACE_STORAGE_ENTER();
    if (!pet_load()) {
        TRACE_STORAGE("no saved pet, generating new...");
        pet_generate();
        pet_save();
    } else {
        TRACE_STORAGE("pet loaded from NVS");
    }
    TRACE_STORAGE_EXIT();
}
