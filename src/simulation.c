#include "simulation.h"

#include <math.h>
#include "raymath.h"
#include <assert.h>
#include <string.h>

/* ── Creature spatial grid (rebuilt every frame) ─────────────── */
/* Static storage avoids large stack frames and heap allocation.  */
static int s_crGridHead[GRID_CELL_COUNT];  /* per-cell list head (-1 = empty) */
static int s_crGridNext[MAX_CREATURES];    /* next creature in same cell       */

/* ── Toroidal distance helper ────────────────────────────────── */
#define TORUS_DELTA(val, dim) \
    ((val) >  (dim)*0.5f ? (val)-(dim) : (val) < -(dim)*0.5f ? (val)+(dim) : (val))

/* Grid cell index for a position known to be in [0, WORLD_W/H). */
static inline int CrCell(float x, float y) {
    int col = (int)(x / GRID_CELL_SIZE) % GRID_COLS;
    int row = (int)(y / GRID_CELL_SIZE) % GRID_ROWS;
    return row * GRID_COLS + col;
}

/* Wrap a raw column/row index (may be negative or >= limit). */
static inline int WrapCol(int c) { return ((c % GRID_COLS) + GRID_COLS) % GRID_COLS; }
static inline int WrapRow(int r) { return ((r % GRID_ROWS) + GRID_ROWS) % GRID_ROWS; }

/* ── Public API ──────────────────────────────────────────────── */

void SimulationInit(Simulation *s) {
    assert(s != NULL);

    WorldInit(&s->world);

    s->creatureCount = 0;
    s->nextId        = 0;
    s->totalDeaths   = 0;
    s->totalBirths   = 0;
    s->aliveCount    = 0;

    /* Zero out history ring buffer */
    memset(&s->history, 0, sizeof(s->history));

    /* Spawn initial creatures at random positions with random genomes */
    for (int i = 0; i < INITIAL_CREATURES && i < MAX_CREATURES; i++) {
        Vector2 pos = {
            (float)GetRandomValue((int)CREATURE_SIZE, s->world.width  - (int)CREATURE_SIZE),
            (float)GetRandomValue((int)CREATURE_SIZE, s->world.height - (int)CREATURE_SIZE)
        };
        Genome genome;
        GenomeRandom(&genome);
        CreatureInit(&s->creatures[i], s->nextId++, pos, &genome);
        s->creatureCount++;
        s->aliveCount++;
    }
}

