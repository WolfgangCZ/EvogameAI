#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

// Window configuration
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define TARGET_FPS 60

// Game area boundaries (relative to window size)
#define BOUNDARY_MARGIN 50.0f

// Circle configuration
#define CIRCLE_COUNT 5
#define CIRCLE_RADIUS 15.0f  // Fixed size for all circles

// Colors
#define BACKGROUND_COLOR DARKGRAY
#define BOUNDARY_COLOR WHITE
#define CIRCLE_COLOR GREEN
#define SELECTED_CIRCLE_COLOR GOLD
#define FOOD_COLOR YELLOW

// Game settings
#define INITIAL_CIRCLES 10
#define MIN_SPEED 50.0f
#define MAX_SPEED 200.0f
#define TRAIL_LENGTH 5

// Circle parameters
#define MAX_CIRCLES 10000  // Increased from 100 to 10000
#define SPAWN_DELAY 0.02f  // Time between spawns in seconds (50 circles per second)
#define DIRECTION_LINE_LENGTH 15.0f  // Base length for direction indicator

// Boundary rectangle parameters
#define BOUNDARY_THICKNESS 2.0f

// Zoom parameters
#define MIN_ZOOM 0.5f
#define MAX_ZOOM 3.0f
#define ZOOM_SPEED 0.5f  // Increased from 0.1f to 0.5f for more noticeable zoom

// Camera movement speed (pixels per second)
#define CAMERA_MOVE_SPEED 500.0f

// Screen dimensions
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

// Health settings
#define MAX_HEALTH 50.0f
#define HEALTH_DRAIN_RATE 0.1f  // Health lost per frame
#define HEALTH_GAIN_FROM_FOOD 20.0f  // Health gained when eating a rectangle

#endif // CONFIG_H 