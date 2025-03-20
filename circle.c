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

// Initialize a circle with random position and zero initial speed
void init_circle(Circle* circle, float boundary_left, float boundary_right, float boundary_top, float boundary_bottom) {
    // Random position within boundaries
    circle->position.x = GetRandomValue(boundary_left + CIRCLE_RADIUS, boundary_right - CIRCLE_RADIUS);
    circle->position.y = GetRandomValue(boundary_top + CIRCLE_RADIUS, boundary_bottom - CIRCLE_RADIUS);
    
    // Zero initial speed
    circle->speed.x = 0.0f;
    circle->speed.y = 0.0f;
    
    // Fixed radius
    circle->radius = CIRCLE_RADIUS;
    
    // Fixed green color for all circles
    circle->color = GREEN;
    
    // Random facing direction
    circle->facing_angle = random_float(0.0f, 2.0f * PI);
    
    // Initialize health
    circle->health = MAX_HEALTH;
    
    // Initialize selection state
    circle->selected = false;
}

// Initialize circles array
void initialize_circles(Circle* circles, int* circle_count, float boundary_left, float boundary_right, 
                      float boundary_top, float boundary_bottom) {
    for (int i = 0; i < *circle_count; i++) {
        init_circle(&circles[i], boundary_left, boundary_right, boundary_top, boundary_bottom);
    }
}

// Update circles
void update_circles(Circle* circles, int circle_count, float boundary_left, float boundary_right, 
                  float boundary_top, float boundary_bottom) {
    for (int i = 0; i < circle_count; i++) {
        // Apply friction to slow down the circle
        float friction = 0.98f;  // Friction coefficient (0.98 means 2% speed loss per frame)
        circles[i].speed.x *= friction;
        circles[i].speed.y *= friction;

        // Update position
        circles[i].position.x += circles[i].speed.x;
        circles[i].position.y += circles[i].speed.y;

        // Handle boundary collisions
        handle_boundary_collision(&circles[i], boundary_left, boundary_right, boundary_top, boundary_bottom);

        // Drain health over time
        circles[i].health -= HEALTH_DRAIN_RATE;
        if (circles[i].health < 0.0f) circles[i].health = 0.0f;  // Ensure health doesn't go below 0

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
        draw_circle(&circles[i], show_debug);
    }
}

void draw_circle(const Circle* circle, bool show_debug) {
    // Draw the main circle
    DrawCircleV(circle->position, circle->radius, circle->color);
    
    // Draw facing direction dot
    float facing_dot_radius = circle->radius * 0.4f;  // Reduced to 40% of circle radius
    Vector2 facing_dot_pos = {
        circle->position.x + cosf(circle->facing_angle) * (circle->radius * 0.7f),  // 70% of radius from center
        circle->position.y + sinf(circle->facing_angle) * (circle->radius * 0.7f)
    };
    DrawCircleV(facing_dot_pos, facing_dot_radius, BACKGROUND_COLOR);
    
    // Draw selection highlight if selected
    if (circle->selected) {
        DrawCircleLinesV(circle->position, circle->radius + 2.0f, WHITE);
    }
    
    // Draw health bar and text in debug mode
    if (show_debug) {
        // Health bar background (red)
        float bar_width = circle->radius * 2.0f;
        float bar_height = 6.0f;  // Made slightly taller
        float bar_y = circle->position.y - circle->radius - 15.0f;  // Moved up slightly
        DrawRectangle(circle->position.x - circle->radius, bar_y, bar_width, bar_height, RED);
        
        // Health bar fill (green)
        float health_width = bar_width * (circle->health / MAX_HEALTH);
        DrawRectangle(circle->position.x - circle->radius, bar_y, health_width, bar_height, GREEN);
        
        // Health text
        char health_text[32];
        sprintf(health_text, "%.1f", circle->health);
        Vector2 text_size = MeasureTextEx(GetFontDefault(), health_text, 12, 1);  // Made text slightly larger
        DrawText(health_text, 
                circle->position.x - text_size.x/2, 
                bar_y - text_size.y - 2, 
                12, WHITE);  // Made text slightly larger
    }
}

void rotate_circle_direction(Circle* circle, float rotation_degrees) {
    // Update facing direction
    circle->facing_angle += rotation_degrees * DEG2RAD;
    
    // Keep angle between 0 and 360 degrees
    while (circle->facing_angle >= 2.0f * PI) circle->facing_angle -= 2.0f * PI;
    while (circle->facing_angle < 0.0f) circle->facing_angle += 2.0f * PI;
    
    // Note: We no longer update the speed vector here
    // The movement direction remains independent of the facing direction
} 