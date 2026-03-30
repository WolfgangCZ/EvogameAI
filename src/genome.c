#include "genome.h"

#include <math.h>
#include <assert.h>
#include "raylib.h"
#include "raymath.h"

/* ── Internal helpers ────────────────────────────────────────── */

/* Apply an activation function to value x */
static float ApplyActivation(ActivationFunc fn, float x) {
    switch (fn) {
        case ACT_LINEAR:  return x;
        case ACT_RELU:    return x > 0.0f ? x : 0.0f;
        case ACT_SIGMOID: return 1.0f / (1.0f + expf(-x));
        case ACT_TANH:    return tanhf(x);
        case ACT_SIN:     return sinf(x);
        case ACT_COS:     return cosf(x);
        case ACT_ABS:     return fabsf(x);
        case ACT_STEP:    return x >= 0.0f ? 1.0f : 0.0f;
        default:          return x;
    }
}

/* Uniform random float in [0, 1] */
static float RandFloat(void) {
    return (float)GetRandomValue(0, 10000) / 10000.0f;
}

/* ── Mutation helpers (internal) ─────────────────────────────── */

/* Perturb existing connection weights */
static void MutateWeights(Genome *g, float rate) {
    for (int c = 0; c < g->connCount; c++) {
        if (RandFloat() < rate) {
            g->conns[c].weight += (RandFloat() * 2.0f - 1.0f) * 0.5f;
            if (g->conns[c].weight >  4.0f) g->conns[c].weight =  4.0f;
            if (g->conns[c].weight < -4.0f) g->conns[c].weight = -4.0f;
        }
    }
}

/* Add a new connection between a valid source and target node */
static void MutateAddConnection(Genome *g) {
    if (g->connCount >= NN_CONN_MAX) return;

    /* from: any input (0..NN_INPUTS-1) or any active hidden node */
    int fromRange = NN_INPUTS + g->hiddenCount;
    int fromSlot  = GetRandomValue(0, fromRange - 1);
    int fromNode  = (fromSlot < NN_INPUTS) ? fromSlot
                                           : NN_NODE_HIDDEN_BASE + (fromSlot - NN_INPUTS);

    /* to: any active hidden node with index > fromNode (avoids backward cycles),
       or any output node */
    int targets[NN_HIDDEN_MAX + NN_OUTPUTS];
    int targetCount = 0;

    for (int h = 0; h < g->hiddenCount; h++) {
        int hidNode = NN_NODE_HIDDEN_BASE + h;
        if (hidNode > fromNode) targets[targetCount++] = hidNode;
    }
    for (int o = 0; o < NN_OUTPUTS; o++) targets[targetCount++] = NN_NODE_OUT_BASE + o;

    if (targetCount == 0) return;

    int toNode = targets[GetRandomValue(0, targetCount - 1)];

    g->conns[g->connCount].from   = fromNode;
    g->conns[g->connCount].to     = toNode;
    g->conns[g->connCount].weight = RandFloat() * 4.0f - 2.0f;
    g->connCount++;
}

/* Split an existing connection by inserting a new hidden node between the endpoints */
static void MutateAddHiddenNode(Genome *g) {
    if (g->hiddenCount >= NN_HIDDEN_MAX) return;
    if (g->connCount == 0) return;

    /* Pick a random connection to split */
    int    splitIdx = GetRandomValue(0, g->connCount - 1);
    NNConn old      = g->conns[splitIdx];

    int newNode = NN_NODE_HIDDEN_BASE + g->hiddenCount;

    /* Redirect split connection: old.from → newNode (weight 1) */
    g->conns[splitIdx].to     = newNode;
    g->conns[splitIdx].weight = 1.0f;

    /* Add second half: newNode → old.to (inherits old weight) */
    if (g->connCount < NN_CONN_MAX) {
        g->conns[g->connCount].from   = newNode;
        g->conns[g->connCount].to     = old.to;
        g->conns[g->connCount].weight = old.weight;
        g->connCount++;
    }

    /* Random activation for the new hidden node */
    g->hiddenAct[g->hiddenCount] = (ActivationFunc)GetRandomValue(0, ACT_COUNT - 1);
    g->hiddenCount++;
}

/* Randomize the activation function of a random existing hidden node */
static void MutateActivation(Genome *g) {
    if (g->hiddenCount == 0) return;
    int h = GetRandomValue(0, g->hiddenCount - 1);
    g->hiddenAct[h] = (ActivationFunc)GetRandomValue(0, ACT_COUNT - 1);
}

