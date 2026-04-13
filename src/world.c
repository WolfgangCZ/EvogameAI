#include "world.h"

#include <assert.h>
#include <string.h>

/* ── Internal helpers ────────────────────────────────────────── */

/* Compute the flat grid cell index for a world position. */
static inline int FoodCellOf(float x, float y) {
    int col = (int)(x / GRID_CELL_SIZE) % GRID_COLS;
    int row = (int)(y / GRID_CELL_SIZE) % GRID_ROWS;
    return row * GRID_COLS + col;
}

/* Insert food slot i into the spatial grid.
   The plant's position and eaten=false must already be set. */
static void FoodGridInsert(World *w, int i) {
    int cell           = FoodCellOf(w->plants[i].position.x, w->plants[i].position.y);
    w->foodGridCell[i] = cell;
    w->foodGridNext[i] = w->foodGridHead[cell];
    w->foodGridHead[cell] = i;
    w->foodCount++;
}

/* Spawn one food item into the first available (eaten) slot.
   Returns true if a slot was found, false if the pool is full. */
static bool SpawnFood(World *w) {
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!w->plants[i].eaten) continue;
        w->plants[i].position  = (Vector2){
            (float)GetRandomValue(8, w->width  - 8),
            (float)GetRandomValue(8, w->height - 8)
        };
        w->plants[i].nutrition = FOOD_NUTRITION;
        w->plants[i].eaten     = false;
        FoodGridInsert(w, i);
        return true;
    }
    return false;  /* pool full */
}

/* ── Public API ──────────────────────────────────────────────── */

void WorldInit(World *w) {
    assert(w != NULL);
    memset(w, 0, sizeof(*w));

    w->width         = WORLD_WIDTH;
    w->height        = WORLD_HEIGHT;
    w->foodTarget    = FOOD_TARGET;
    w->foodSpawnRate = FOOD_SPAWN_RATE;
    w->foodCount     = 0;

    /* Initialize spatial grid heads to -1 (empty linked lists) */
    memset(w->foodGridHead, -1, sizeof(w->foodGridHead));
    memset(w->foodGridNext, -1, sizeof(w->foodGridNext));

    /* Mark all slots as available */
    for (int i = 0; i < MAX_FOOD; i++) w->plants[i].eaten = true;

    /* Seed with full target amount */
    for (int i = 0; i < w->foodTarget; i++) SpawnFood(w);
}

void WorldUpdate(World *w, float dt) {
    assert(w != NULL);

    /* Rate-limited spawn, capped at w->foodTarget */
    if (w->foodCount < w->foodTarget) {
        w->foodSpawnTimer += dt;
        float interval = (w->foodSpawnRate > 0.0f) ? 1.0f / w->foodSpawnRate : 9999.0f;
        while (w->foodSpawnTimer >= interval && w->foodCount < w->foodTarget) {
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
    return w->foodCount;  /* O(1) — maintained incrementally */
}

/* Remove food item foodIdx from the spatial grid and decrement foodCount.
   Must be called before marking the plant as eaten. */
void WorldFoodGridRemove(World *w, int foodIdx) {
    assert(w != NULL);
    assert(foodIdx >= 0 && foodIdx < MAX_FOOD);

    int cell = w->foodGridCell[foodIdx];

    if (w->foodGridHead[cell] == foodIdx) {
        w->foodGridHead[cell] = w->foodGridNext[foodIdx];
    } else {
        int prev = w->foodGridHead[cell];
        while (prev != -1 && w->foodGridNext[prev] != foodIdx) {
            prev = w->foodGridNext[prev];
        }
        if (prev != -1) w->foodGridNext[prev] = w->foodGridNext[foodIdx];
    }
    w->foodGridNext[foodIdx] = -1;
    w->foodCount--;
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
