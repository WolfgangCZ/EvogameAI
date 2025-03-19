#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"

// Initialize camera with default settings
Camera2D initialize_camera(void);

// Update camera position based on input
void update_game_camera(Camera2D* camera, float delta_time);

// Update camera zoom based on input
void update_camera_zoom(Camera2D* camera, float delta_time);

#endif // CAMERA_H 