/* Perturb physical trait values within their valid ranges */
static void MutateTraits(Genome *g, float rate) {
    /* Capped traits — mutate within [min, max] */
    float *traits[] = { &g->size, &g->speed, &g->visionAngle, &g->metabolism, &g->lifespan };
    float  mins[]   = { 3.0f, 20.0f, 0.01f, 1.0f, 60.0f };
    float  maxs[]   = { 12.0f, 120.0f, PI,  8.0f, 600.0f };
    for (int i = 0; i < 5; i++) {
        if (RandFloat() < rate) {
            float range = maxs[i] - mins[i];
            *traits[i] += (RandFloat() * 2.0f - 1.0f) * range * 0.1f;
            if (*traits[i] < mins[i]) *traits[i] = mins[i];
            if (*traits[i] > maxs[i]) *traits[i] = maxs[i];
        }
    }

    /* Vision range — uncapped, naturally selected; minimum 10px */
    if (RandFloat() < rate) {
        g->vision += (RandFloat() * 2.0f - 1.0f) * 20.0f;
        if (g->vision < 10.0f) g->vision = 10.0f;
    }
    if (RandFloat() < rate) {
        g->mutationRate += (RandFloat() * 2.0f - 1.0f) * 0.05f;
        if (g->mutationRate < 0.005f) g->mutationRate = 0.005f;
        if (g->mutationRate > 0.5f)   g->mutationRate = 0.5f;
    }
}

/* ── Public API ──────────────────────────────────────────────── */

void GenomeRandom(Genome *g) {
    assert(g != NULL);

    /* Randomize physical traits within their natural ranges */
    g->size         = Lerp(3.0f,   12.0f,  RandFloat());
    g->speed        = Lerp(20.0f,  120.0f, RandFloat());
    g->vision       = Lerp(40.0f,  200.0f, RandFloat());
    g->visionAngle  = Lerp(0.01f,  PI,     RandFloat());
    g->metabolism   = Lerp(1.0f,   8.0f,   RandFloat());
    g->lifespan     = Lerp(60.0f,  600.0f, RandFloat());
    g->mutationRate = Lerp(0.01f,  0.3f,   RandFloat());

    /* Start with no hidden nodes */
    g->hiddenCount = 0;
    for (int h = 0; h < NN_HIDDEN_MAX; h++) g->hiddenAct[h] = ACT_LINEAR;

    /* Sparse input → output connections (40% probability each pair) */
    g->connCount = 0;
    for (int i = 0; i < NN_INPUTS; i++) {
        for (int o = 0; o < NN_OUTPUTS; o++) {
            if (RandFloat() < 0.4f && g->connCount < NN_CONN_MAX) {
                g->conns[g->connCount].from   = i;
                g->conns[g->connCount].to     = NN_NODE_OUT_BASE + o;
                g->conns[g->connCount].weight = RandFloat() * 4.0f - 2.0f;
                g->connCount++;
            }
        }
    }
}

void GenomeCrossover(const Genome *a, const Genome *b, Genome *child, float mutationRate) {
    assert(a != NULL && b != NULL && child != NULL);

    /* Start as an exact copy of parent a (asexual reproduction — b ignored for now) */
    *child = *a;
    (void)b;

    float structRate = mutationRate * 0.25f;

    MutateWeights(child, mutationRate);
    MutateTraits(child, mutationRate);
    if (RandFloat() < structRate)          MutateAddConnection(child);
    if (RandFloat() < structRate * 0.4f)   MutateAddHiddenNode(child);
    if (RandFloat() < structRate)          MutateActivation(child);
}

void GenomeEvalNN(const Genome *g, const float inputs[NN_INPUTS],
                  float hidden_out[NN_HIDDEN_MAX], float outputs[NN_OUTPUTS]) {
    assert(g != NULL && inputs != NULL && hidden_out != NULL && outputs != NULL);

    /* Node value array indexed by global node index */
    float nodeVals[NN_NODE_COUNT] = {0};

    /* Load inputs into node value slots */
    for (int i = 0; i < NN_INPUTS; i++) nodeVals[i] = inputs[i];

    /* Process hidden nodes in slot order (topologically sorted by construction) */
    for (int h = 0; h < g->hiddenCount; h++) {
        int   nodeIdx = NN_NODE_HIDDEN_BASE + h;
        float sum     = 0.0f;
        for (int c = 0; c < g->connCount; c++) {
            if (g->conns[c].to == nodeIdx)
                sum += nodeVals[g->conns[c].from] * g->conns[c].weight;
        }
        nodeVals[nodeIdx] = ApplyActivation(g->hiddenAct[h], sum);
        hidden_out[h]     = nodeVals[nodeIdx];
    }

    /* Process output nodes */
    for (int o = 0; o < NN_OUTPUTS; o++) {
        int   nodeIdx = NN_NODE_OUT_BASE + o;
        float sum     = 0.0f;
        for (int c = 0; c < g->connCount; c++) {
            if (g->conns[c].to == nodeIdx)
                sum += nodeVals[g->conns[c].from] * g->conns[c].weight;
        }
        /* tanh for thrust/turn (outputs 0,1), sigmoid for reproduce (output 2) */
        if (o < 2) outputs[o] = tanhf(sum);
        else       outputs[o] = 1.0f / (1.0f + expf(-sum));
    }
}

const char *ActivationFuncName(ActivationFunc f) {
    static const char *names[ACT_COUNT] = {
        "linear",
        "relu",
        "sigmoid",
        "tanh",
        "sin",
        "cos",
        "abs",
        "step"
    };
    if (f < 0 || f >= ACT_COUNT) return "unknown";
    return names[f];
}
