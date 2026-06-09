#ifndef PET_MANAGER_H
#define PET_MANAGER_H

#include "pet_sprites.h"

#ifdef __cplusplus
extern "C" {
#endif

// 获取宠物数据（只读）
const PetData* pet_get(void);

// 喂养（增加 hunger，上限 100）
void pet_feed(uint8_t amount);

// 玩耍（增加 joy，上限 100）
void pet_play(uint8_t amount);

// 设置帽子（用于 pet 页面长按菜单）
void pet_set_hat(PetHat hat);

// 设置外观（物种/眼睛/颜色，用于 pet_select 页面）
void pet_set_appearance(PetSpecies species, PetEye eye, PetColor color);

// 设置帽子（用于 pet_select 页面）
void pet_set_hat_select(PetHat hat);

// 完全重新生成宠物（随机）
void pet_generate_new(void);

// 重新随机 stats/rarity（保留外观）
void pet_reroll_stats(void);

// 衰减（定时调用，减少 hunger 和 joy）
// 返回 true 如果有变化
bool pet_decay(void);

// 初始化（从 NVS 加载或生成新宠物）
void pet_manager_init(void);

#ifdef __cplusplus
}
#endif

#endif
