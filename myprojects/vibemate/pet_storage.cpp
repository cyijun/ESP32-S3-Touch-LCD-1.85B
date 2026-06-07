#include "pet_storage.h"
#include "pet_sprites.h"
#include "debug_trace.h"
#include <Preferences.h>

static const char* NAMESPACE = "buddy";

bool pet_has_saved(void)
{
    TRACE_STORAGE_ENTER();
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, true)) {
        TRACE_STORAGE("prefs.begin failed");
        return false;
    }
    bool has = prefs.isKey("species");
    prefs.end();
    TRACE_STORAGE("has_saved=%d", has);
    return has;
}

bool pet_load(void)
{
    TRACE_STORAGE_ENTER();
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, true)) {
        TRACE_STORAGE("prefs.begin failed");
        return false;
    }
    if (!prefs.isKey("species")) {
        prefs.end();
        TRACE_STORAGE("no species key found");
        return false;
    }

    g_pet.species = (PetSpecies)prefs.getUChar("species", 0);
    g_pet.eye     = (PetEye)prefs.getUChar("eye", 0);
    g_pet.hat     = (PetHat)prefs.getUChar("hat", 0);
    g_pet.color   = (PetColor)prefs.getUChar("color", 0);
    g_pet.rarity  = (PetRarity)prefs.getUChar("rarity", 0);
    g_pet.shiny   = prefs.getBool("shiny", false) ? 1 : 0;
    g_pet.hunger  = prefs.getUChar("hunger", 0);
    g_pet.joy     = prefs.getUChar("joy", 0);

    g_pet.stats[0] = prefs.getUChar("stat0", 0);
    g_pet.stats[1] = prefs.getUChar("stat1", 0);
    g_pet.stats[2] = prefs.getUChar("stat2", 0);
    g_pet.stats[3] = prefs.getUChar("stat3", 0);
    g_pet.stats[4] = prefs.getUChar("stat4", 0);

    String name = prefs.getString("name", "");
    strncpy(g_pet.name, name.c_str(), sizeof(g_pet.name) - 1);
    g_pet.name[sizeof(g_pet.name) - 1] = '\0';

    prefs.end();
    TRACE_STORAGE("loaded species=%d eye=%d hat=%d color=%d hunger=%d joy=%d",
                  g_pet.species, g_pet.eye, g_pet.hat, g_pet.color, g_pet.hunger, g_pet.joy);
    TRACE_STORAGE_EXIT();
    return true;
}

void pet_save(void)
{
    TRACE_STORAGE_ENTER();
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, false)) {
        TRACE_STORAGE("prefs.begin failed");
        return;
    }

    prefs.putUChar("species", (uint8_t)g_pet.species);
    prefs.putUChar("eye",     (uint8_t)g_pet.eye);
    prefs.putUChar("hat",     (uint8_t)g_pet.hat);
    prefs.putUChar("color",   (uint8_t)g_pet.color);
    prefs.putUChar("rarity",  (uint8_t)g_pet.rarity);
    prefs.putBool("shiny",    g_pet.shiny != 0);
    prefs.putUChar("hunger",  g_pet.hunger);
    prefs.putUChar("joy",     g_pet.joy);

    prefs.putUChar("stat0", g_pet.stats[0]);
    prefs.putUChar("stat1", g_pet.stats[1]);
    prefs.putUChar("stat2", g_pet.stats[2]);
    prefs.putUChar("stat3", g_pet.stats[3]);
    prefs.putUChar("stat4", g_pet.stats[4]);

    prefs.putString("name", String(g_pet.name));

    prefs.end();
    TRACE_STORAGE("saved species=%d eye=%d hat=%d color=%d hunger=%d joy=%d",
                  g_pet.species, g_pet.eye, g_pet.hat, g_pet.color, g_pet.hunger, g_pet.joy);
    TRACE_STORAGE_EXIT();
}
