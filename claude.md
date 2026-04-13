# Agent Rules for Evolution Simulation Project

## Purpose
This file sets project-specific agent behavior rules and workflow expectations while developing the Raylib C evolution/genetics simulation.

## Generic C-language rules
1. Prefer standard C code (C11 or later) and avoid non-portable compiler extensions.
2. Keep code modular with clear header/source separation (`*.h` / `*.c`).
3. Use `typedef struct` for data types and avoid global mutable state except where explicitly controlled.
4. Handle memory allocation carefully; avoid leaks and use defensively safe patterns with clear ownership.
5. Provide clear comment documentation for each public function and data struct.
6. Favor readability: consistent naming, indentation, and minimal nesting.
7. Use asserts and error-handling for invariants (e.g., bounds, null checks).
8. With Raylib, use `Vector2` and framing utilities idiomatically, and keep rendering code in `ui.c`.
9. Keep clean folder structure

## Development workflow expectations
1. Work in incremental steps; deliver one key feature or module per iteration.
2. Indicate explicitly when you want me to run and test the game in my environment.
3. Include implementation notes and any known limitations for the current step.
4. For any failing build or run step, provide a concise bug summary and immediate fix plan.
5. Track progress in `PROGRESS.md` after each step

## Testing and verification
1. Keep `README` or plan updated with run instructions and expected behavior.
2. Log any manual test results after each step.

## Communication
- You (assistant) must produce short, precise progress updates in markdown.
- Do not proceed to the next major feature until confirmation that the current step works.
- Keep answers short and technical
- Always suggest the next most valuable step.
