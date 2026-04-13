#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "ui.h"
#include "simulation.h"
#include "settings.h"
#include "config.h"
#include "history.h"

#include <string.h>

/* ── Internal chart helper ───────────────────────────────────── */

/* Draw a line chart from a float ring buffer.
   data  : ring buffer array of length HISTORY_LEN
   count : number of valid samples
   head  : index of the NEXT write position (oldest sample at head when full)
   minV/maxV: value range for vertical scaling
   lineColor: color of the polyline
   label : short text shown in top-left corner of chart */
static void DrawLineChart(int x, int y, int w, int h,
                           const float *data, int count, int head,
                           float minV, float maxV,
                           Color lineColor, const char *label)
{
    /* Dark background */
    DrawRectangle(x, y, w, h, (Color){ 10, 10, 20, 240 });
    /* Border */
    DrawRectangleLines(x, y, w, h, (Color){ 60, 60, 80, 200 });
    /* Label */
    DrawText(label, x + 3, y + 2, 10, (Color){ 160, 160, 180, 255 });

    if (count < 2) return;

    float range = maxV - minV;
    if (range < 0.0001f) range = 0.0001f;

    int   samples  = (count < HISTORY_LEN) ? count : HISTORY_LEN;
    /* oldest sample index in the ring */
    int   startIdx = (count < HISTORY_LEN) ? 0 : head;

    float prevX = 0.0f, prevY = 0.0f;

    for (int i = 0; i < samples; i++) {
        int   idx   = (startIdx + i) % HISTORY_LEN;
        float val   = data[idx];
        float normX = (float)i / (float)(HISTORY_LEN - 1);
        float normY = (val - minV) / range;
        if (normY < 0.0f) normY = 0.0f;
        if (normY > 1.0f) normY = 1.0f;

        float px = (float)x + normX * (float)(w - 1);
        float py = (float)(y + h - 1) - normY * (float)(h - 3);

        if (i > 0) {
            DrawLine((int)prevX, (int)prevY, (int)px, (int)py, lineColor);
        }
        prevX = px;
        prevY = py;
    }
}

/* Overload for integer ring buffers — converts to float on-the-fly */
static void DrawLineChartInt(int x, int y, int w, int h,
                              const int *data, int count, int head,
                              float minV, float maxV,
                              Color lineColor, const char *label)
{
    /* Dark background */
    DrawRectangle(x, y, w, h, (Color){ 10, 10, 20, 240 });
    /* Border */
    DrawRectangleLines(x, y, w, h, (Color){ 60, 60, 80, 200 });
    /* Label */
    DrawText(label, x + 3, y + 2, 10, (Color){ 160, 160, 180, 255 });

    if (count < 2) return;

    float range = maxV - minV;
    if (range < 0.0001f) range = 0.0001f;

    int samples  = (count < HISTORY_LEN) ? count : HISTORY_LEN;
    int startIdx = (count < HISTORY_LEN) ? 0 : head;

    float prevX = 0.0f, prevY = 0.0f;

    for (int i = 0; i < samples; i++) {
        int   idx   = (startIdx + i) % HISTORY_LEN;
        float val   = (float)data[idx];
        float normX = (float)i / (float)(HISTORY_LEN - 1);
        float normY = (val - minV) / range;
        if (normY < 0.0f) normY = 0.0f;
        if (normY > 1.0f) normY = 1.0f;

        float px = (float)x + normX * (float)(w - 1);
        float py = (float)(y + h - 1) - normY * (float)(h - 3);

        if (i > 0) {
            DrawLine((int)prevX, (int)prevY, (int)px, (int)py, lineColor);
        }
        prevX = px;
        prevY = py;
    }
}

/* ── Public API ──────────────────────────────────────────────── */

