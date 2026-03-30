#pragma once

// Window
#define WINDOW_WIDTH    1280
#define WINDOW_HEIGHT   720
#define WINDOW_TITLE    "Evolution Simulation"
#define TARGET_FPS      60

// World
#define WORLD_WIDTH     1200
#define WORLD_HEIGHT    720

// UI sidebar
#define UI_PANEL_WIDTH  (WINDOW_WIDTH - WORLD_WIDTH)

// Food
#define MAX_FOOD        600
#define FOOD_SPAWN_RATE 3.0f    // food items per second
#define FOOD_NUTRITION  20.0f
#define FOOD_SIZE       5.0f

// Creatures (placeholders for later milestones)
#define MAX_CREATURES   500
#define INITIAL_CREATURES 40

// Genetics
#define GENE_COUNT      9

// Simulation
#define FIXED_DT        (1.0f / 60.0f)
