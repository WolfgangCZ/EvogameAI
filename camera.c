#include "camera.h"
#include "config.h"

// Initialize camera with default settings
Camera2D initialize_camera(void) {
    Camera2D camera = { 0 };
    camera.target = (Vector2){ SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };  // Camera target (rotation and zoom origin)
    camera.offset = (Vector2){ SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };  // Camera offset (displacement from target)
    camera.rotation = 0.0f;  // Camera rotation in degrees
    camera.zoom = 1.0f;      // Camera zoom (scaling)
    return camera;
}

// Update camera position based on input
void update_game_camera(Camera2D* camera, float delta_time) {
    // Handle keyboard movement
    float move_speed = CAMERA_MOVE_SPEED * delta_time;
    if (IsKeyDown(KEY_LEFT)) {
        camera->target.x += move_speed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        camera->target.x -= move_speed;
    }
    if (IsKeyDown(KEY_UP)) {
        camera->target.y += move_speed;
    }
    if (IsKeyDown(KEY_DOWN)) {
        camera->target.y -= move_speed;
    }

    // Handle middle mouse button drag
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        // Scale the movement by the current zoom level (slower movement when zoomed in)
        delta.x /= camera->zoom;
        delta.y /= camera->zoom;
        // Move in the opposite direction of the mouse movement
        camera->target.x -= delta.x;
        camera->target.y -= delta.y;
    }
}

// Update camera zoom based on input
void update_camera_zoom(Camera2D* camera, float delta_time) {
    // Handle keyboard zoom
    if (IsKeyDown(KEY_I)) {  // I key for zoom in
        camera->zoom += ZOOM_SPEED * delta_time;
        if (camera->zoom > MAX_ZOOM) camera->zoom = MAX_ZOOM;
    }
    if (IsKeyDown(KEY_O)) {  // O key for zoom out
        camera->zoom -= ZOOM_SPEED * delta_time;
        if (camera->zoom < MIN_ZOOM) camera->zoom = MIN_ZOOM;
    }

    // Handle mouse wheel zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        // Get the world point that is under the mouse before zooming
        Vector2 mouse_world_pos = GetScreenToWorld2D(GetMousePosition(), *camera);
        
        // Zoom increment
        const float zoom_increment = 0.125f;
        camera->zoom += (wheel * zoom_increment);
        
        // Clamp zoom
        if (camera->zoom > MAX_ZOOM) camera->zoom = MAX_ZOOM;
        if (camera->zoom < MIN_ZOOM) camera->zoom = MIN_ZOOM;
        
        // Get the new world point under the mouse
        Vector2 new_mouse_world_pos = GetScreenToWorld2D(GetMousePosition(), *camera);
        
        // Adjust target to keep the point under the mouse fixed
        camera->target.x += (mouse_world_pos.x - new_mouse_world_pos.x);
        camera->target.y += (mouse_world_pos.y - new_mouse_world_pos.y);
    }
} 