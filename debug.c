#include "debug.h"

// Draw debug information
void DrawDebugInfo(int fps, int circleCount, float zoom) {
    DrawText("Debug Info:", 10, 10, 20, LIGHTGRAY);
    DrawText(TextFormat("FPS: %d", fps), 10, 35, 20, LIGHTGRAY);
    DrawText(TextFormat("Circles: %d", circleCount), 10, 60, 20, LIGHTGRAY);
    DrawText("Press 'F1' to toggle debug info", 10, 85, 20, LIGHTGRAY);
    DrawText(TextFormat("Zoom: %.2fx", zoom), 10, 110, 20, LIGHTGRAY);
    DrawText("Camera Controls:", 10, 135, 20, LIGHTGRAY);
    DrawText("- Arrow keys or middle mouse to move", 10, 160, 20, LIGHTGRAY);
    DrawText("- I/O keys or mouse wheel to zoom", 10, 185, 20, LIGHTGRAY);
} 