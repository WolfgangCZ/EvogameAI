#ifndef CIRCLE_H
#define CIRCLE_H

#include "raylib.h"

#define TRAIL_LENGTH 5  // Number of previous positions to store

typedef struct {
    Vector2 position;
    Vector2 speed;
    Color color;
    float radius;
    float facing_direction;  // Direction the circle is facing (in degrees)
    Vector2 trail[TRAIL_LENGTH];  // Array to store previous positions
    int trail_index;  // Current index in the trail array
} Circle;

// Helper functions
float random_float(float min, float max);
Vector2 random_speed(void);
Color random_color(void);
float get_speed(const Vector2* speed);

// Collision functions
bool broad_phase_collision(const Circle* a, const Circle* b);
bool narrow_phase_collision(const Circle* a, const Circle* b, float* distance, float* nx, float* ny);
void handle_boundary_collision(Circle* circle, float left, float right, float top, float bottom);

// Circle management
void initialize_circles(Circle* circles, int* circle_count, float boundary_left, float boundary_right, 
                      float boundary_top, float boundary_bottom);
void update_circles(Circle* circles, int circle_count, float boundary_left, float boundary_right, 
                  float boundary_top, float boundary_bottom);
void draw_circles(const Circle* circles, int circle_count, bool show_debug);
void rotate_circle_direction(Circle* circle, float rotation_degrees);  // New function for direction rotation

#endif // CIRCLE_H 