# Simple Raylib Game

A simple game built with raylib that features bouncing circles with physics and collision detection.

## Features

- Multiple bouncing circles with random attributes
- Realistic physics and collision detection
- Dynamic circle creation
- Debug mode with movement indicators and performance metrics
- Smooth animations and colorful visuals

## Controls

- **A Key (Hold)**: Create new circles at the mouse cursor position
- **D Key**: Toggle debug mode
  - Shows direction indicators for each circle
  - Displays FPS counter
  - Shows current number of circles
  - Displays helpful instructions

## Building

### Prerequisites

- CMake (version 3.10 or higher)
- A C compiler (GCC, Clang, or MSVC)
- Git (for cloning the repository)

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/evogame.git
   cd evogame
   ```

2. Create a build directory and navigate to it:
   ```bash
   mkdir build
   cd build
   ```

3. Generate the build files:
   ```bash
   cmake ..
   ```

4. Build the project:
   ```bash
   cmake --build . --config Release
   ```

The executable will be created in the `build/Release` directory.

## Running the Game

After building, you can run the game by executing the generated executable:

```bash
./Release/SimpleRaylibGame
```

## Game Settings

- Initial number of circles: 10
- Maximum number of circles: 10000
- Circle radius range: 4-8 pixels
- Circle speed range: 2-5 pixels per frame
- Spawn rate: 50 circles per second (while holding 'A' key)

## License

This project is licensed under the MIT License - see the LICENSE file for details. 