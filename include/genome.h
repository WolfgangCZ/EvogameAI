#pragma once

#include "config.h"
#include <stdbool.h>

/* ── Activation function enum ────────────────────────────────── */
typedef enum {
    ACT_LINEAR = 0,
    ACT_RELU,
    ACT_SIGMOID,
    ACT_TANH,
    ACT_SIN,
    ACT_COS,
    ACT_ABS,
    ACT_STEP,
    ACT_COUNT
} ActivationFunc;

/* ── Connection between two nodes in the neural network ──────── */
typedef struct {
    int   from;    /* source node index (global: 0..NN_NODE_COUNT-1) */
    int   to;      /* destination node index */
    float weight;  /* connection weight [-4, 4] */
} NNConn;

/* ── Genome — physical traits + dynamic NN topology ─────────── */
typedef struct {
    /* Physical traits (stored directly, not as gene indices) */
    float          size;         /* body radius px [3, 12] */
    float          speed;        /* max speed px/s [20, 120] */
    float          vision;       /* detection radius px [40, 200] */
    float          visionAngle;  /* half-angle of FOV cone radians [0.01, PI] */
    float          metabolism;   /* base energy drain per second [1, 8] */
    float          lifespan;     /* max age in seconds [60, 600] */
    float          mutationRate; /* per-trait mutation probability [0.005, 0.5] */

    /* Network topology */
    int            hiddenCount;                 /* 0..NN_HIDDEN_MAX active hidden nodes */
    ActivationFunc hiddenAct[NN_HIDDEN_MAX];   /* activation per hidden slot */
    int            connCount;                   /* 0..NN_CONN_MAX active connections */
    NNConn         conns[NN_CONN_MAX];          /* connection list */
} Genome;

/* Initialize genome with random traits and sparse input→output connections (hiddenCount=0) */
void GenomeRandom(Genome *g);

/* Create offspring genome from parent a with mutation applied.
   b is accepted for API compatibility but ignored (asexual reproduction). */
void GenomeCrossover(const Genome *a, const Genome *b, Genome *child, float mutationRate);

/* Evaluate neural network given inputs; fills hidden_out and outputs arrays.
   hidden_out must be NN_HIDDEN_MAX elements; only [0..hiddenCount-1] are written. */
void GenomeEvalNN(const Genome *g, const float inputs[NN_INPUTS],
                  float hidden_out[NN_HIDDEN_MAX], float outputs[NN_OUTPUTS]);

/* Return human-readable name string for an activation function */
const char *ActivationFuncName(ActivationFunc f);

/* ── Genome serialization ────────────────────────────────────── */

/* Maximum hex string length (null-terminated) for a fully packed genome */
#define GENOME_HEX_MAX  580

/* Encode genome to an uppercase hex string.
   buf must be at least GENOME_HEX_MAX bytes. */
void GenomeEncode(const Genome *g, char *buf, int bufSize);

/* Decode a hex string produced by GenomeEncode back into a genome.
   Returns true on success, false if the string is malformed. */
bool GenomeDecode(const char *hex, Genome *g);
