#include "raylib.h"
#include "config.h"
#include "circle.h"
#include "camera.h"
#include "debug.h"
#include "rectangle.h"
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

    // Initialize circles
    Circle circles[CIRCLE_COUNT];
    int circle_count = CIRCLE_COUNT;
    float spawn_timer = 0.0f;
    bool show_debug = false;  // Debug mode flag
    int selected_circle_index = 0;  // Start with first circle selected
    float speed_adjustment = 50.0f;  // Speed change per second (scaled by delta_time)
    float rotation_speed = 180.0f;  // Degrees per second
    
    // Initialize circles
    // Initialize initial circles
    initialize_circles(circles, &circle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

    // Initialize rectangles
    GameRectangle rectangles[MAX_RECTANGLES];
    int rectangle_count;
    init_rectangles(rectangles, &rectangle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

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
                float angle_rad = selected->facing_direction * DEG2RAD;
                float speed_change = speed_adjustment * delta_time;  // Scale by delta_time
                selected->speed.x += cosf(angle_rad) * speed_change;
                selected->speed.y += sinf(angle_rad) * speed_change;
            }
            if (IsKeyDown(KEY_S)) {
                // Reduce speed in the facing direction
                float angle_rad = selected->facing_direction * DEG2RAD;
                float speed_change = speed_adjustment * delta_time;  // Scale by delta_time
                selected->speed.x -= cosf(angle_rad) * speed_change;
                selected->speed.y -= sinf(angle_rad) * speed_change;
            }

            // Direction control with A/D
            if (IsKeyDown(KEY_A)) {
                rotate_circle_direction(selected, -rotation_speed * delta_time);
            }
            else if (IsKeyDown(KEY_D)) {
                rotate_circle_direction(selected, rotation_speed * delta_time);
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
                    circles[circle_count].speed = (Vector2){0.0f, 0.0f};  // Zero initial speed
                    circles[circle_count].color = random_color();
                    circles[circle_count].radius = CIRCLE_RADIUS;  // Use fixed circle radius
                    circles[circle_count].facing_direction = random_float(0, 360);
                    circle_count++;
                }
                spawn_timer = 0.0f;
            }
        } else {
            spawn_timer = 0.0f;
        }

        // Update circles
        update_circles(circles, circle_count, boundary_left, boundary_right, boundary_top, boundary_bottom);

        // Check for circle-rectangle collisions
        handle_circle_rectangle_collisions(rectangles, rectangle_count, circles, circle_count,
                                         boundary_left, boundary_right, boundary_top, boundary_bottom);

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
                const char* debug_text = TextFormat(
                    "FPS: %d\n"
                    "Circles: %d\n"
                    "Selected: %d\n"
                    "Selected Size: %.1f\n"
                    "Selected Speed: %.1f\n"
                    "Camera: %.1f, %.1f\n"
                    "Zoom: %.2f",
                    GetFPS(),
                    circle_count,
                    selected_circle_index,
                    selected_circle_index >= 0 ? circles[selected_circle_index].radius : 0.0f,
                    selected_circle_index >= 0 ? sqrtf(circles[selected_circle_index].speed.x * circles[selected_circle_index].speed.x + 
                                                    circles[selected_circle_index].speed.y * circles[selected_circle_index].speed.y) : 0.0f,
                    camera.target.x,
                    camera.target.y,
                    camera.zoom
                );
                DrawText(debug_text, 10, 10, 20, WHITE);
            }
        EndDrawing();
    }

    // Cleanup
    free(circles);
    CloseWindow();
    return 0;
} 