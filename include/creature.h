#pragma once

#include "raylib.h"
#include "config.h"
#include "genome.h"

typedef struct {
    int     id;
    Vector2 position;
    Vector2 velocity;
    float   energy;
    float   maxEnergy;
    float   age;          /* seconds alive */
    /* Traits — cached from genome at init for fast physics */
    float   speed;        /* max speed px/s (= genome.speed) */
    float   vision;       /* detection radius px (= genome.vision) */
    float   visionAngle;  /* half-angle of FOV cone radians (= genome.visionAngle) */
    float   metabolism;   /* base energy drain per second (= genome.metabolism) */
    float   size;         /* body radius px (= genome.size) */
    /* genome.lifespan is used directly — no separate lifespan field */
    /* Genome and NN state */
    Genome  genome;
    float   nnInputs[NN_INPUTS];          /* last NN input values, for visualization */
    float   hiddenOut[NN_HIDDEN_MAX];     /* last NN hidden activations, for visualization */
    float   nnOutputs[NN_OUTPUTS];        /* last NN output values, for visualization */
    /* Internal state */
    float   facing;               /* current facing angle in radians */
    float   reproductionCooldown; /* seconds remaining before can reproduce again */
    bool    alive;
} Creature;

/* Initialize creature at position with traits derived from genome */
void CreatureInit(Creature *c, int id, Vector2 pos, const Genome *genome);

/* Apply NN outputs to velocity, wrap toroidally, drain energy, age */
void CreatureUpdate(Creature *c, float dt, int worldW, int worldH);

/* Draw body circle, direction line, FOV cone, energy indicator */
void CreatureDraw(const Creature *c);
