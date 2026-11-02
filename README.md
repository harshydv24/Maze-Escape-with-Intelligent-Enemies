# Maze Escape with Intelligent Enemies

A 2D procedural maze game written in C with raylib. The player collects the
required keys, avoids pathfinding enemies, and reaches the exit before the
level timer expires.

## Features

- Recursive-backtracking maze generation
- Five progressively larger levels
- Easy, Normal, and Hard difficulty modes
- Patrol, BFS Hunter, and A* Hunter enemies
- Enemy state machine with patrol, chase, search, and return behavior
- Fog of war, power-ups, and health
- Full-screen display requested at 1920 x 1080, with a centered camera that fits the entire maze
- Save/load and persistent high scores
- AI comparison demo for Random, Greedy, DFS, BFS, and A*
- Debug overlay for enemy paths and AI state

## Controls

| Key | Action |
| --- | --- |
| W/A/S/D or arrow keys | Move |
| Left Shift | Sprint while moving (uses stamina) |
| Q | Throw a decoy (two per level) |
| Escape | Pause or go back |
| F1 | Toggle debug overlay |
| F2 | Toggle fog of war |
| Enter | Select or confirm |

While moving, a newly pressed direction takes effect immediately when the
player can safely move that way. Blocked directions are ignored; the player
continues until reaching a wall or junction.

The main menu contains New Game, Continue, High Scores, Instructions, Settings,
AI Demo, and Quit. Settings control sound volume, music volume, FPS display,
and fog of war. Settings are available from both the main menu and pause menu.

Explore dead ends for an optional glowing secret cache worth 500 points. Enemy
spotted cues, movement and pickup sounds, damage flashes, light camera shake,
and animated pickup feedback make important events easier to read. Audio files
are optional; see [assets/sounds/README.md](assets/sounds/README.md) for exact
filenames and locations.

Player, enemy, key, and power-up icons can use separate transparent PNG files.
Place them in `assets/images/`; see [assets/images/README.md](assets/images/README.md)
for the exact filenames. Missing images use the built-in shape icons.
UI text can use a TrueType font at `assets/fonts/game.ttf`; see
[assets/fonts/README.md](assets/fonts/README.md). If it is absent, raylib's
built-in font is used.

## Difficulty

Difficulty affects maze size, timer, enemy speed, detection range, and vision.
Enemy counts also vary by difficulty and increase over later levels:

| Mode | Maze | Initial enemies | Timer | Vision |
| --- | --- | ---: | ---: | ---: |
| Easy | 21 x 21 | 1 | 180 seconds | 8 cells |
| Normal | 31 x 31 | 2 | 120 seconds | 6 cells |
| Hard | 41 x 41 | 3 | 90 seconds | 4 cells |

Maze size grows by four cells per level, capped at 51 x 51. Enemy counts are
capped at four.

## Save Files

Save data is stored in `saves/save.dat` and high scores in
`saves/highscores.dat`, relative to the process working directory.

The save file currently stores the level, difficulty, score, timer, key counts,
player position and health, maze cells, key positions, enemy positions/types/
states, power-up positions/types/collection state, and explored cells. It does
not store transient power-up timers, enemy paths, patrol progress, camera state,
or debug/settings state. It stores remaining decoys and whether the secret cache
has been collected. The save format version was updated for these new fields.

Malformed or truncated save and high-score files are rejected safely.

## Build

The project expects w64devkit GCC and raylib at the paths configured in the
Makefile. From a shell with `make` available:

```text
make
make test
make test-sanitize
make run
```

Build locations can be overridden without editing the Makefile:

```text
make CC=gcc RAYLIB=/path/to/raylib
```

`make test-sanitize` requires a compiler with AddressSanitizer and
UndefinedBehaviorSanitizer runtimes installed. The bundled w64devkit compiler
may not include those runtimes.

The tests cover maze generation, object placement, queue/heap behavior,
pathfinding, malformed saves, save round-trips, and malformed leaderboards.

## Project Layout

```text
include/  Public headers and shared types
src/      Game, maze, AI, rendering, audio, save, and score modules
tests/    Standalone C test runners
assets/   Optional sound and music assets
saves/    Runtime save and high-score files
```

