#pragma once

#include <stdbool.h>

/* Runtime-adjustable simulation parameters exposed via the UI panel */
typedef struct {
    bool  paused;
    int   speedMult;      /* 1..20 sim steps per frame */
    int   foodTarget;     /* runtime food cap */
    float foodSpawnRate;  /* food items spawned per second */
    float mutRateMult;    /* multiplier on genome mutation rate */
    int   minPopulation;  /* respawn floor: keep at least this many creatures alive */
} SimSettings;

/* Fill *s with safe defaults */
void SimSettingsDefault(SimSettings *s);
