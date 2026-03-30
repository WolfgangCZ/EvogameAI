#pragma once

#include "creature.h"

/* Draw the neural network inspector overlay for the selected creature.
   Call in screen-space (outside BeginMode2D). */
void NNViewDraw(const Creature *c);
