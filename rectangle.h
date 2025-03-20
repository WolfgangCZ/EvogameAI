#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "raylib.h"
#include "circle.h"

#define MAX_RECTANGLES 50
#define RECTANGLE_SIZE (CIRCLE_RADIUS * 1.5f)  // Make rectangles 1.5x the circle radius

// Rectangle structure (renamed to GameRectangle to avoid conflicts with raylib's Rectangle)
typedef struct {
    Vector2 position;
    float width;
    float height;
    Color color;
} GameRectangle;

// Initialize rectangles array
void init_rectangles(GameRectangle* rectangles, int* rectangle_count, float boundary_left, float boundary_right, 
                    float boundary_top, float boundary_bottom);

// Draw rectangles
void draw_rectangles(const GameRectangle* rectangles, int rectangle_count);

// Handle rectangle clicks and respawning
void handle_rectangle_clicks(GameRectangle* rectangles, int rectangle_count, Vector2 click_pos,
                           float boundary_left, float boundary_right, float boundary_top, float boundary_bottom);

void handle_circle_rectangle_collisions(GameRectangle* rectangles, int rectangle_count,
                                      Circle* circles, int circle_count,
                                      float boundary_left, float boundary_right,
                                      float boundary_top, float boundary_bottom);

void respawn_rectangle(GameRectangle* rect, float boundary_left, float boundary_right, 
                      float boundary_top, float boundary_bottom);

#endif // RECTANGLE_H 