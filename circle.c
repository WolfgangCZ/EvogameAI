#include "circle.h"
#include "config.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Helper function to get random float between min and max
float random_float(float min, float max) {
    return min + (float)rand() / (float)(RAND_MAX / (max - min));
}

// Helper function to get random speed vector
Vector2 random_speed(void) {
    float angle = random_float(0, 2 * PI);
    float speed = random_float(MIN_SPEED, MAX_SPEED);
    return (Vector2){ cosf(angle) * speed, sinf(angle) * speed };
}

// Helper function to get random color
Color random_color(void) {
    Color colors[] = {MAROON, RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, PINK, SKYBLUE, VIOLET};
    return colors[rand() % 10];
}

// Helper function to get speed magnitude
float get_speed(const Vector2* speed) {
    return sqrtf(speed->x * speed->x + speed->y * speed->y);
}

// Broad phase collision check using AABB
bool broad_phase_collision(const Circle* a, const Circle* b) {
    float total_radius = a->radius + b->radius;
    return fabsf(a->position.x - b->position.x) <= total_radius &&
           fabsf(a->position.y - b->position.y) <= total_radius;
}

// Narrow phase collision check
bool narrow_phase_collision(const Circle* a, const Circle* b, float* distance, float* nx, float* ny) {
    float dx = a->position.x - b->position.x;
    float dy = a->position.y - b->position.y;
    *distance = sqrtf(dx * dx + dy * dy);
    float min_distance = a->radius + b->radius;

    if (*distance < min_distance) {
        *nx = dx / *distance;
        *ny = dy / *distance;
        return true;
    }
    return false;
}

// Function to check and handle boundary collisions
void handle_boundary_collision(Circle* circle, float left, float right, float top, float bottom) {
    // Left boundary
    if (circle->position.x <= (left + circle->radius)) {
        circle->position.x = left + circle->radius;
        circle->speed.x = fabsf(circle->speed.x);
    }
    // Right boundary
    if (circle->position.x >= (right - circle->radius)) {
        circle->position.x = right - circle->radius;
        circle->speed.x = -fabsf(circle->speed.x);
    }
    // Top boundary
    if (circle->position.y <= (top + circle->radius)) {
        circle->position.y = top + circle->radius;
        circle->speed.y = fabsf(circle->speed.y);
    }
    // Bottom boundary
    if (circle->position.y >= (bottom - circle->radius)) {
        circle->position.y = bottom - circle->radius;
        circle->speed.y = -fabsf(circle->speed.y);
    }
}

// Initialize circles array
void initialize_circles(Circle* circles, int* circle_count, float boundary_left, float boundary_right, 
                      float boundary_top, float boundary_bottom) {
    *circle_count = INITIAL_CIRCLES;
    for (int i = 0; i < *circle_count; i++) {
        circles[i].position = (Vector2){
            random_float(boundary_left + MAX_RADIUS, boundary_right - MAX_RADIUS),
            random_float(boundary_top + MAX_RADIUS, boundary_bottom - MAX_RADIUS)
        };
        circles[i].speed = random_speed();
        circles[i].color = random_color();
        circles[i].radius = random_float(MIN_RADIUS, MAX_RADIUS);
        circles[i].facing_angle = random_float(0, 360); // Initialize random facing angle
    }
}

// Update circles
void update_circles(Circle* circles, int circle_count, float boundary_left, float boundary_right, 
                  float boundary_top, float boundary_bottom) {
    for (int i = 0; i < circle_count; i++) {
        // Update position
        circles[i].position.x += circles[i].speed.x;
        circles[i].position.y += circles[i].speed.y;

        // Check boundary collisions
        handle_boundary_collision(&circles[i], boundary_left, boundary_right, boundary_top, boundary_bottom);

        // Check circle collisions with broad and narrow phase
        for (int j = i + 1; j < circle_count; j++) {
            // Broad phase check first
            if (!broad_phase_collision(&circles[i], &circles[j])) {
                continue;
            }

            // If broad phase passes, do narrow phase check
            float distance, nx, ny;
            if (narrow_phase_collision(&circles[i], &circles[j], &distance, &nx, &ny)) {
                // Relative velocity
                float dvx = circles[i].speed.x - circles[j].speed.x;
                float dvy = circles[i].speed.y - circles[j].speed.y;
                float speed = dvx * nx + dvy * ny;

                // Don't resolve if circles are moving apart
                if (speed > 0) continue;

                // Elastic collision response
                circles[i].speed.x -= speed * nx;
                circles[i].speed.y -= speed * ny;
                circles[j].speed.x += speed * nx;
                circles[j].speed.y += speed * ny;

                // Move circles apart to prevent sticking
                float overlap = (circles[i].radius + circles[j].radius - distance) / 2;
                circles[i].position.x += overlap * nx;
                circles[i].position.y += overlap * ny;
                circles[j].position.x -= overlap * nx;
                circles[j].position.y -= overlap * ny;
            }
        }
    }
}

// Draw circles and their direction indicators
void draw_circles(const Circle* circles, int circle_count, bool show_debug) {
    for (int i = 0; i < circle_count; i++) {
        // Draw the circle
        DrawCircleV(circles[i].position, circles[i].radius, circles[i].color);
        
        // Draw facing direction line
        if (show_debug) {
            // Convert facing angle to radians
            float angle_rad = circles[i].facing_angle * DEG2RAD;
            Vector2 direction = (Vector2){
                cosf(angle_rad),
                sinf(angle_rad)
            };
            Vector2 line_end = (Vector2){
                circles[i].position.x + direction.x * DIRECTION_LINE_LENGTH,
                circles[i].position.y + direction.y * DIRECTION_LINE_LENGTH
            };
            
            // Draw facing direction line
            DrawLineV(circles[i].position, line_end, BLACK);
        }
    }
} 