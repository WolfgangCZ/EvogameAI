# Evolution Simulation

A 2D evolution/genetics simulation built with Raylib in C.

## Prerequisites

- CMake 3.20+
- A C compiler (MSVC, GCC/MinGW, or Clang)
- Git (CMake uses it to fetch Raylib automatically)

## Build

CMake fetches Raylib on first configure — no manual download needed.

```sh
cmake -B build
cmake --build build --config Release
./build/Release/evo_sim.exe   # Windows (MSVC)
./build/evo_sim               # Linux / macOS
```

Or with the Makefile (requires `lib/raylib.h` + `lib/libraylib.a` placed manually):

```sh
make run
```

## Controls

| Key     | Action      |
|---------|-------------|
| SPACE   | Pause/Resume |

## Milestones

- [x] M1 — Window, world, random food spawn
- [ ] M2 — Creatures, movement, energy drain, food collection
- [ ] M3 — Death and asexual reproduction
- [ ] M4 — Genome → phenotype mapping
- [ ] M5 — Sexual reproduction and mutation
- [ ] M6 — Carnivores, predation, fleeing
- [ ] M7 — Charts and sliders UI
- [ ] M8 — Biomes, events, diseases
