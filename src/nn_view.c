#include "nn_view.h"
#include "genome.h"
#include "config.h"

#include <math.h>
#include "raymath.h"

/* ── Layout constants ────────────────────────────────────────── */
#define PANEL_X      10
#define PANEL_Y      10
#define PANEL_W     370
#define NODE_R       11
#define NODES_H     300   /* height of the node-drawing area */
#define HEADER_PAD    6   /* top padding inside panel */
#define ROW_H        15   /* pixels between text baseline rows */
#define SEP_PAD       4   /* gap above/below separator line */
#define COL_LABEL_H  14   /* reserved height for INPUTS/HIDDEN/OUTPUTS labels */
#define FOOTER_H     16   /* space below node area for ESC hint */

/* Column x positions (panel-relative) */
#define COL_IN      70
#define COL_HID    185
#define COL_OUT    300

/* ── Helpers ─────────────────────────────────────────────────── */

/* Compute the y-centre of node slot `idx` out of `total` within the node area. */
static Vector2 NodePos(int col, int idx, int total, int nodeAreaTop) {
    float spacing = (float)NODES_H / (float)(total > 0 ? total : 1);
    float y = (float)nodeAreaTop + ((float)idx + 0.5f) * spacing;
    return (Vector2){ (float)(PANEL_X + col), y };
}

/* Map an activation value → color: dim gray at 0, bright cyan-white as |val| grows */
static Color NodeColor(float val) {
    float t = tanhf(fabsf(val));
    return (Color){
        (unsigned char)(30  + t * 180),
        (unsigned char)(40  + t * 210),
        (unsigned char)(60  + t * 195),
        235
    };
}

/* Draw a weighted edge between two screen positions */
static void DrawEdge(Vector2 a, Vector2 b, float weight) {
    float absW = fabsf(weight);
    if (absW < 0.05f) return;

    unsigned char alpha = (unsigned char)Clamp(absW / 2.0f * 180.0f + 25.0f, 25.0f, 200.0f);
    float thick = Clamp(absW * 1.4f, 0.5f, 3.5f);
    Color col = weight > 0.0f
        ? (Color){ 70, 130, 255, alpha }
        : (Color){ 255,  70,  70, alpha };
    DrawLineEx(a, b, thick, col);
}

/* ── Public API ──────────────────────────────────────────────── */

