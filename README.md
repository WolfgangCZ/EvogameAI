# Evolution Game

A simple 2D game built with raylib where you control circles that interact with yellow rectangles in an evolving environment.

## Features

- Control multiple circles with different behaviors
- Circles can be selected and controlled individually
- Yellow rectangles that respawn when hit by circles or clicked
- Camera controls for zooming and panning
- Grid system for visual reference
- Debug information display
- Smooth physics with friction

## Controls

### Circle Controls
- `Left Mouse Button`: Select a circle
- `W/S`: Increase/Decrease selected circle's speed
- `A/D`: Rotate selected circle's facing direction
- `Space`: Spawn a new circle
- `R`: Reset all circles to initial positions
- `Tab`: Switch between selected circles

### Camera Controls
- `Mouse Wheel`: Zoom in/out
- `Right Mouse Button + Drag`: Pan camera
- `Right Mouse Button + Mouse Wheel`: Zoom in/out

### Rectangle Interaction
- `Left Mouse Button`: Click on a yellow rectangle to make it respawn at a random location
- Circles will automatically make rectangles respawn on collision

## Building

### Prerequisites
- CMake
- A C compiler (gcc, clang, or MSVC)
- raylib library

### Build Steps
1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Configure with CMake:
   ```bash
   cmake ..
   ```

3. Build the project:
   ```bash
   cmake --build . --config Release
   ```

4. Run the game:
   ```bash
   ./Release/SimpleRaylibGame
   ```

## Game Elements

### Circles
- Green circles that can be controlled
- Each circle has:
  - Position
  - Speed
  - Facing direction (indicated by a dot)
  - Fixed size (15 units radius)
  - Friction for smooth movement

### Rectangles
- Yellow rectangles that add interactivity
- Fixed size (1.5x circle radius)
- Respawn at random locations when:
  - Hit by a circle
  - Clicked by the player
- Drawn behind circles for better visibility

### Environment
- Grid system for visual reference
- Camera controls for exploring the game world
- Debug information showing selected circle's properties

## Development

The project is structured into several components:
- `main.c`: Main game loop and initialization
- `circle.c/h`: Circle management and physics
- `rectangle.c/h`: Rectangle management and collision detection
- `config.h`: Game configuration and constants

## License

This project is open source and available under the MIT License. 