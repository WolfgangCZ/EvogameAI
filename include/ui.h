#pragma once

#include "raylib.h"
#include "simulation.h"
#include "settings.h"

/* Draw the right-side UI panel: stats, raygui controls, line charts */
void UIDraw(const Simulation *s, SimSettings *settings);
