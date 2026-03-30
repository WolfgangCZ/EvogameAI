#pragma once

/* ── Window ──────────────────────────────────────────────────── */
#define WINDOW_WIDTH    1600
#define WINDOW_HEIGHT    900
#define WINDOW_TITLE    "Evolution Simulation"
#define TARGET_FPS        60

/* ── Simulation world (the toroidal box creatures live in) ───── */
#define WORLD_WIDTH     12000
#define WORLD_HEIGHT     9000

/* ── Viewport (camera renders here) and UI sidebar ───────────── */
#define UI_PANEL_WIDTH  300
#define UI_PANEL_X      (WINDOW_WIDTH - UI_PANEL_WIDTH)
#define VIEWPORT_WIDTH  UI_PANEL_X
#define VIEWPORT_HEIGHT WINDOW_HEIGHT

/* ── Camera ──────────────────────────────────────────────────── */
#define CAMERA_ZOOM_MIN  0.05f
#define CAMERA_ZOOM_MAX  8.0f
#define CAMERA_ZOOM_STEP 0.12f   /* fraction of current zoom per wheel tick */

/* ── Food ────────────────────────────────────────────────────── */
#define MAX_FOOD          8000
#define FOOD_TARGET       2000    /* maximum food items in the world at once */
#define FOOD_SPAWN_RATE   1.5f    /* new food items spawned per second (rate-limited) */
#define FOOD_NUTRITION   20.0f
#define FOOD_SIZE         5.0f

/* ── Creatures ───────────────────────────────────────────────── */
#define MAX_CREATURES     3000
#define INITIAL_CREATURES  20

/* Creature defaults (used as fallback / size for initial spawn bounds) */
#define CREATURE_MAX_ENERGY   100.0f
#define CREATURE_SIZE           6.0f   /* body radius px — used for spawn bounds only */

/* ── Neural network ──────────────────────────────────────────── */
#define NN_INPUTS    7
#define NN_OUTPUTS   3

/* Dynamic topology constants (NEAT-style) */
#define NN_HIDDEN_MAX         8
#define NN_CONN_MAX          64
#define NN_NODE_HIDDEN_BASE   NN_INPUTS
#define NN_NODE_OUT_BASE      (NN_INPUTS + NN_HIDDEN_MAX)
#define NN_NODE_COUNT         (NN_INPUTS + NN_HIDDEN_MAX + NN_OUTPUTS)

/* ── Population floor ────────────────────────────────────────── */
#define MIN_POPULATION    10     /* respawn random creatures when alive count falls below this */

/* ── Reproduction ────────────────────────────────────────────── */
#define REPRODUCE_MIN_ENERGY      40.0f
#define REPRODUCE_NN_THRESHOLD     0.7f   /* output[2] must exceed this */
#define REPRODUCE_ENERGY_COST       0.5f    /* fraction of energy passed to child */
#define REPRODUCE_COOLDOWN          8.0f    /* seconds before creature can reproduce again */

/* ── Vision cost (energy/sec per unit of cone area: vision² × halfAngle) ── */
#define VISION_COST_SCALE           0.0000001f

/* ── Size/weight energy scaling (reference radius = CREATURE_SIZE) ────── */
/*    maxEnergy  = CREATURE_MAX_ENERGY × (size/CREATURE_SIZE)²            */
/*    drain cost = base × (size/CREATURE_SIZE)²                           */

/* ── Simulation ──────────────────────────────────────────────── */
#define FIXED_DT  (1.0f / 60.0f)
