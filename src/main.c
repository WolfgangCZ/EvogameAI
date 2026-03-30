#include "raylib.h"
#include "raymath.h"
#include "config.h"
#include "simulation.h"
#include "settings.h"
#include "ui.h"
#include "nn_view.h"

int main(void) {
    /* ── Init ─────────────────────────────────────────────────── */
    SetRandomSeed(42);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(TARGET_FPS);

    static Simulation sim;
    SimulationInit(&sim);

    SimSettings settings;
    SimSettingsDefault(&settings);

    /* Camera starts zoomed to fit the whole world in the viewport */
    Camera2D camera = { 0 };
    camera.target = (Vector2){ WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f };
    {
        int vpW = GetScreenWidth() - UI_PANEL_WIDTH;
        int vpH = GetScreenHeight();
        camera.offset = (Vector2){ vpW * 0.5f, vpH * 0.5f };
        camera.zoom   = (float)vpW / WORLD_WIDTH;
    }

    int selectedIdx = -1;   /* index into sim.creatures, -1 = none */
    int lastVpW = 0, lastVpH = 0;

    /* ── Main loop ────────────────────────────────────────────── */
    while (!WindowShouldClose()) {

        /* Runtime viewport size (changes when window is resized) */
        int vpW = GetScreenWidth() - UI_PANEL_WIDTH;
        int vpH = GetScreenHeight();

        /* Re-center camera pivot when window is resized */
        if (vpW != lastVpW || vpH != lastVpH) {
            camera.offset = (Vector2){ vpW * 0.5f, vpH * 0.5f };
            lastVpW = vpW;
            lastVpH = vpH;
        }

        /* ── Input ────────────────────────────────────────────── */
        if (IsKeyPressed(KEY_SPACE))  settings.paused = !settings.paused;
        if (IsKeyPressed(KEY_ESCAPE)) selectedIdx = -1;

        /* Zoom around mouse cursor (viewport only) */
        Vector2 mouse = GetMousePosition();
        if (mouse.x < vpW) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) {
                Vector2 mouseWorld = GetScreenToWorld2D(mouse, camera);
                camera.offset = mouse;
                camera.target = mouseWorld;
                camera.zoom  *= 1.0f + wheel * CAMERA_ZOOM_STEP;
                if (camera.zoom < CAMERA_ZOOM_MIN) camera.zoom = CAMERA_ZOOM_MIN;
                if (camera.zoom > CAMERA_ZOOM_MAX) camera.zoom = CAMERA_ZOOM_MAX;
            }
        }

        /* Pan with right-click drag */
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && mouse.x < vpW) {
            Vector2 delta = GetMouseDelta();
            camera.target.x -= delta.x / camera.zoom;
            camera.target.y -= delta.y / camera.zoom;
        }

        /* Left-click in viewport: select nearest creature */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse.x < vpW) {
            Vector2 worldPos = GetScreenToWorld2D(mouse, camera);
            int    bestIdx  = -1;
            float  bestDist = 20.0f / camera.zoom;   /* pick radius scales with zoom */
            for (int i = 0; i < sim.creatureCount; i++) {
                if (!sim.creatures[i].alive) continue;
                float d = Vector2Distance(worldPos, sim.creatures[i].position);
                if (d < bestDist) { bestDist = d; bestIdx = i; }
            }
            selectedIdx = bestIdx;   /* -1 if clicked empty space */
        }

        /* Invalidate selection if creature died */
        if (selectedIdx >= 0 && !sim.creatures[selectedIdx].alive)
            selectedIdx = -1;

        /* ── Update ───────────────────────────────────────────── */
        if (!settings.paused) {
            int   steps  = settings.speedMult;
            float stepDt = GetFrameTime();
            for (int i = 0; i < steps; i++) {
                SimulationUpdate(&sim, stepDt, &settings);
            }
        }

        /* ── Draw ─────────────────────────────────────────────── */
        BeginDrawing();
            ClearBackground(BLACK);

            /* World — clipped to viewport */
            BeginScissorMode(0, 0, vpW, vpH);
                BeginMode2D(camera);
                    SimulationDraw(&sim);
                EndMode2D();

                /* NN inspector overlay (screen-space, inside scissor) */
                if (selectedIdx >= 0)
                    NNViewDraw(&sim.creatures[selectedIdx]);

            EndScissorMode();

            /* UI panel */
            UIDraw(&sim, &settings);

            /* Pause overlay */
            if (settings.paused) {
                DrawText("PAUSED  [SPACE]",
                         vpW / 2 - 70, vpH / 2, 20,
                         (Color){ 255, 220, 80, 200 });
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