void NNViewDraw(const Creature *c) {
    static const char *inputLabels[NN_INPUTS] = {
        "FoodDst", "FoodSin", "FoodCos",
        "CrtDst",  "CrtSin",
        "Energy",  "Bias"
    };
    static const char *outputLabels[NN_OUTPUTS] = {
        "Thrust", "Turn", "Repro"
    };

    const Genome *g   = &c->genome;
    int  hidCount     = g->hiddenCount;
    bool hasHidden    = (hidCount > 0);
    float fovDeg      = c->visionAngle * RAD2DEG;

    /* Energy cost breakdown */
    float sizeW      = (c->size / CREATURE_SIZE) * (c->size / CREATURE_SIZE);
    float costBase   = c->metabolism * sizeW;
    float costVision = c->vision * c->vision * c->visionAngle * VISION_COST_SCALE;
    float costMove   = 0.03f * c->speed * sizeW;   /* extra at full speed */

    /* ── Compute layout geometry (no drawing yet) ────────────── */
    /* Each call to a "row" just advances py by ROW_H.
       Adding a new row is a one-liner; order can be freely changed. */
    int py = PANEL_Y + HEADER_PAD;
    py += ROW_H + 2;   /* title row (slightly taller) */
    py += ROW_H;       /* age / energy */
    py += ROW_H;       /* size / speed / lifespan */
    py += ROW_H;       /* vision / FOV / metabolism */
    py += ROW_H;       /* mutation rate / repro CD / conns / hidden */
    py += ROW_H;       /* energy costs */
    py += SEP_PAD;     /* gap before separator */
    int sepY = py;     /* separator y */
    py += 1 + SEP_PAD; /* separator + gap after */
    int colLabelY = py;/* column label y */
    py += COL_LABEL_H;
    int nodeAreaTop = py;  /* node drawing area starts here */

    int panelH = nodeAreaTop - PANEL_Y + NODES_H + FOOTER_H;

    /* ── Panel background ────────────────────────────────────── */
    DrawRectangle(PANEL_X, PANEL_Y, PANEL_W, panelH, (Color){ 12, 12, 22, 230 });
    DrawRectangleLines(PANEL_X, PANEL_Y, PANEL_W, panelH, (Color){ 80, 80, 110, 255 });

    /* ── Header rows ─────────────────────────────────────────── */
    Color c1 = (Color){ 180, 220, 180, 255 };
    Color c2 = (Color){ 150, 190, 220, 230 };
    Color c3 = (Color){ 190, 170, 220, 220 };
    Color c4 = (Color){ 130, 160, 180, 200 };

    py = PANEL_Y + HEADER_PAD;

    DrawText(TextFormat("Creature #%d", c->id),
             PANEL_X + 8, py, 11, c1);
    py += ROW_H + 2;

    DrawText(TextFormat("Age: %.0fs   Energy: %.0f / %.0f",
                        c->age, c->energy, c->maxEnergy),
             PANEL_X + 8, py, 10, c1);
    py += ROW_H;

    DrawText(TextFormat("Size: %.1fpx   Speed: %.0fpx/s   Lifespan: %.0fs",
                        c->size, c->speed, g->lifespan),
             PANEL_X + 8, py, 10, c2);
    py += ROW_H;

    DrawText(TextFormat("Vision: %.0fpx   FOV: %.0fdeg   Metabolism: %.2f/s",
                        c->vision, fovDeg * 2.0f, c->metabolism),
             PANEL_X + 8, py, 10, c2);
    py += ROW_H;

    DrawText(TextFormat("Mut.rate: %.3f   Repro.CD: %.1fs   Conns: %d   Hidden: %d",
                        g->mutationRate, c->reproductionCooldown, g->connCount, hidCount),
             PANEL_X + 8, py, 10, c3);
    py += ROW_H;

    DrawText(TextFormat("Cost/s:  base=%.2f  vis=%.3f  move+%.2f  max=%.2f",
                        costBase, costVision, costMove, costBase + costVision + costMove),
             PANEL_X + 8, py, 10, c4);

    /* Separator */
    DrawLine(PANEL_X + 4, sepY, PANEL_X + PANEL_W - 4, sepY, (Color){ 60, 60, 90, 180 });

    /* Column labels — always below separator */
    DrawText("INPUTS",  PANEL_X + COL_IN  - 25, colLabelY, 10, GRAY);
    if (hasHidden)
        DrawText("HIDDEN",  PANEL_X + COL_HID - 20, colLabelY, 10, GRAY);
    DrawText("OUTPUTS", PANEL_X + COL_OUT - 25, colLabelY, 10, GRAY);

    /* ── Precompute node screen positions ────────────────────── */
    Vector2 inPos[NN_INPUTS];
    Vector2 hidPos[NN_HIDDEN_MAX];
    Vector2 outPos[NN_OUTPUTS];

    for (int i = 0; i < NN_INPUTS;  i++) inPos[i]  = NodePos(COL_IN,  i, NN_INPUTS,  nodeAreaTop);
    for (int h = 0; h < hidCount;   h++) hidPos[h]  = NodePos(COL_HID, h, hidCount,   nodeAreaTop);
    for (int o = 0; o < NN_OUTPUTS; o++) outPos[o]  = NodePos(COL_OUT, o, NN_OUTPUTS, nodeAreaTop);

    /* Global node index → screen position lookup */
    Vector2 nodeScreenPos[NN_NODE_COUNT];
    for (int i = 0; i < NN_INPUTS;  i++) nodeScreenPos[i]                       = inPos[i];
    for (int h = 0; h < hidCount;   h++) nodeScreenPos[NN_NODE_HIDDEN_BASE + h] = hidPos[h];
    for (int o = 0; o < NN_OUTPUTS; o++) nodeScreenPos[NN_NODE_OUT_BASE + o]    = outPos[o];

    /* ── Edges ───────────────────────────────────────────────── */
    for (int ci = 0; ci < g->connCount; ci++) {
        int   fromNode = g->conns[ci].from;
        int   toNode   = g->conns[ci].to;
        float weight   = g->conns[ci].weight;

        bool fromValid = (fromNode < NN_INPUTS) ||
                         (fromNode >= NN_NODE_HIDDEN_BASE &&
                          fromNode <  NN_NODE_HIDDEN_BASE + hidCount);
        bool toValid   = (toNode >= NN_NODE_HIDDEN_BASE &&
                          toNode <  NN_NODE_HIDDEN_BASE + hidCount) ||
                         (toNode >= NN_NODE_OUT_BASE &&
                          toNode <  NN_NODE_OUT_BASE + NN_OUTPUTS);

        if (fromValid && toValid)
            DrawEdge(nodeScreenPos[fromNode], nodeScreenPos[toNode], weight);
    }

    /* ── Input nodes ─────────────────────────────────────────── */
    for (int i = 0; i < NN_INPUTS; i++) {
        float val = c->nnInputs[i];
        DrawCircleV(inPos[i], NODE_R, NodeColor(val));
        DrawCircleLinesV(inPos[i], NODE_R, (Color){ 160, 160, 200, 180 });
        DrawText(inputLabels[i],
                 (int)(inPos[i].x - NODE_R - 52), (int)(inPos[i].y - 5), 9,
                 (Color){ 200, 200, 220, 220 });
        DrawText(TextFormat("%.2f", val),
                 (int)(inPos[i].x - 10), (int)(inPos[i].y + NODE_R + 1), 8,
                 (Color){ 150, 200, 150, 200 });
    }

    /* ── Hidden nodes ────────────────────────────────────────── */
    for (int h = 0; h < hidCount; h++) {
        float val      = c->hiddenOut[h];
        ActivationFunc act = g->hiddenAct[h];
        DrawCircleV(hidPos[h], NODE_R, NodeColor(val));
        DrawCircleLinesV(hidPos[h], NODE_R, (Color){ 160, 160, 200, 180 });
        DrawText(ActivationFuncName(act),
                 (int)(hidPos[h].x - 14), (int)(hidPos[h].y - 5), 8,
                 (Color){ 220, 220, 180, 230 });
        DrawText(TextFormat("%.2f", val),
                 (int)(hidPos[h].x - 10), (int)(hidPos[h].y + NODE_R + 1), 8,
                 (Color){ 150, 200, 150, 200 });
    }

    /* ── Output nodes ────────────────────────────────────────── */
    for (int o = 0; o < NN_OUTPUTS; o++) {
        float val = c->nnOutputs[o];
        DrawCircleV(outPos[o], NODE_R, NodeColor(val));
        DrawCircleLinesV(outPos[o], NODE_R, (Color){ 160, 160, 200, 180 });
        DrawText(outputLabels[o],
                 (int)(outPos[o].x + NODE_R + 4), (int)(outPos[o].y - 5), 9,
                 (Color){ 200, 200, 220, 220 });
        DrawText(TextFormat("%.2f", val),
                 (int)(outPos[o].x - 10), (int)(outPos[o].y + NODE_R + 1), 8,
                 (Color){ 150, 200, 150, 200 });
    }

    /* Dismiss hint */
    DrawText("[ESC] close", PANEL_X + PANEL_W - 68, PANEL_Y + panelH - 14, 9,
             (Color){ 100, 100, 120, 180 });
}
