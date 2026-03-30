#include "world.h"

#include <assert.h>
#include <string.h>

/* ── Internal helpers ────────────────────────────────────────── */

/* Spawn one food item into the first available (eaten) slot.
   Returns true if a slot was found, false if the pool is full. */
static bool SpawnFood(World *w) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (w->plants[i].eaten) {
            w->plants[i].position  = (Vector2){
                (float)GetRandomValue(8, w->width  - 8),
                (float)GetRandomValue(8, w->height - 8)
            };
            w->plants[i].nutrition = FOOD_NUTRITION;
            w->plants[i].eaten     = false;
            return true;
        }
    }
    return false;  /* pool full */
}

/* ── Public API ──────────────────────────────────────────────── */

void WorldInit(World *w) {
    assert(w != NULL);
    memset(w, 0, sizeof(*w));

    w->width      = WORLD_WIDTH;
    w->height     = WORLD_HEIGHT;
    w->foodTarget    = FOOD_TARGET;
    w->foodSpawnRate = FOOD_SPAWN_RATE;

    /* Mark all slots as available */
    for (int i = 0; i < MAX_FOOD; i++) {
        w->plants[i].eaten = true;
    }

    /* Seed with full target amount */
    for (int i = 0; i < w->foodTarget; i++) {
        SpawnFood(w);
    }
}

void WorldUpdate(World *w, float dt) {
    assert(w != NULL);

    /* Rate-limited spawn, capped at w->foodTarget */
    if (WorldFoodCount(w) < w->foodTarget) {
        w->foodSpawnTimer += dt;
        float interval = (w->foodSpawnRate > 0.0f) ? 1.0f / w->foodSpawnRate : 9999.0f;
        while (w->foodSpawnTimer >= interval && WorldFoodCount(w) < w->foodTarget) {
            SpawnFood(w);
            w->foodSpawnTimer -= interval;
        }
    } else {
        w->foodSpawnTimer = 0.0f;  /* don't accumulate while at cap */
    }

    w->tick++;
}

int WorldFoodCount(const World *w) {
    assert(w != NULL);
    int count = 0;
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!w->plants[i].eaten) count++;
    }
    return count;
}

void WorldDraw(const World *w) {
    assert(w != NULL);

    /* World background */
    DrawRectangle(0, 0, w->width, w->height, (Color){ 18, 38, 18, 255 });

    /* Grid — 200px cells for the larger world */
    const int gridStep = 200;
    const Color gridColor = (Color){ 30, 55, 30, 255 };
    for (int x = 0; x < w->width; x += gridStep)
        DrawLine(x, 0, x, w->height, gridColor);
    for (int y = 0; y < w->height; y += gridStep)
        DrawLine(0, y, w->width, y, gridColor);

    /* World border — bright so it's visible when zoomed out */
    DrawRectangleLines(0, 0, w->width, w->height, (Color){ 80, 160, 80, 255 });

    /* Food items */
    const float half = FOOD_SIZE * 0.5f;
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!w->plants[i].eaten) {
            DrawRectangleV(
                (Vector2){ w->plants[i].position.x - half,
                           w->plants[i].position.y - half },
                (Vector2){ FOOD_SIZE, FOOD_SIZE },
                (Color){ 80, 200, 80, 220 }
            );
        }
    }
}
