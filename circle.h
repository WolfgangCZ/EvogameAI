#ifndef CIRCLE_H
#define CIRCLE_H

#include "raylib.h"

#define MAX_HEALTH 50.0f

typedef struct {
    Vector2 position;
    Vector2 speed;
    float radius;
    Color color;
    bool selected;
    float facing_angle;  // Angle in radians
    float health;       // Current health of the circle
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
void draw_circle(const Circle* circle, bool show_debug);
void rotate_circle_direction(Circle* circle, float rotation_degrees);

#endif // CIRCLE_H 