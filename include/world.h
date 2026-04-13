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

    /* Spatial hash grid — maintained incrementally on spawn/eat.
       Linked-list buckets: head[cell] = first food index, -1 = empty. */
    int  foodGridHead[GRID_CELL_COUNT]; /* per-cell list head (-1 = empty)  */
    int  foodGridNext[MAX_FOOD];        /* next food in same cell (-1 = end) */
    int  foodGridCell[MAX_FOOD];        /* which cell food[i] belongs to     */
    int  foodCount;                     /* cached count of uneaten food      */
} World;

/* Initialize world and populate up to FOOD_TARGET food items */
void WorldInit(World *w);

/* Advance world: top up food to w->foodTarget, increment tick */
void WorldUpdate(World *w, float dt);

/* Return cached count of currently active (uneaten) food items — O(1) */
int WorldFoodCount(const World *w);

/* Remove a food item from the spatial grid (call before marking eaten).
   Also decrements w->foodCount. */
void WorldFoodGridRemove(World *w, int foodIdx);

/* Draw world background and all food items */
void WorldDraw(const World *w);
