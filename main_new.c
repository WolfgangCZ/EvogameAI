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
    const float boundaryLeft = BOUNDARY_MARGIN;
    const float boundaryRight = SCREEN_WIDTH - BOUNDARY_MARGIN;
    const float boundaryTop = BOUNDARY_MARGIN;
    const float boundaryBottom = SCREEN_HEIGHT - BOUNDARY_MARGIN;

    // Initialize camera
    Camera2D camera = InitializeCamera();

    // Initialize random seed
    srand(time(NULL));

    // Initialize circles array
    Circle* circles = (Circle*)malloc(MAX_CIRCLES * sizeof(Circle));
    int circleCount = 0;
    float spawnTimer = 0.0f;
    bool showDebug = false;  // Debug mode flag
    
    // Initialize initial circles
    InitializeCircles(circles, &circleCount, boundaryLeft, boundaryRight, boundaryTop, boundaryBottom);

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Handle camera movement and zoom
        float deltaTime = GetFrameTime();
        UpdateGameCamera(&camera, deltaTime);
        UpdateCameraZoom(&camera, deltaTime);

        // Toggle debug mode with 'D' key
        if (IsKeyPressed(KEY_D)) {
            showDebug = !showDebug;
        }

        // Add new circles while holding 'A' key
        if (IsKeyDown(KEY_A) && circleCount < MAX_CIRCLES) {
            spawnTimer += deltaTime;
            if (spawnTimer >= SPAWN_DELAY) {
                Vector2 mousePos = GetMousePosition();
                circles[circleCount].position = mousePos;
                circles[circleCount].speed = RandomSpeed();
                circles[circleCount].color = RandomColor();
                circles[circleCount].radius = RandomFloat(MIN_RADIUS, MAX_RADIUS);
                circleCount++;
                spawnTimer = 0.0f;
            }
        } else {
            spawnTimer = 0.0f;
        }

        // Update circles
        UpdateCircles(circles, circleCount, boundaryLeft, boundaryRight, boundaryTop, boundaryBottom);

        // Draw
        BeginDrawing();
            ClearBackground((Color){ 40, 40, 40, 255 });  // Soft dark gray
            
            // Begin 2D mode with camera
            BeginMode2D(camera);
                // Draw boundary rectangle
                DrawRectangleLinesEx((Rectangle){
                    boundaryLeft,
                    boundaryTop,
                    boundaryRight - boundaryLeft,
                    boundaryBottom - boundaryTop
                }, BOUNDARY_THICKNESS, WHITE);
                
                // Draw circles
                DrawCircles(circles, circleCount, showDebug);
            EndMode2D();

            // Draw debug information
            if (showDebug) {
                DrawDebugInfo(GetFPS(), circleCount, camera.zoom);
            }
        EndDrawing();
    }

    // Cleanup
    free(circles);
    CloseWindow();
    return 0;
} 