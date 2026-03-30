#include "simulation.h"

#include <math.h>
#include "raymath.h"
#include <assert.h>
#include <string.h>

/* ── Public API ──────────────────────────────────────────────── */

void SimulationInit(Simulation *s) {
    assert(s != NULL);

    WorldInit(&s->world);

    s->creatureCount = 0;
    s->nextId        = 0;
    s->totalDeaths   = 0;
    s->totalBirths   = 0;

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
    }
}

void SimulationUpdate(Simulation *s, float dt, const SimSettings *settings) {
    assert(s != NULL);
    assert(settings != NULL);

    /* Apply runtime food settings */
    s->world.foodTarget    = settings->foodTarget;
    s->world.foodSpawnRate = settings->foodSpawnRate;

    WorldUpdate(&s->world, dt);

    /* ── Toroidal helpers (local to this function) ──────────────────── */
#define TORUS_DELTA(val, dim) \
    ((val) >  (dim)*0.5f ? (val)-(dim) : (val) < -(dim)*0.5f ? (val)+(dim) : (val))

    /* Gather NN inputs and evaluate the network for each alive creature */
    for (int i = 0; i < s->creatureCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive) continue;

        float inputs[NN_INPUTS];

        /* ── Food sensor ───────────────────────────────────────── */
        int   bestFoodIdx  = -1;
        float bestFoodDist = c->vision;  /* only within vision radius */

        for (int f = 0; f < MAX_FOOD; f++) {
            if (s->world.plants[f].eaten) continue;
            float dx = TORUS_DELTA(s->world.plants[f].position.x - c->position.x, s->world.width);
            float dy = TORUS_DELTA(s->world.plants[f].position.y - c->position.y, s->world.height);
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist >= bestFoodDist) continue;
            /* Angular check: must be within the creature's FOV cone */
            float angle = atan2f(dy, dx) - c->facing;
            while (angle >  PI) angle -= 2.0f * PI;
            while (angle < -PI) angle += 2.0f * PI;
            if (fabsf(angle) > c->visionAngle) continue;
            bestFoodDist = dist;
            bestFoodIdx  = f;
        }

        if (bestFoodIdx >= 0) {
            inputs[0] = bestFoodDist / c->vision;
            float dx = TORUS_DELTA(s->world.plants[bestFoodIdx].position.x - c->position.x, s->world.width);
            float dy = TORUS_DELTA(s->world.plants[bestFoodIdx].position.y - c->position.y, s->world.height);
            float relAngle = atan2f(dy, dx) - c->facing;
            inputs[1] = sinf(relAngle);
            inputs[2] = cosf(relAngle);
        } else {
            inputs[0] = 1.0f;
            inputs[1] = 0.0f;
            inputs[2] = 0.0f;
        }

        /* ── Nearest other creature sensor ─────────────────────── */
        int   bestCreatureIdx  = -1;
        float bestCreatureDist = c->vision;

        for (int j = 0; j < s->creatureCount; j++) {
            if (j == i) continue;
            if (!s->creatures[j].alive) continue;
            float dx = TORUS_DELTA(s->creatures[j].position.x - c->position.x, s->world.width);
            float dy = TORUS_DELTA(s->creatures[j].position.y - c->position.y, s->world.height);
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist >= bestCreatureDist) continue;
            float angle = atan2f(dy, dx) - c->facing;
            while (angle >  PI) angle -= 2.0f * PI;
            while (angle < -PI) angle += 2.0f * PI;
            if (fabsf(angle) > c->visionAngle) continue;
            bestCreatureDist = dist;
            bestCreatureIdx  = j;
        }

        if (bestCreatureIdx >= 0) {
            inputs[3] = bestCreatureDist / c->vision;
            float dx = TORUS_DELTA(s->creatures[bestCreatureIdx].position.x - c->position.x, s->world.width);
            float dy = TORUS_DELTA(s->creatures[bestCreatureIdx].position.y - c->position.y, s->world.height);
            float relAngle = atan2f(dy, dx) - c->facing;
            inputs[4] = sinf(relAngle);
        } else {
            inputs[3] = 1.0f;
            inputs[4] = 0.0f;
        }

        /* ── Energy and bias ───────────────────────────────────── */
        inputs[5] = c->energy / c->maxEnergy;  /* energy_norm: [0,1] */
        inputs[6] = 1.0f;                       /* bias */

        /* Store inputs on creature for visualization */
        for (int ii = 0; ii < NN_INPUTS; ii++) c->nnInputs[ii] = inputs[ii];

        /* ── Evaluate neural network ───────────────────────────── */
        GenomeEvalNN(&c->genome, inputs, c->hiddenOut, c->nnOutputs);
    }

    /* Update creature physics, energy, aging (uses nnOutputs set above) */
    for (int i = 0; i < s->creatureCount; i++) {
        CreatureUpdate(&s->creatures[i], dt, s->world.width, s->world.height);
    }

    /* Check eating: alive creature within (size + FOOD_SIZE) of uneaten food */
    for (int i = 0; i < s->creatureCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive) continue;

        float eatRadius = c->size + FOOD_SIZE;

        for (int f = 0; f < MAX_FOOD; f++) {
            if (s->world.plants[f].eaten) continue;

            float dist = Vector2Distance(c->position, s->world.plants[f].position);
            if (dist < eatRadius) {
                s->world.plants[f].eaten = true;
                c->energy += s->world.plants[f].nutrition;
                if (c->energy > c->maxEnergy) c->energy = c->maxEnergy;
                break;  /* one food per creature per tick */
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
        if (SimulationAliveCount(s) >= MAX_CREATURES) break;

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
    }

    /* Population floor: respawn random creatures if alive count drops below slider value */
    while (SimulationAliveCount(s) < settings->minPopulation && s->creatureCount < MAX_CREATURES) {
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
    }

    /* Count newly dead creatures this tick (age sentinel -1 = already counted) */
    for (int i = 0; i < s->creatureCount; i++) {
        Creature *c = &s->creatures[i];
        if (!c->alive && c->age >= 0.0f) {
            s->totalDeaths++;
            c->age = -1.0f;  /* sentinel: already counted */
        }
    }

    /* Record history sample every HISTORY_SAMPLE_TICKS ticks */
    if (s->world.tick % HISTORY_SAMPLE_TICKS == 0) {
        int   aliveCount  = SimulationAliveCount(s);
        float sumSpeed    = 0.0f;
        float sumMeta     = 0.0f;
        int   counted     = 0;

        for (int i = 0; i < s->creatureCount; i++) {
            if (!s->creatures[i].alive) continue;
            sumSpeed += s->creatures[i].speed;
            sumMeta  += s->creatures[i].metabolism;
            counted++;
        }

        float avgSpeed = (counted > 0) ? sumSpeed / counted : 0.0f;
        float avgMeta  = (counted > 0) ? sumMeta  / counted : 0.0f;

        HistoryRecord(&s->history, aliveCount, WorldFoodCount(&s->world), avgSpeed, avgMeta);
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

    int count = 0;
    for (int i = 0; i < s->creatureCount; i++) {
        if (s->creatures[i].alive) count++;
    }
    return count;
}