void UIDraw(const Simulation *s, SimSettings *settings) {
    const int panelX = GetScreenWidth() - UI_PANEL_WIDTH;
    const int panelW = UI_PANEL_WIDTH;
    const int screenH = GetScreenHeight();

    /* Panel background */
    DrawRectangle(panelX, 0, panelW, screenH, (Color){ 20, 20, 30, 255 });
    DrawLine(panelX, 0, panelX, screenH, (Color){ 60, 60, 80, 255 });

    int px = panelX + 8;
    int py = 10;
    const int lineH  = 18;
    const int gap    = 8;

    /* ── Title ───────────────────────────────────────────────── */
    DrawText("EVO SIM", px, py, 16, (Color){ 180, 220, 180, 255 });
    py += 22;

    /* ── Stats ───────────────────────────────────────────────── */
    DrawText(TextFormat("Tick: %d",  s->world.tick),               px, py, 12, LIGHTGRAY); py += lineH;
    DrawText(TextFormat("Pop:  %d",  SimulationAliveCount(s)),     px, py, 12, LIGHTGRAY); py += lineH;
    DrawText(TextFormat("Food: %d",  WorldFoodCount(&s->world)),   px, py, 12, LIGHTGRAY); py += lineH;
    DrawText(TextFormat("Born: %d",  s->totalBirths),              px, py, 12, (Color){ 100, 210, 255, 255 }); py += lineH;
    DrawText(TextFormat("Dead: %d",  s->totalDeaths),              px, py, 12, (Color){ 220, 100, 100, 255 }); py += lineH;

    py += gap;
    DrawLine(panelX, py, panelX + panelW, py, (Color){ 60, 60, 80, 180 });
    py += gap + 2;

    /* ── Controls ────────────────────────────────────────────── */

    /* Pause / Resume button */
    if (GuiButton((Rectangle){ (float)px, (float)py, (float)(panelW - 16), 24.0f },
                  settings->paused ? "RESUME" : "PAUSE")) {
        settings->paused = !settings->paused;
    }
    py += 30;

    /* Speed multiplier slider (1..20, integer steps) */
    float speedF = (float)settings->speedMult;
    GuiSliderBar((Rectangle){ (float)(px + 50), (float)py, (float)(panelW - 116), 16.0f },
                 "Speed",
                 TextFormat("x%d", settings->speedMult),
                 &speedF, 1.0f, 20.0f);
    settings->speedMult = (int)(speedF + 0.5f);
    if (settings->speedMult < 1)  settings->speedMult = 1;
    if (settings->speedMult > 20) settings->speedMult = 20;
    py += 26;

    /* Food max cap slider (100..8000) */
    float foodF = (float)settings->foodTarget;
    GuiSliderBar((Rectangle){ (float)(px + 50), (float)py, (float)(panelW - 116), 16.0f },
                 "FoodMax",
                 TextFormat("%d", settings->foodTarget),
                 &foodF, 100.0f, 8000.0f);
    settings->foodTarget = (int)(foodF + 0.5f);
    if (settings->foodTarget < 100)  settings->foodTarget = 100;
    if (settings->foodTarget > 8000) settings->foodTarget = 8000;
    py += 26;

    /* Food spawn rate slider (0.5..20.0 items/sec) */
    GuiSliderBar((Rectangle){ (float)(px + 50), (float)py, (float)(panelW - 116), 16.0f },
                 "FoodRate",
                 TextFormat("%.1f/s", settings->foodSpawnRate),
                 &settings->foodSpawnRate, 0.5f, 100.0f);
    if (settings->foodSpawnRate < 0.5f)  settings->foodSpawnRate = 0.5f;
    if (settings->foodSpawnRate > 100.0f) settings->foodSpawnRate = 100.0f;
    py += 26;

    /* Mutation rate multiplier (0.1..5.0) */
    GuiSliderBar((Rectangle){ (float)(px + 50), (float)py, (float)(panelW - 116), 16.0f },
                 "Mut",
                 TextFormat("%.1fx", settings->mutRateMult),
                 &settings->mutRateMult, 0.1f, 5.0f);
    if (settings->mutRateMult < 0.1f) settings->mutRateMult = 0.1f;
    if (settings->mutRateMult > 5.0f) settings->mutRateMult = 5.0f;
    py += 26;

    /* Min population floor slider (0..500) */
    float minPopF = (float)settings->minPopulation;
    GuiSliderBar((Rectangle){ (float)(px + 50), (float)py, (float)(panelW - 116), 16.0f },
                 "MinPop",
                 TextFormat("%d", settings->minPopulation),
                 &minPopF, 0.0f, 500.0f);
    settings->minPopulation = (int)(minPopF + 0.5f);
    if (settings->minPopulation < 0)   settings->minPopulation = 0;
    if (settings->minPopulation > 500) settings->minPopulation = 500;
    py += 26;

    py += gap;
    DrawLine(panelX, py, panelX + panelW, py, (Color){ 60, 60, 80, 180 });
    py += gap + 2;

    /* ── Charts ──────────────────────────────────────────────── */
    const int chartW = panelW - 16;
    const int chartH = 60;

    const History *h = &s->history;

    DrawLineChartInt(
        px, py, chartW, chartH,
        h->population, h->count, h->head,
        0.0f, (float)MAX_CREATURES,
        (Color){ 100, 210, 100, 255 }, "Population");
    py += chartH + 6;

    DrawLineChart(
        px, py, chartW, chartH,
        h->avgSpeed, h->count, h->head,
        0.0f, 400.0f,
        (Color){ 100, 180, 255, 255 }, "Avg Speed");
    py += chartH + 6;

    DrawLineChart(
        px, py, chartW, chartH,
        h->avgMetabolism, h->count, h->head,
        0.0f, 10.0f,
        (Color){ 255, 160, 80, 255 }, "Avg Metabolism");
    py += chartH + 6;

    DrawLineChartInt(
        px, py, chartW, chartH,
        h->food, h->count, h->head,
        0.0f, 8000.0f,
        (Color){ 80, 200, 80, 255 }, "Food");

    (void)py;  /* suppress unused-variable warning */
}
