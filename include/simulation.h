#pragma once

#include "world.h"
#include "creature.h"
#include "history.h"
#include "settings.h"

typedef struct {
    World    world;
    Creature creatures[MAX_CREATURES];
    int      creatureCount;
    int      nextId;
    int      totalDeaths;
    int      totalBirths;
    History  history;
} Simulation;

void SimulationInit(Simulation *s);
void SimulationUpdate(Simulation *s, float dt, const SimSettings *settings);
void SimulationDraw(const Simulation *s);
int  SimulationAliveCount(const Simulation *s);
