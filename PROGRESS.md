# Progress

## Milestone 1 — Window + World + Food ✅
- Dark green world with grid background
- Food pool (MAX_FOOD=600): 200 pre-seeded, 3 spawn/sec
- Sidebar: tick, food count
- SPACE to pause

## Milestone 2 — Creatures + Movement + Energy ✅
- 20 creatures spawn at random positions
- Seek nearest food within vision radius (100px), wander when none found
- Energy drain: metabolism (3/s) + movement cost; die at 0
- Body color: orange-red (starving) → cyan-blue (healthy), distinct from green food
- Sidebar: population count, total deaths

## Milestone 3 — Death + Reproduction ✅
- Asexual reproduction: energy > 80 triggers spawn of one offspring near parent
- Offspring inherit parent traits ±10% random variation (proto-mutation, no genome yet)
- Parent pays 50% energy cost; 8s cooldown before reproducing again
- Dead slots recycled so population can recover and grow
- Triangle FOV cone drawn per creature (faint cyan, 70° total, vision radius length)
- Sidebar: Born + Dead counters
## Milestone 4 — Genome + Neural Network ✅
- GENOME_SIZE=81 floats: 42 input→hidden weights, 6 hidden biases, 6 activation fn indices, 18 hidden→output weights, 3 output biases, 6 physical trait genes
- 8 activation functions per hidden node: linear, ReLU, sigmoid, tanh, sin, cos, abs, step
- NN inputs: food dist/angle (sin/cos), nearest creature dist/angle (sin), energy_norm, bias
- NN outputs: thrust (-1..1), turn (-1..1), reproduce trigger (0..1 sigmoid)
- All hardcoded steering removed — behavior emerges purely from genome
- Physical traits (size, speed, vision, metabolism, lifespan, mutation rate) genome-derived
- Asexual reproduction: self-crossover + mutation using genome's own mutation rate gene
- Death by energy starvation or old age (genome-driven lifespan 60–600s)
- **Redesigned to dynamic topology (simplified NEAT-style):** genome now stores traits as direct
  float fields and a connection list (NNConn conns[NN_CONN_MAX]) instead of a flat gene array.
  Creatures start with hiddenCount=0 (direct input→output connections, 40% sparsity) and gain
  hidden nodes via MutateAddHiddenNode (node-splitting). Up to NN_HIDDEN_MAX=8 hidden slots and
  NN_CONN_MAX=64 connections. Structural mutations (add connection, add node, change activation)
  fire at 25% of the weight mutation rate. nn_view.c updated for dynamic 2- or 3-column layout.

## Milestone 5 — Sexual Reproduction ⏭ (bonus, deferred)
## Milestone 6 — Carnivores + Predation 🔲
## Milestone 7 — Charts + UI Controls ✅
- UI_PANEL_WIDTH expanded to 300px; VIEWPORT_WIDTH adjusted automatically
- `SimSettings` struct (`include/settings.h` / `src/settings.c`): paused, speedMult(1..8), foodTarget(100..3000), mutRateMult(0.1..5.0)
- `History` ring buffer (`include/history.h` / `src/history.c`): 300 samples, recorded every 10 ticks
- `World.foodTarget` field — runtime food cap applied each update
- `SimulationUpdate` now takes `const SimSettings *settings` — applies foodTarget, mutRateMult, records history
- raygui PAUSE/RESUME button, Speed/Food/Mut sliders drawn in panel via `GuiButton`/`GuiSliderBar`
- Three 60px line charts (Population, Avg Speed, Avg Metabolism) stacked at bottom of panel
- Multi-step loop in `main.c`: runs `speedMult` sim steps per frame with proportional dt
- Known limit: chart Y-axis ranges are hardcoded (pop 0–500, speed 0–400, metabolism 0–10)

## Recent updates (post-milestone)
- **Genetic traits expanded:** `size` now scales `maxEnergy` and energy drain by `(size/6)²`; `visionAngle` added as FOV half-angle gene [0.1,π rad]; vision cost = `vision² × visionAngle × 0.000012` energy/s
- **FOV detection:** food/creature sensors now restricted to the creature's own FOV cone
- **Vision rendering:** full circular sector (`DrawCircleSector`) replaces flat triangle, subtle fill (α8) + outline (α30)
- **Population floor slider:** `minPopulation` (0–500, default 50) added to `SimSettings`; replaces hard-coded `MIN_POPULATION` constant; shown as **MinPop** slider in UI
- **Limits raised:** `MAX_CREATURES`=3000, `MAX_FOOD`=8000, `HISTORY_LEN`=2000, food slider 100–8000
- **Food graph:** fourth chart added (Food over time)
- **Stack overflow fix:** `Simulation` declared `static` in `main` to move ~3 MB struct off the stack
- **Resizable window:** `FLAG_WINDOW_RESIZABLE` enabled; viewport, scissor, camera pivot all use runtime `GetScreenWidth()`/`GetScreenHeight()`; default size raised to 1600×900
- **Larger world:** `WORLD_WIDTH`×`WORLD_HEIGHT` raised from 4000×3000 → **12000×9000**; vision range uncapped (naturally selected, min 10px)

## Performance optimization ✅
- **Spatial hash grid for food** (`World.foodGridHead/Next/Cell[GRID_CELL_COUNT]`): food sensing reduced from O(3000×8000)=24M ops → O(3000×~7)=21K ops per frame (~1000× faster)
- **Spatial hash grid for creatures** (static `s_crGridHead/Next` rebuilt each frame): neighbor sensing reduced from O(n²)=9M ops → O(n×density)=~3K ops per frame
- **Eating loop** also uses the food grid (3×3 cell query, eatRadius ~17px << 200px cell size)
- **Toroidal eating bug fixed**: eating now uses `TORUS_DELTA` for correct wrap-around distance
- **Cached `aliveCount`** on `Simulation` (incremented on birth, decremented on death detection); `SimulationAliveCount()` is now O(1) instead of O(n) — called ~5×/frame previously
- **Cached `foodCount`** on `World`; `WorldFoodCount()` is now O(1) instead of O(8000)
- **Squared-distance comparisons** in sensing loops: `sqrtf` called only once per creature (for the winner), not for every candidate
- **Death detection moved before eating/reproduction**: `aliveCount` is accurate during reproduction check
- **Speed slider raised to 20×** (was 8×)

## Recent tuning & polish
- **Vision cost raised to 0.00005/s** (was 0.000001 — 50× increase): large/wide FOV cones now impose meaningful evolutionary pressure; creatures are expected to converge on narrower or shorter vision under resource scarcity
- **CMake cache repaired**: stale `wvo2` paths in all `CMakeCache.txt` files patched; binary now outputs to `EvogameAI/build/Release/evo_sim.exe`
- **NN inspector layout redesigned** (`nn_view.c`): replaced hardcoded pixel offsets with a two-phase cursor-based layout — geometry computed first (`sepY`, `colLabelY`, `nodeAreaTop` derived from row count), then drawn once; panel height auto-sizes; adding/removing a row is now a single line
- **Energy cost row added to NN inspector**: shows `base`, `vis`, `move+`, and `max` drain/s computed from live genome values
- **INPUTS/OUTPUTS label overlap fixed**: column labels now anchored below the separator via `colLabelY`, not at a magic offset from `NODES_TOP`

## Milestone 8 — Biomes + Events 🔲
