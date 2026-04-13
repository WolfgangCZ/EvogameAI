# Evolutionary Genetics Simulation in Raylib (C)

## Overview
A 2D simulation where autonomous creatures compete for resources, eat food or each other, reproduce, and evolve over time via genetic inheritance and mutation.

Objectives:
- emergent behavior from simple trait-based rules
- real-time interactive visualization with Raylib
- evolutionary pressure through resource scarcity and predation
- genetic complexity (crossover, mutation, phenotype mapping)

---

## ⚠️ CORE DESIGN PRINCIPLE — Genome-Driven Neural Behavior (NON-NEGOTIABLE)

**Every behavior a creature exhibits must emerge purely from its genome. There must be no hardcoded decision logic such as "if food nearby then move toward it". Creatures must figure out survival entirely on their own through evolution.**

### How it works

Each creature has a small neural network (NN) whose **structure and weights are fully encoded in the genome**. The NN is randomly wired at birth — most creatures will be useless at first, but selection pressure will favor those whose random wiring happens to produce useful behavior.

#### Inputs (sensory genes)
Each input neuron corresponds to a sensor gene:
- Nearest food: distance, angle relative to facing direction
- Nearest creature: distance, angle, size delta (am I bigger?)
- Own energy level (normalized)
- Own age (normalized)
- Random bias neuron (always 1.0 — allows constant outputs)

#### Hidden layer
- Fixed number of hidden neuron slots (e.g. 6), but each slot has a genome-encoded **activation function index**
- Activation function is chosen randomly at birth and can mutate to a different one
- Available activation functions (each is a valid gene expression):
  - `linear` (identity)
  - `ReLU` — `max(0, x)`
  - `sigmoid` — `1 / (1 + e^-x)`
  - `tanh`
  - `sin` — allows oscillatory / rhythmic behavior to emerge
  - `cos`
  - `abs` — `|x|`
  - `step` — `x > 0 ? 1 : 0` (binary trigger)
- A creature may have all hidden nodes using `sin` and produce rhythmic walking; another may use `ReLU` chains and produce sharp reactive behavior — both are valid, neither is designed
- Weights, biases, and activation function indices all encoded as genome floats

#### Outputs (motor genes)
Each output neuron drives one actuator:
- Move forward/backward (thrust)
- Turn left/right (angular velocity)
- Reproduction trigger (fires when output > threshold)

#### Genome encodes
- All NN weights and biases (the core of the genome)
- Physical traits: body size, max speed, vision range, metabolism rate, lifespan
- Mutation rate (self-evolving)

#### What this means in practice
- A creature with random genome will move randomly or not at all
- A creature whose NN happens to wire "food detected → move forward" will eat and survive
- Over generations, selection will favor better-wired NNs
- No gene says "eat food" — eating happens automatically when close enough (physics), but **moving toward food must be learned by the NN**
- Reproduction is triggered by an output neuron, not a hardcoded energy threshold