void SimulationUpdate(Simulation *s, float dt, const SimSettings *settings) {
    assert(s != NULL);
    assert(settings != NULL);

    /* Apply runtime food settings */
    s->world.foodTarget    = settings->foodTarget;
    s->world.foodSpawnRate = settings->foodSpawnRate;

    WorldUpdate(&s->world, dt);

    /* ── Build creature spatial grid ──────────────────────── */
    memset(s_crGridHead, -1, sizeof(s_crGridHead));
    for (int i = 0; i < s->creatureCount; i++) {
        if (!s->creatures[i].alive) continue;
        int cell = CrCell(s->creatures[i].position.x, s->creatures[i].position.y);
        s_crGridNext[i]    = s_crGridHead[cell];
        s_crGridHead[cell] = i;
    }

    /* ── Sense environment + evaluate NN for each alive creature ── */
    for (int i = 0; i < s->creatureCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive) continue;

        float inputs[NN_INPUTS];

        /* Precompute vision radius squared and cell range once */
        float visionSq = c->vision * c->vision;
        int minCol = (int)((c->position.x - c->vision) / GRID_CELL_SIZE);
        int maxCol = (int)((c->position.x + c->vision) / GRID_CELL_SIZE);
        int minRow = (int)((c->position.y - c->vision) / GRID_CELL_SIZE);
        int maxRow = (int)((c->position.y + c->vision) / GRID_CELL_SIZE);

        /* ── Food sensor ───────────────────────────────────── */
        int   bestFoodIdx    = -1;
        float bestFoodDistSq = visionSq;  /* only within vision radius */

        for (int gr = minRow; gr <= maxRow; gr++) {
            int row = WrapRow(gr);
            for (int gc = minCol; gc <= maxCol; gc++) {
                int col  = WrapCol(gc);
                int cell = row * GRID_COLS + col;
                for (int f = s->world.foodGridHead[cell]; f != -1;
                         f = s->world.foodGridNext[f]) {
                    float dx = TORUS_DELTA(s->world.plants[f].position.x - c->position.x,
                                          s->world.width);
                    float dy = TORUS_DELTA(s->world.plants[f].position.y - c->position.y,
                                          s->world.height);
                    float dSq = dx*dx + dy*dy;
                    if (dSq >= bestFoodDistSq) continue;
                    /* Angular check: must be within creature's FOV cone */
                    float angle = atan2f(dy, dx) - c->facing;
                    while (angle >  PI) angle -= 2.0f * PI;
                    while (angle < -PI) angle += 2.0f * PI;
                    if (fabsf(angle) > c->visionAngle) continue;
                    bestFoodDistSq = dSq;
                    bestFoodIdx    = f;
                }
            }
        }

        if (bestFoodIdx >= 0) {
            inputs[0] = sqrtf(bestFoodDistSq) / c->vision;
            float dx = TORUS_DELTA(s->world.plants[bestFoodIdx].position.x - c->position.x,
                                   s->world.width);
            float dy = TORUS_DELTA(s->world.plants[bestFoodIdx].position.y - c->position.y,
                                   s->world.height);
            float relAngle = atan2f(dy, dx) - c->facing;
            inputs[1] = sinf(relAngle);
            inputs[2] = cosf(relAngle);
        } else {
            inputs[0] = 1.0f;
            inputs[1] = 0.0f;
            inputs[2] = 0.0f;
        }

        /* ── Nearest other creature sensor ─────────────────── */
        int   bestCIdx    = -1;
        float bestCDistSq = visionSq;

        for (int gr = minRow; gr <= maxRow; gr++) {
            int row = WrapRow(gr);
            for (int gc = minCol; gc <= maxCol; gc++) {
                int col  = WrapCol(gc);
                int cell = row * GRID_COLS + col;
                for (int j = s_crGridHead[cell]; j != -1; j = s_crGridNext[j]) {
                    if (j == i) continue;
                    float dx = TORUS_DELTA(s->creatures[j].position.x - c->position.x,
                                          s->world.width);
                    float dy = TORUS_DELTA(s->creatures[j].position.y - c->position.y,
                                          s->world.height);
                    float dSq = dx*dx + dy*dy;
                    if (dSq >= bestCDistSq) continue;
                    float angle = atan2f(dy, dx) - c->facing;
                    while (angle >  PI) angle -= 2.0f * PI;
                    while (angle < -PI) angle += 2.0f * PI;
                    if (fabsf(angle) > c->visionAngle) continue;
                    bestCDistSq = dSq;
                    bestCIdx    = j;
                }
            }
        }

        if (bestCIdx >= 0) {
            inputs[3] = sqrtf(bestCDistSq) / c->vision;
            float dx = TORUS_DELTA(s->creatures[bestCIdx].position.x - c->position.x,
                                   s->world.width);
            float dy = TORUS_DELTA(s->creatures[bestCIdx].position.y - c->position.y,
                                   s->world.height);
            float relAngle = atan2f(dy, dx) - c->facing;
            inputs[4] = sinf(relAngle);
        } else {
            inputs[3] = 1.0f;
            inputs[4] = 0.0f;
        }

        /* ── Energy and bias ───────────────────────────────── */
        inputs[5] = c->energy / c->maxEnergy;  /* energy_norm: [0,1] */
        inputs[6] = 1.0f;                       /* bias */

        /* Store inputs on creature for visualization */
        for (int ii = 0; ii < NN_INPUTS; ii++) c->nnInputs[ii] = inputs[ii];

        /* ── Evaluate neural network ───────────────────────── */
        GenomeEvalNN(&c->genome, inputs, c->hiddenOut, c->nnOutputs);
    }

    /* Update creature physics, energy, aging (uses nnOutputs set above) */
    for (int i = 0; i < s->creatureCount; i++) {
        CreatureUpdate(&s->creatures[i], dt, s->world.width, s->world.height);
    }

    /* Detect deaths (before eating/reproduction so aliveCount is accurate) */
    for (int i = 0; i < s->creatureCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive && c->age >= 0.0f) {
            s->totalDeaths++;
            s->aliveCount--;
            c->age = -1.0f;  /* sentinel: already counted */
        }
    }

    /* Check eating: use food grid for fast proximity query.
       eatRadius (max ~17 px) << GRID_CELL_SIZE (200 px) so a 3×3 cell
       neighbourhood is always sufficient — no food can be missed. */
    for (int i = 0; i < s->creatureCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive) continue;

        float eatRadius = c->size + FOOD_SIZE;
        float eatRadSq  = eatRadius * eatRadius;
        int   crCol     = (int)(c->position.x / GRID_CELL_SIZE) % GRID_COLS;
        int   crRow     = (int)(c->position.y / GRID_CELL_SIZE) % GRID_ROWS;

        bool ate = false;
        for (int gr = crRow - 1; gr <= crRow + 1 && !ate; gr++) {
            int row = WrapRow(gr);
            for (int gc = crCol - 1; gc <= crCol + 1 && !ate; gc++) {
                int col  = WrapCol(gc);
                int cell = row * GRID_COLS + col;
                int f    = s->world.foodGridHead[cell];
                while (f != -1 && !ate) {
                    int nextF = s->world.foodGridNext[f];  /* save before removal */
                    float dx = TORUS_DELTA(s->world.plants[f].position.x - c->position.x,
                                          s->world.width);
                    float dy = TORUS_DELTA(s->world.plants[f].position.y - c->position.y,
                                          s->world.height);
                    if (dx*dx + dy*dy < eatRadSq) {
                        float nutrition = s->world.plants[f].nutrition;
                        WorldFoodGridRemove(&s->world, f);
                        s->world.plants[f].eaten = true;
                        c->energy += nutrition;
                        if (c->energy > c->maxEnergy) c->energy = c->maxEnergy;
                        ate = true;
                    }
                    f = nextF;
                }
            }
        }
    }

    /* Reproduce: NN output[2] above threshold + enough energy + no cooldown */
    int currentCount = s->creatureCount;  /* snapshot so new births don't trigger again */
    for (int i = 0; i < currentCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive) continue;
        if (c->reproductionCooldown > 0.0f) continue;
        if (c->nnOutputs[2] <= REPRODUCE_NN_THRESHOLD) continue;
        if (c->energy < REPRODUCE_MIN_ENERGY) continue;
        if (s->aliveCount >= MAX_CREATURES) break;

        /* Find a free slot: dead slot first, then extend array */
        int slot = -1;
        for (int j = 0; j < s->creatureCount; j++) {
            if (!s->creatures[j].alive && s->creatures[j].age < 0.0f) {
                slot = j;
                break;
            }
        }
        if (slot < 0 && s->creatureCount < MAX_CREATURES) {
            slot = s->creatureCount++;
        }
        if (slot < 0) continue;

        /* Spawn child near parent */
        float ox = (float)GetRandomValue(-20, 20);
        float oy = (float)GetRandomValue(-20, 20);
        Vector2 childPos = {
            Clamp(c->position.x + ox, c->size, (float)s->world.width  - c->size),
            Clamp(c->position.y + oy, c->size, (float)s->world.height - c->size)
        };

        /* Asexual reproduction: self-crossover with mutation, scaled by mutRateMult */
        Genome childGenome;
        float mutRate = c->genome.mutationRate * settings->mutRateMult;
        GenomeCrossover(&c->genome, &c->genome, &childGenome, mutRate);
        CreatureInit(&s->creatures[slot], s->nextId++, childPos, &childGenome);

        /* Transfer energy */
        s->creatures[slot].energy = c->energy * REPRODUCE_ENERGY_COST;
        c->energy *= (1.0f - REPRODUCE_ENERGY_COST);
        c->reproductionCooldown = REPRODUCE_COOLDOWN;
        s->totalBirths++;
        s->aliveCount++;
    }

    /* Population floor: respawn random creatures if alive count drops below slider value */
    while (s->aliveCount < settings->minPopulation && s->creatureCount < MAX_CREATURES) {
        int slot = -1;
        for (int j = 0; j < s->creatureCount; j++) {
            if (!s->creatures[j].alive && s->creatures[j].age < 0.0f) { slot = j; break; }
        }
        if (slot < 0) slot = s->creatureCount++;

        Vector2 pos = {
            (float)GetRandomValue((int)CREATURE_SIZE, s->world.width  - (int)CREATURE_SIZE),
            (float)GetRandomValue((int)CREATURE_SIZE, s->world.height - (int)CREATURE_SIZE)
        };
        Genome genome;
        GenomeRandom(&genome);
        CreatureInit(&s->creatures[slot], s->nextId++, pos, &genome);
        s->aliveCount++;
    }

    /* Record history sample every HISTORY_SAMPLE_TICKS ticks */
    if (s->world.tick % HISTORY_SAMPLE_TICKS == 0) {
        float sumSpeed = 0.0f, sumMeta = 0.0f;
        int   counted  = 0;

        for (int i = 0; i < s->creatureCount; i++) {
            if (!s->creatures[i].alive) continue;
            sumSpeed += s->creatures[i].speed;
            sumMeta  += s->creatures[i].metabolism;
            counted++;
        }

        float avgSpeed = (counted > 0) ? sumSpeed / counted : 0.0f;
        float avgMeta  = (counted > 0) ? sumMeta  / counted : 0.0f;

        HistoryRecord(&s->history, s->aliveCount, s->world.foodCount, avgSpeed, avgMeta);
    }
}

void SimulationDraw(const Simulation *s) {
    assert(s != NULL);

    WorldDraw(&s->world);

    for (int i = 0; i < s->creatureCount; i++) {
        CreatureDraw(&s->creatures[i]);
    }
}

int SimulationAliveCount(const Simulation *s) {
    assert(s != NULL);
    return s->aliveCount;  /* O(1) — maintained incrementally */
}
