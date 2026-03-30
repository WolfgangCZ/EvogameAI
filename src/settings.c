#include "settings.h"
#include "config.h"

/* Initialize settings to sensible defaults */
void SimSettingsDefault(SimSettings *s) {
    s->paused        = false;
    s->speedMult     = 1;
    s->foodTarget    = FOOD_TARGET;
    s->foodSpawnRate = FOOD_SPAWN_RATE;
    s->mutRateMult   = 1.0f;
    s->minPopulation = 50;
}
