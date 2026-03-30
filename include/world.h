#pragma once

#include "raylib.h"
#include "config.h"

/* A single plant-food item in the world */
typedef struct {
    Vector2 position;
    float   nutrition;
    bool    eaten;
} Food;

/* Global environment state */
typedef struct {
    int   width;
    int   height;
    Food  plants[MAX_FOOD];
    float foodSpawnTimer;
    int   tick;
    int   foodTarget;    /* runtime-adjustable food cap */
    float foodSpawnRate; /* runtime-adjustable spawn rate (items/sec) */
} World;

/* Initialize world and populate up to FOOD_TARGET food items */
void WorldInit(World *w);

/* Advance world: top up food to w->foodTarget, increment tick */
void WorldUpdate(World *w, float dt);

/* Count currently active (uneaten) food items */
int WorldFoodCount(const World *w);

/* Draw world background and all food items */
void WorldDraw(const World *w);
