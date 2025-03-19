#include "rectangle.h"
#include "config.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Helper function to check if a point is inside a rectangle
bool is_point_in_rectangle(Vector2 point, const GameRectangle* rect) {
    return (point.x >= rect->position.x && 
            point.x <= rect->position.x + rect->width &&
            point.y >= rect->position.y && 
            point.y <= rect->position.y + rect->height);
}

// Function to respawn a rectangle at a new random position
void respawn_rectangle(GameRectangle* rect, float boundary_left, float boundary_right, 
                      float boundary_top, float boundary_bottom) {
    rect->position.x = boundary_left + (rand() % (int)(boundary_right - boundary_left - RECTANGLE_SIZE));
    rect->position.y = boundary_top + (rand() % (int)(boundary_bottom - boundary_top - RECTANGLE_SIZE));
}

// Function to handle rectangle clicks and respawning
void handle_rectangle_clicks(GameRectangle* rectangles, int rectangle_count, Vector2 click_pos,
                           float boundary_left, float boundary_right, float boundary_top, float boundary_bottom) {
    for (int i = 0; i < rectangle_count; i++) {
        if (is_point_in_rectangle(click_pos, &rectangles[i])) {
            // Respawn this rectangle at a new position
            respawn_rectangle(&rectangles[i], boundary_left, boundary_right, boundary_top, boundary_bottom);
            break;  // Only handle one rectangle per click
        }
    }
}

void init_rectangles(GameRectangle* rectangles, int* rectangle_count, float boundary_left, float boundary_right, 
                    float boundary_top, float boundary_bottom) {
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    // Generate random number of rectangles (between 5 and MAX_RECTANGLES)
    *rectangle_count = 5 + (rand() % (MAX_RECTANGLES - 4));
    
    // Initialize each rectangle
    for (int i = 0; i < *rectangle_count; i++) {
        rectangles[i].width = RECTANGLE_SIZE;
        rectangles[i].height = RECTANGLE_SIZE;
        rectangles[i].color = YELLOW;
        
        // Random position within boundaries
        respawn_rectangle(&rectangles[i], boundary_left, boundary_right, boundary_top, boundary_bottom);
    }
}

void draw_rectangles(const GameRectangle* rectangles, int rectangle_count) {
    for (int i = 0; i < rectangle_count; i++) {
        DrawRectangleV(rectangles[i].position, (Vector2){rectangles[i].width, rectangles[i].height}, rectangles[i].color);
    }
}

// Helper function to check if a circle collides with a rectangle
static bool check_circle_rectangle_collision(Vector2 circle_pos, float circle_radius, GameRectangle* rect) {
    // Find the closest point on the rectangle to the circle's center
    float closest_x = fmaxf(rect->position.x, fminf(circle_pos.x, rect->position.x + RECTANGLE_SIZE));
    float closest_y = fmaxf(rect->position.y, fminf(circle_pos.y, rect->position.y + RECTANGLE_SIZE));
    
    // Calculate the distance between the circle's center and the closest point
    float dx = circle_pos.x - closest_x;
    float dy = circle_pos.y - closest_y;
    float distance = sqrtf(dx*dx + dy*dy);
    
    return distance <= circle_radius;
}

// Function to handle circle-rectangle collisions
void handle_circle_rectangle_collisions(GameRectangle* rectangles, int rectangle_count, Circle* circles, int circle_count,
                                      float boundary_left, float boundary_right, float boundary_top, float boundary_bottom) {
    for (int i = 0; i < rectangle_count; i++) {
        for (int j = 0; j < circle_count; j++) {
            if (check_circle_rectangle_collision(circles[j].position, circles[j].radius, &rectangles[i])) {
                // Respawn the rectangle at a new random position
                respawn_rectangle(&rectangles[i], boundary_left, boundary_right, boundary_top, boundary_bottom);
                break;  // Break inner loop since we've respawned this rectangle
            }
        }
    }
} 