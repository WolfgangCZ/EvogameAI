#include "raylib.h"
#include "config.h"
#include "circle.h"
#include "camera.h"
#include "debug.h"
#include "rectangle.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int main(void)
{
    // Initialize random seed
    srand(time(NULL));

    printf("Starting initialization...\n");

    // Initialize window and camera
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bouncing Circles");
    SetTargetFPS(60);

    printf("Window initialized\n");

    // Calculate boundary rectangle
    const float boundary_left = BOUNDARY_MARGIN;
    const float boundary_right = SCREEN_WIDTH - BOUNDARY_MARGIN;
    const float boundary_top = BOUNDARY_MARGIN;
    const float boundary_bottom = SCREEN_HEIGHT - BOUNDARY_MARGIN;

    // Initialize camera
    Camera2D camera = initialize_camera();

    printf("Camera initialized\n");

    // Initialize circles
    Circle circles[MAX_CIRCLES];
    int circle_count = INITIAL_CIRCLES;
    float spawn_timer = 0.0f;
    bool show_debug = false;  // Debug mode flag
    int selected_circle_index = 0;  // Start with first circle selected
    float speed_adjustment = 50.0f;  // Speed change per second (scaled by delta_time)
    float rotation_speed = 180.0f;  // Degrees per second
    
    printf("Initializing circles...\n");
    // Initialize circles
    initialize_circles(circles, &circle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

    printf("Circles initialized\n");

    // Initialize rectangles
    GameRectangle rectangles[MAX_RECTANGLES];
    int rectangle_count;
    init_rectangles(rectangles, &rectangle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

    printf("Starting main game loop\n");

    // Main game loop
    while (!WindowShouldClose())
    {
        // Delta time
        float delta_time = GetFrameTime();

        // Toggle debug mode with 'F1' key
        if (IsKeyPressed(KEY_F1)) {
            show_debug = !show_debug;
        }

        // Handle circle selection with left mouse button
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);
            
            // First check for rectangle clicks
            handle_rectangle_clicks(rectangles, rectangle_count, mouse_pos, 
                                  boundary_left, boundary_right, boundary_top, boundary_bottom);
            
            // Then check for circle selection
            selected_circle_index = -1;  // Reset selection
            for (int i = 0; i < circle_count; i++) {
                float dx = mouse_pos.x - circles[i].position.x;
                float dy = mouse_pos.y - circles[i].position.y;
                float distance = sqrtf(dx*dx + dy*dy);
                
                if (distance <= circles[i].radius) {
                    selected_circle_index = i;
                    break;  // Only select the first hit circle
                }
            }
        }

        // Adjust speed and direction of selected circle
        if (selected_circle_index >= 0) {
            Circle* selected = &circles[selected_circle_index];
            float current_speed = sqrtf(selected->speed.x * selected->speed.x + selected->speed.y * selected->speed.y);
            
            // Speed control
            if (IsKeyDown(KEY_W)) {
                // Add speed in the facing direction
                float speed_change = speed_adjustment * delta_time;  // Scale by delta_time
                selected->speed.x += cosf(selected->facing_angle) * speed_change;
                selected->speed.y += sinf(selected->facing_angle) * speed_change;
            }
            if (IsKeyDown(KEY_S)) {
                // Reduce speed in the facing direction
                float speed_change = speed_adjustment * delta_time;  // Scale by delta_time
                selected->speed.x -= cosf(selected->facing_angle) * speed_change;
                selected->speed.y -= sinf(selected->facing_angle) * speed_change;
            }

            // Direction control with A/D
            if (IsKeyDown(KEY_A)) {
                rotate_circle_direction(selected, -rotation_speed * delta_time);
            }
            else if (IsKeyDown(KEY_D)) {
                rotate_circle_direction(selected, rotation_speed * delta_time);
            }
        }

        // Update camera
        update_game_camera(&camera, delta_time);
        update_camera_zoom(&camera, delta_time);

        // Update circles
        update_circles(circles, circle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

        // Check for circle-rectangle collisions
        for (int i = 0; i < circle_count; i++) {
            Circle* circle = &circles[i];
            for (int j = 0; j < rectangle_count; j++) {
                GameRectangle* rect = &rectangles[j];
                
                // Check if circle is close enough to rectangle
                float dx = circle->position.x - rect->position.x;
                float dy = circle->position.y - rect->position.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                // If circle is close enough to eat the rectangle
                if (distance < (circle->radius + RECTANGLE_SIZE/2)) {
                    // Replenish health
                    circle->health += HEALTH_GAIN_FROM_FOOD;
                    if (circle->health > MAX_HEALTH) circle->health = MAX_HEALTH;
                    
                    // Respawn rectangle at new position
                    respawn_rectangle(rect, boundary_left, boundary_right, boundary_top, boundary_bottom);
                }
            }
        }

        // Check if space key is pressed to spawn a new circle
        if (IsKeyPressed(KEY_SPACE) && circle_count < MAX_CIRCLES) {
            // Convert mouse position from screen coordinates to world coordinates
            Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);
            
            // Only spawn if within boundary
            if (mouse_pos.x >= boundary_left && mouse_pos.x <= boundary_right &&
                mouse_pos.y >= boundary_top && mouse_pos.y <= boundary_bottom) {
                circles[circle_count].position = mouse_pos;
                circles[circle_count].speed = (Vector2){0.0f, 0.0f};  // Zero initial speed
                circles[circle_count].color = GREEN;
                circles[circle_count].radius = CIRCLE_RADIUS;  // Use fixed circle radius
                circles[circle_count].facing_angle = random_float(0.0f, 2.0f * PI);
                circles[circle_count].health = MAX_HEALTH;  // Initialize health
                circle_count++;
                printf("Added new circle. Total count: %d\n", circle_count);
            }
        }

        // Draw
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            
            // Begin 2D mode with camera
            BeginMode2D(camera);
                // Draw rectangles first (so they appear behind circles)
                draw_rectangles(rectangles, rectangle_count);
                
                // Draw boundary rectangle
                DrawRectangleLinesEx((Rectangle){
                    boundary_left,
                    boundary_top,
                    boundary_right - boundary_left,
                    boundary_bottom - boundary_top
                }, BOUNDARY_THICKNESS, WHITE);
                
                // Draw circles
                draw_circles(circles, circle_count, show_debug);

                // Draw selection indicator if a circle is selected
                if (selected_circle_index >= 0 && selected_circle_index < circle_count) {
                    Circle* selected = &circles[selected_circle_index];
                    float padding = selected->radius * 0.5f;  // Add 50% padding
                    float size = (selected->radius + padding) * 2;
                    float x = selected->position.x - (selected->radius + padding);
                    float y = selected->position.y - (selected->radius + padding);
                    float corner_length = size * 0.3f;  // Length of each corner line
                    
                    // Draw four corners
                    // Top-left corner
                    DrawLineEx((Vector2){x, y}, (Vector2){x + corner_length, y}, 2, GOLD);
                    DrawLineEx((Vector2){x, y}, (Vector2){x, y + corner_length}, 2, GOLD);
                    
                    // Top-right corner
                    DrawLineEx((Vector2){x + size, y}, (Vector2){x + size - corner_length, y}, 2, GOLD);
                    DrawLineEx((Vector2){x + size, y}, (Vector2){x + size, y + corner_length}, 2, GOLD);
                    
                    // Bottom-left corner
                    DrawLineEx((Vector2){x, y + size}, (Vector2){x + corner_length, y + size}, 2, GOLD);
                    DrawLineEx((Vector2){x, y + size}, (Vector2){x, y + size - corner_length}, 2, GOLD);
                    
                    // Bottom-right corner
                    DrawLineEx((Vector2){x + size, y + size}, (Vector2){x + size - corner_length, y + size}, 2, GOLD);
                    DrawLineEx((Vector2){x + size, y + size}, (Vector2){x + size, y + size - corner_length}, 2, GOLD);
                }
            EndMode2D();

            // Draw debug information
            if (show_debug) {
                DrawRectangle(10, 10, 200, 100, Fade(BLACK, 0.5f));
                DrawText("Debug Info:", 20, 20, 20, WHITE);
                DrawText(TextFormat("Circle Count: %d/%d", circle_count, MAX_CIRCLES), 20, 45, 20, WHITE);
                DrawText(TextFormat("Selected Circle: %d", selected_circle_index), 20, 70, 20, WHITE);
                if (selected_circle_index >= 0 && selected_circle_index < circle_count) {
                    DrawText(TextFormat("Health: %.1f", circles[selected_circle_index].health), 20, 95, 20, WHITE);
                }
            }
        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    return 0;
} 