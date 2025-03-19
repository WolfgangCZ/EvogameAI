#include "raylib.h"
#include "config.h"
#include "circle.h"
#include "camera.h"
#include "debug.h"
#include <stdlib.h>
#include <time.h>

int main(void)
{
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bouncing Circles");

    // Calculate boundary rectangle
    const float boundary_left = BOUNDARY_MARGIN;
    const float boundary_right = SCREEN_WIDTH - BOUNDARY_MARGIN;
    const float boundary_top = BOUNDARY_MARGIN;
    const float boundary_bottom = SCREEN_HEIGHT - BOUNDARY_MARGIN;

    // Initialize camera
    Camera2D camera = initialize_camera();

    // Initialize random seed
    srand(time(NULL));

    // Initialize circles array
    Circle* circles = (Circle*)malloc(MAX_CIRCLES * sizeof(Circle));
    int circle_count = 0;
    float spawn_timer = 0.0f;
    bool show_debug = false;  // Debug mode flag
    int selected_circle_index = -1;  // -1 means no circle is selected
    float speed_adjustment = 50.0f;  // Speed change per frame
    float rotation_speed = 180.0f;  // Degrees per second
    
    // Initialize initial circles
    initialize_circles(circles, &circle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Handle camera movement and zoom
        float delta_time = GetFrameTime();
        update_game_camera(&camera, delta_time);
        update_camera_zoom(&camera, delta_time);

        // Toggle debug mode with 'F1' key
        if (IsKeyPressed(KEY_F1)) {
            show_debug = !show_debug;
        }

        // Handle circle selection with left mouse button
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);
            selected_circle_index = -1;  // Reset selection
            
            // Check each circle for hit
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
            
            // Speed control with W/S
            if (IsKeyDown(KEY_W)) {
                // Increase speed while maintaining direction
                float speed_multiplier = (current_speed + speed_adjustment * delta_time) / current_speed;
                selected->speed.x *= speed_multiplier;
                selected->speed.y *= speed_multiplier;
            }
            else if (IsKeyDown(KEY_S)) {
                // Decrease speed while maintaining direction
                float speed_multiplier = (current_speed - speed_adjustment * delta_time) / current_speed;
                if (speed_multiplier > 0) {  // Prevent negative speed
                    selected->speed.x *= speed_multiplier;
                    selected->speed.y *= speed_multiplier;
                }
            }

            // Direction control with A/D
            if (current_speed > 0) {  // Only rotate if circle is moving
                float angle = rotation_speed * delta_time;  // Angle to rotate this frame
                
                if (IsKeyDown(KEY_A)) {
                    // Rotate counterclockwise
                    float cos_a = cosf(angle * DEG2RAD);
                    float sin_a = sinf(angle * DEG2RAD);
                    float new_x = selected->speed.x * cos_a - selected->speed.y * sin_a;
                    float new_y = selected->speed.x * sin_a + selected->speed.y * cos_a;
                    selected->speed.x = new_x;
                    selected->speed.y = new_y;
                }
                else if (IsKeyDown(KEY_D)) {
                    // Rotate clockwise
                    float cos_a = cosf(-angle * DEG2RAD);
                    float sin_a = sinf(-angle * DEG2RAD);
                    float new_x = selected->speed.x * cos_a - selected->speed.y * sin_a;
                    float new_y = selected->speed.x * sin_a + selected->speed.y * cos_a;
                    selected->speed.x = new_x;
                    selected->speed.y = new_y;
                }
            }
        }

        // Add new circles while holding 'SPACE' key
        if (IsKeyDown(KEY_SPACE) && circle_count < MAX_CIRCLES) {
            spawn_timer += delta_time;
            if (spawn_timer >= SPAWN_DELAY) {
                // Convert mouse position from screen coordinates to world coordinates
                Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);
                
                // Only spawn if within boundary
                if (mouse_pos.x >= boundary_left && mouse_pos.x <= boundary_right &&
                    mouse_pos.y >= boundary_top && mouse_pos.y <= boundary_bottom) {
                    circles[circle_count].position = mouse_pos;
                    circles[circle_count].speed = random_speed();
                    circles[circle_count].color = random_color();
                    circles[circle_count].radius = random_float(MIN_RADIUS, MAX_RADIUS);
                    circles[circle_count].facing_angle = random_float(0, 360);
                    circle_count++;
                }
                spawn_timer = 0.0f;
            }
        } else {
            spawn_timer = 0.0f;
        }

        // Update circles
        update_circles(circles, circle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

        // Draw
        BeginDrawing();
            ClearBackground((Color){ 40, 40, 40, 255 });  // Soft dark gray
            
            // Begin 2D mode with camera
            BeginMode2D(camera);
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
                if (selected_circle_index >= 0) {
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
                DrawDebugInfo(GetFPS(), circle_count, camera.zoom);
            }
        EndDrawing();
    }

    // Cleanup
    free(circles);
    CloseWindow();
    return 0;
} 