#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

// Window settings
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define TARGET_FPS 60

// Game settings
#define INITIAL_CIRCLES 10
#define MIN_RADIUS 4
#define MAX_RADIUS 8
#define MIN_SPEED 20
#define MAX_SPEED 40
#define TRAIL_LENGTH 5

// Colors
#define BACKGROUND_COLOR (Color){ 25, 25, 25, 255 }  // Dark gray background

// Circle parameters
#define MAX_CIRCLES 10000
#define SPAWN_DELAY 0.02f  // Time between spawns in seconds (50 circles per second)
#define DIRECTION_LINE_LENGTH 15.0f  // Base length for direction indicator

// Boundary rectangle parameters
#define BOUNDARY_MARGIN 100  // Distance from screen edges
#define BOUNDARY_THICKNESS 2  // Thickness of boundary lines

// Zoom parameters
#define MIN_ZOOM 0.5f
#define MAX_ZOOM 3.0f
#define ZOOM_SPEED 0.5f  // Increased from 0.1f to 0.5f for more noticeable zoom

// Camera movement speed (pixels per second)
#define CAMERA_MOVE_SPEED 500.0f

// Screen dimensions
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

#endif // CONFIG_H 