### UI library
- **raygui** (https://github.com/raysan5/raygui, tag 4.0) — single-header pure-C immediate-mode UI built for Raylib
- Used for all controls: sliders, buttons, labels, panels, checkboxes
- Fetched automatically via CMake FetchContent — no manual setup
- No other UI library; all neural network visualization is drawn directly with Raylib primitives

### Neural network visualization (click-to-inspect)
When the player clicks on a creature in the world, a panel overlays the UI sidebar showing that creature's neural network:
- Input nodes on the left, hidden nodes in the middle, output nodes on the right
- Nodes drawn as circles using Raylib draw calls; label shows node type (e.g. "food dist", "energy", "thrust")
- Hidden nodes labeled with their activation function name (e.g. "sin", "ReLU")
- Edges drawn as lines; line thickness encodes weight magnitude; color encodes sign (blue = positive, red = negative)
- Active firing values shown as brightness on each node (updated live each tick)
- Clicking elsewhere or pressing ESC deselects the creature
- Genome hex string displayed at bottom of inspector — compact binary encoding of all traits + topology + weights, shown as uppercase hex. Enables copy/share/seed of exact creatures.

### Implementation note
Current M1–M3 code uses hardcoded steering and a fixed energy threshold for reproduction. **This must be replaced entirely when the NN is introduced (Milestone 4 rewrite).** The placeholder traits (speed, vision, metabolism) become physical parameters mapped from genome floats; behavior comes from the NN weights.

---

## Project Structure (C)
- `main.c` - app lifecycle, raylib setup, UI loop
- `world.h/c` - environment, food spawning, global parameters
- `creature.h/c` - creature state, motion, satisfaction, death
- `genome.h/c` - genome representation, crossover, mutation, phenotype mapping
- `simulation.h/c` - update tick, interactions, reproduction, ecology
- `ui.h/c` - draw maps, stats, controls
- `config.h` - tunable constants and initial conditions

## Core Data Types
### Creature
- `Vector2 position, velocity`
- `float energy, health, age`
- `Genome genome`
- `float size, speed, vision, aggression, metabolism, carnivory`
- `bool alive`
- `int id`
- `int reproductionCooldown`

### Genome
- `float genes[GENE_COUNT]` (values 0..1)
- mapped to traits
- include mutation propensity

### Food
- `Vector2 position`
- `float nutrition`
- `bool eaten`

### World
- `int width, height`
- `List<Food> plants, meat`
- `float foodSpawnRate`
- `int tick`

### Stats
- population size
- average trait values
- births/deaths per tick
- generation count

## Simulation Loop
1. Handle input
   - pause/play
   - sliders: mutation rate, food rate, speed multiplier
   - export stats
2. Update creatures
   - sense nearest food and creatures within vision
   - decide (eat, attack, flee, explore, reproduce)
   - move (steering, obstacles, wrap/bound)
   - consume energy (`metabolism`, speed)
3. Interactions
   - eating food
   - attack/defense outcome (based on size/aggression)
   - reproduction if conditions
4. Reproduction
   - sexual or asexual (configurable)
   - crossover + mutation in `genome.c`
   - energy cost and cooldown
5. Environment
   - spawn plant food at rate
   - decays/meat disappears
6. Death
   - energy <= 0 or age > maxAge
   - corpse turns into meat source
7. Record stats

## Genetics and Traits
- Genes as normalized float array:
  - speed, metabolism, sizeDominant, vision, aggression, carnivory, social, mutationRate, lifespan
- Trait mapping functions:
  - `speed = LERP(min,max, gene[SPEED])`
  - `vision = base + gene[VISION]*max` etc.
- Reproduction:
  - select parents in local neighborhood
  - uniform crossover or single-point
  - per-gene mutation chance from gene[MUTATION_RATE]
- optional allele specifics:
  - dominance, pleiotropy, epistasis (later)

## Behavior AI
- goal weights from phenotype:
  - hungerLevel => food seeking
  - aggression => attack random smaller prey if carnivory > threshold
  - fear => run from bigger aggressors
  - mating drive when energy high
- decision tree per tick

## UI
- world view render:
  - creatures as colored circles (trait-based colors)
  - vision arcs optionally
  - food as small squares
  - meat as darker spots
- dashboard on side:
  - population, births, deaths, avg traits
  - buttons and sliders
- graph area with history (line plots)

## Development Milestones

| # | Feature | Status |
|---|---------|--------|
| 1 | Raylib window + world + food spawn | ✅ done |
| 2 | Creatures, movement, energy drain | ✅ done |
| 3 | Death + asexual reproduction | ✅ done |
| 4 | NEAT-style genome + neural network | ✅ done |
| 5 | Sexual reproduction | ⏭ deferred |
| 6 | Carnivores + predation + fleeing | 🔲 next |
| 7 | Charts + UI controls (sliders, history) | ✅ done |
| 8 | Biomes + environmental events | 🔲 planned |
| 9 | Advanced genetics (epistasis, sexual selection) | 🔲 future |

### What diverged from the original design
- **Genome**: not a flat float array. Uses direct trait fields + a NEAT-style dynamic connection list (`NNConn conns[64]`), up to 8 hidden nodes. Structural mutations (add node, add connection, change activation) fire at 25% of weight-mutation rate.
- **Spatial partitioning**: 200×200 px uniform grid already implemented (M1–M7 had O(n²) sensing; now O(n × local density)). See `GRID_CELL_SIZE` in `config.h`.
- **World size**: 12 000 × 9 000 px toroidal (was originally smaller).
- **Vision cost**: `vision² × visionAngle × 0.00005 /s` — creates genuine evolutionary trade-off between sensing range and energy budget.
- **NN inspector**: live panel showing all node values, weights, and per-second energy cost breakdown. Opened by left-clicking a creature.

### Milestone 6 — Carnivores + Predation (next)
Planned additions:
- `carnivory` gene [0,1] shifts creature role from herbivore → carnivore
- Attack: carnivore within range deals `attackPower × dt` damage to smaller creature; winner gains energy proportional to prey energy
- Flee: prey creature senses attacker via existing neighbor sensor; `CrtSin`/`CrtDst` inputs already wired
- Corpse food: dead creatures leave a meat item (higher nutrition, limited lifetime) that carnivores prefer
- Color: carnivore tint shifts toward red with `carnivory` gene value

### Milestone 8 — Biomes + Events (planned)
- Temperature/fertility zones affecting food density and metabolic multiplier
- Seasonal cycles: food abundance oscillates on a configurable period
- Random events: drought (food rate → 0 for N ticks), disease wave (energy drain spike in a region)

## Build and Run
- CMake + FetchContent (auto-downloads raylib 5.5 and raygui 4.0):
  ```
  cmake -S . -B build -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
  cmake --build build --config Release
  build\Release\evo_sim.exe
  ```

## Advanced Complexity Enhancements
- Niche specialization:
  - assign each creature an ecosystem role from genes (herbivore, carnivore, scavenger, omnivore) across an energy spectrum.
  - support resource partitioning (plant type A/B, protein vs carbs) and efficiency tradeoffs.
- Adaptive traits and plasticity:
  - express phenotype via gene+environment interaction (epigenetic-like influence): e.g., `metabolism` increases when scarce food is detected.
  - allow reversible temporary traits from environment (e.g., camouflage, hibernation, schooling tendency).
- Multi-layer genetics:
  - `GENE_COUNT` includes regulatory network with weights controlling behavior and environmental responses.
  - implement epistasis: gene interactions where one gene modulates expression of another (e.g., size affects speed scale).
  - add per-gene dominance/recessive with allele pairs (float for two-allele trait, blended and nondeterministic for meiosis selection).
- Sexual selection and mating behavior:
  - render sexes and mating displays driven by visual traits, enabling sexual dimorphism.
  - implement mate choice: prefer on traits (color, size, health) with probabilistic selection.
- Genetic drift and mutation spectrum:
  - gene-specific mutation rates plus rare macro-mutation events (trait inversion, duplication, deletion).
  - include `transposon` factor where chunks of genome scramble occasionally.
- Co-evolution and arms race:
  - implement dynamic defense traits (armor, venom) and predation countertraits; fitness landscapes shift as average population evolves.
- Environmental dynamics:
  - multizone biome map (temperature, humidity, fertility) with migration cost.
  - seasonal cycles, catastrophes (fires, droughts, pathogen outbreak) forcing adaptation.
  - dynamic plant cycle (germination, maturity, senescence, nutrient value change).
- Ecology and population-level effects:
  - add disease/pathogen that spreads by proximity and can be resisted by immune trait genes.
  - support stable and unstable equilibrium dynamics (boom/bust cycles), with carrying capacity and density-dependent mortality.
  - implement `keystone species` and food-web graph, carrying energy flow metrics (trophic levels, productivity).
- Evolutionary metrics and analysis:
  - real-time line plots for variance, covariance, heritability score, Q90 quantiles, allele frequency.
  - allow data export to CSV for offline analysis.
- Performance complexity:
  - spatial partitioning (quadtrees) with separate update rules per zone.
  - multithread simulation and rendering separation (optional, with lock-free event state).

## Developer Features (Debug & UI)
- Debug mode with configurable compile-time and runtime switches:
  - `DEBUG_AI` to visualize perception lines and decision vectors.
  - `DEBUG_COLLISION` to show bounding circles and spatial partition buckets.
  - `DEBUG_STATS` to print log surface debug traces and occasional breakpoints.
- In-game console overlay:
  - toggled with `F1` key
  - command interface for spawn, kill, set parameters (`set foodRate 1.2`, `spawn creature 10`).
- Rewind and fast-forward:
  - local frame buffer with state snapshots, `Ctrl+Z` for undo tick, `Ctrl+Y` for redo.
- Deterministic replay:
  - seed-based RNG and event log to reproduce runs exactly.
  - `record` / `playback` file output for research.
- Visual debugging tracers:
  - show nearest food/target lines, dominant stimulus arrow, awareness radius.
  - highlight body traits via dynamic color coding (metabolism, aggression, immunity level).
- Config file reloading:
  - `config.ini` or JSON with runtime hot-reload of simulation constants.
- Performance profiling overlay:
  - show tick time breakdown: AI, movement, collision, rendering.
  - simple FPS / creature count / memory usage section.
- UI panels for fast tuning:
  - sliders and numeric values for all major constants:
    - spawn rates, mutation, carnivory weight, max population.
  - pause/time-step frame-by-frame mode.
  - save/load world state presets.
- Editor-style plugin hooks:
  - callbacks to inject plugins in `simulation.c` (e.g., custom behavior from script functions).

## Notes
- Keep loops efficient; use spatial partitioning (grid/QUAD-tree) if >1000 creatures.
- Use fixed timestep for simulation stability.
- Make random seed configurable.
