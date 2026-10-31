# Complete Development Prompt — Maze Escape with Intelligent Enemies

You are an experienced C game programmer and software architect.

I need you to develop a complete **2D top-down maze escape game in C using the raylib library**.

The project is for my **Advanced C Programming** college subject, so the project must demonstrate advanced C programming concepts, data structures, algorithms, dynamic memory management, modular programming, file handling, and AI/pathfinding.

The game should be polished enough for a college project demonstration while keeping the implementation understandable enough for me to explain during a viva.

---

# 1. Project Title

**Maze Escape with Intelligent Enemies**

Subtitle:

**A 2D Procedural Maze Game with AI-Based Enemy Pathfinding**

---

# 2. Technology Requirements

Use:

- Programming language: **C**
- Graphics/input/audio: **raylib**
- Compiler: GCC / MinGW
- Build system: Makefile
- Platform: Windows primarily, but keep the code portable where reasonably possible

Do NOT use:

- C++
- Unity
- Unreal Engine
- SDL unless absolutely necessary
- External AI/pathfinding libraries
- External maze-generation libraries
- External data-structure libraries

All important algorithms and data structures must be implemented manually in C.

raylib should primarily be used for:

- Window creation
- Rendering
- Keyboard input
- Mouse input where necessary
- Audio
- Fonts/text
- Timing
- Basic graphics

The actual game logic, maze generation, enemy AI, pathfinding, collision detection, save/load system, scoring system, etc. must be implemented manually.

---

# 3. Core Game Concept

The player is trapped inside a procedurally generated maze.

The objective is:

> Find the required key(s), avoid intelligent enemies, unlock the exit, and escape the maze before time runs out.

The maze should be different each time the player starts a new game.

The game should have multiple levels with increasing difficulty.

---

# 4. Game Perspective

Use a **2D top-down perspective**.

The maze consists of grid cells.

Example conceptual representation:

    ########################
    #P       #             #
    # #####  #  #########  #
    #     #     #          #
    ##### ##### # #########
    #       E             X#
    ########################

Where:

- `#` = Wall
- Empty space = Walkable area
- `P` = Player
- `E` = Enemy
- `X` = Exit

In the graphical version, use raylib shapes/textures rather than ASCII characters.

---

# 5. Main Gameplay Loop

The game should follow this general flow:

    Main Menu
        ↓
    Select Difficulty
        ↓
    Generate Maze
        ↓
    Place Player
        ↓
    Place Key(s)
        ↓
    Place Exit
        ↓
    Place Enemies
        ↓
    Start Level
        ↓
    Player Moves
        ↓
    Enemy AI Updates
        ↓
    Collision Detection
        ↓
    Check Objectives
        ↓
    Check Win/Lose
        ↓
    Next Level / Game Over
        ↓
    Save Score
        ↓
    Main Menu

---

# 6. Main Menu

Create a polished main menu.

Options:

1. New Game
2. Continue Game
3. High Scores
4. Instructions
5. Settings
6. Quit

Use raylib for menu rendering.

The menu should support keyboard navigation.

Highlight the currently selected option.

Controls:

- UP / DOWN → Navigate
- ENTER → Select
- ESC → Back

---

# 7. Difficulty System

Provide three difficulty levels:

## Easy

- Small maze
- 1 enemy
- Slower enemies
- Longer timer
- Larger player vision
- Less aggressive AI

## Normal

- Medium maze
- 2 enemies
- Medium enemy speed
- Normal timer
- Normal vision

## Hard

- Large maze
- 3–4 enemies
- Faster enemies
- Shorter timer
- Smaller player vision
- More intelligent enemy behavior

Difficulty should affect actual gameplay rather than just changing cosmetic values.

---

# 8. Procedural Maze Generation

Implement procedural maze generation manually.

Preferred algorithm:

**Recursive Backtracking / DFS**

Do NOT use an external maze-generation library.

The algorithm should:

1. Start from an initial cell.
2. Mark it visited.
3. Randomly choose an unvisited neighboring cell.
4. Remove the wall between the cells.
5. Move to the neighboring cell.
6. Continue recursively.
7. Backtrack when no unvisited neighbors remain.

Use a random seed based on current time.

Example:

    srand((unsigned int)time(NULL));

The maze dimensions should change based on difficulty.

For example:

Easy:

    21 × 21

Normal:

    31 × 31

Hard:

    41 × 41

Make the dimensions configurable through constants rather than hardcoding them throughout the project.

---

# 9. Maze Representation

Represent the maze using a suitable C data structure.

Prefer:

    typedef enum {
        CELL_WALL,
        CELL_PATH,
        CELL_START,
        CELL_EXIT,
        CELL_KEY
    } CellType;

Use a dynamically allocated 2D maze if appropriate.

Demonstrate:

- pointers
- malloc/calloc
- free
- structures
- enums

Create proper allocation and deallocation functions.

Example:

    Maze *createMaze(int width, int height);
    void destroyMaze(Maze *maze);

Do not create memory leaks.

---

# 10. Maze Validation

After generating a maze, verify that:

- Player can reach the key.
- Key can reach the exit.
- Enemies can move through the maze.
- The maze is not accidentally impossible to complete.

Use BFS or another suitable pathfinding method to validate connectivity.

If the maze is invalid, regenerate it.

---

# 11. Player System

Create a Player structure.

Example concept:

    typedef struct {
        Vector2 position;
        Vector2 gridPosition;
        int health;
        int maxHealth;
        int keysCollected;
        float speed;
        float sprintSpeed;
        float stamina;
    } Player;

You may modify the structure if a better design is appropriate.

Player controls:

- W / UP → Move up
- A / LEFT → Move left
- S / DOWN → Move down
- D / RIGHT → Move right
- SHIFT → Sprint
- ESC → Pause

Movement should feel smooth rather than teleporting between cells.

The player should not be able to move through walls.

Implement proper collision detection between the player and maze walls.

---

# 12. Camera System

Use a camera that follows the player.

Use raylib's:

    Camera2D

The camera should:

- Follow the player.
- Keep the player near the center of the screen.
- Prevent unnecessary movement outside the maze.
- Work correctly with different maze sizes.

---

# 13. Enemy System

Create an Enemy structure.

Example:

    typedef enum {
        ENEMY_PATROL,
        ENEMY_HUNTER,
        ENEMY_SMART
    } EnemyType;

Each enemy should have:

- Position
- Type
- Speed
- Detection range
- Current state
- Health if needed
- Target position
- Path information

---

# 14. Enemy Types

Implement at least three enemy types.

## Enemy 1 — Patrol Guard

The enemy follows a patrol route.

Behavior:

    Patrol
      ↓
    Detect Player
      ↓
    Chase
      ↓
    Lose Player
      ↓
    Return to Patrol

The patrol route can consist of predefined maze cells.

---

## Enemy 2 — BFS Hunter

This enemy uses **Breadth First Search** to find the shortest path to the player.

Implement BFS manually.

Do NOT use an external pathfinding library.

Use a queue.

Example structure:

    typedef struct {
        int x;
        int y;
    } GridNode;

Implement your own queue using an array or dynamically allocated memory.

The BFS system should:

1. Start at the enemy.
2. Explore neighboring cells.
3. Record parent relationships.
4. Stop when the player is found.
5. Reconstruct the path.
6. Make the enemy follow the path.

---

# 15. Enemy 3 — A* Smart Hunter

Implement A* pathfinding manually.

Use:

    f(n) = g(n) + h(n)

Where:

- `g(n)` = movement cost from enemy to current node
- `h(n)` = heuristic estimate to player
- `f(n)` = total estimated cost

Use Manhattan distance as the heuristic because movement is grid-based.

Implement:

- Open list
- Closed list
- Parent tracking
- Node costs
- Path reconstruction

Do NOT use external A* libraries.

The A* enemy should generally find efficient paths while exploring fewer cells than BFS in suitable situations.

---

# 16. AI State Machine

Implement an enemy state machine.

Possible states:

    PATROL
    INVESTIGATE
    CHASE
    SEARCH
    RETURN_TO_PATROL

Behavior:

### PATROL

Enemy follows its patrol route.

### INVESTIGATE

If the enemy notices suspicious activity or the player's last known location, it moves there.

### CHASE

If the player is detected, calculate a path and chase the player.

### SEARCH

If the player disappears, move toward the last known player position.

### RETURN_TO_PATROL

If the player is not found, return to the normal patrol route.

This should be implemented using C enums and state-management functions.

---

# 17. Line of Sight

Enemies should not automatically know the player's position.

Implement a basic line-of-sight system.

The enemy should detect the player when:

- Player is within detection range.
- There is no wall blocking the view.

Use a suitable ray/grid traversal method.

The player should be able to hide behind walls.

---

# 18. Enemy Difficulty

Enemy intelligence should increase with difficulty.

Easy:

- Lower detection range
- Lower speed
- Longer reaction time

Normal:

- Medium detection range
- Medium speed
- Faster reaction

Hard:

- Large detection range
- Higher speed
- More frequent path recalculation
- More aggressive pursuit

---

# 19. Fog of War

Implement a basic visibility system.

The player should not automatically see the entire maze.

Only cells within a certain radius around the player should be visible.

Example:

    visibilityRadius = 6;

Different difficulties may have different visibility ranges.

For example:

Easy:

    8 cells

Normal:

    6 cells

Hard:

    4 cells

Use a visibility array or another appropriate data structure.

---

# 20. Keys and Exit

The player must collect at least one key before escaping.

Gameplay:

    Find Key
       ↓
    Collect Key
       ↓
    Find Exit
       ↓
    Unlock Exit
       ↓
    Escape

The exit should remain locked until the required key is collected.

---

# 21. Power-Ups

Add several optional power-ups.

At minimum:

### Speed Boost

Temporarily increases movement speed.

### Invisibility

Enemies cannot detect the player for a limited period.

### Freeze

Temporarily freezes enemies.

### Health

Restores some health.

Power-ups should spawn at random valid positions.

---

# 22. Health and Enemy Collision

The player should have health.

Example:

    Health: 100

When an enemy catches the player:

    Health -= damage;

Add a short invincibility period after taking damage to prevent damage every frame.

If health reaches zero:

    GAME OVER

---

# 23. Timer

Every level should have a countdown timer.

Example:

    TIME: 02:00

When time reaches zero:

    GAME OVER

The timer should be based on actual elapsed time rather than decrementing once per frame.

Use raylib timing functions appropriately.

---

# 24. Score System

Calculate score based on:

- Level completed
- Remaining time
- Remaining health
- Keys collected
- Enemies avoided
- Power-ups collected

Example:

    Level Score =
        Base Score
        + Time Bonus
        + Health Bonus
        + Exploration Bonus

Display the current score during gameplay.

---

# 25. Multiple Levels

Create at least 5 levels.

Each level should increase difficulty.

Example:

Level 1:

    Small maze
    1 enemy

Level 2:

    Larger maze
    1–2 enemies

Level 3:

    Larger maze
    BFS enemy

Level 4:

    Large maze
    BFS + patrol enemies

Level 5:

    Large maze
    A* smart enemy
    Multiple objectives

Generate a new maze for every level.

---

# 26. Pause Menu

Press ESC to pause.

Display:

    PAUSED

    Resume
    Restart Level
    Settings
    Save Game
    Main Menu

---

# 27. Save / Load System

Implement save/load manually using C file handling.

Use:

    FILE *
    fopen()
    fread()
    fwrite()
    fclose()

Save:

- Current level
- Player position
- Player health
- Score
- Timer
- Keys collected
- Enemy positions
- Maze state
- Difficulty

Use a binary save file such as:

    save.dat

Do not rely on external databases.

---

# 28. High Score System

Create a leaderboard.

Example:

    HIGH SCORES

    1. HARSH       9500
    2. PLAYER2     8200
    3. PLAYER3     7600
    4. PLAYER4     6400
    5. PLAYER5     5900

Store scores in a file.

Create a structure such as:

    typedef struct {
        char name[32];
        int score;
    } ScoreEntry;

Sort the leaderboard using a sorting algorithm implemented manually.

Prefer Quick Sort.

---

# 29. AI Visualization / Debug Mode

This is an important feature for the college demonstration.

Create a debug mode that can be toggled with a key such as F1.

When enabled, display:

- Enemy detection range
- Enemy line of sight
- BFS explored cells
- A* explored cells
- Current enemy state
- Current target
- Current path

For example:

    Enemy State: CHASE
    Algorithm: A*
    Nodes Explored: 47
    Path Length: 18

Draw the calculated path using raylib lines or highlighted cells.

This will allow me to demonstrate the algorithms to my teacher.

---

# 30. AI Comparison Mode

Add an optional demonstration mode.

Allow the user to compare:

    Random
    Greedy
    DFS
    BFS
    A*

For the same maze, display:

    Algorithm     Path Length     Nodes Explored
    ----------------------------------------------
    Random        83              124
    Greedy        41               65
    DFS           39              101
    BFS           24               82
    A*            24               47

The exact values should be calculated dynamically.

Do not hardcode results.

This mode is primarily for demonstrating algorithmic differences during the project presentation.

---

# 31. Rendering

Use raylib to create a polished but simple visual style.

Use:

- Rectangles for walls
- Shapes or sprites for player/enemies
- Highlighted cells for keys and power-ups
- Text for UI
- Simple animations
- Camera movement

Do not make the visual system unnecessarily complicated.

The focus is on programming and algorithms.

Use a clean color scheme.

Example:

- Dark background
- Bright walls
- Distinct player color
- Different colors for enemy types
- Highlighted exit
- Clear UI

---

# 32. Player Visuals

The player should be visually distinguishable.

At minimum:

- Circle or simple sprite
- Direction indicator
- Optional walking animation

Do not require external art assets for the first version.

Use raylib primitives so the game works immediately after compilation.

---

# 33. Enemy Visuals

Use different visual representations for different enemy types.

For example:

Patrol:

    Blue

BFS Hunter:

    Red

A* Smart Hunter:

    Purple

Do not hardcode these colors throughout the code.

Create configurable rendering constants.

---

# 34. UI

During gameplay display:

    LEVEL: 3
    HEALTH: 75
    KEYS: 1/1
    SCORE: 2450
    TIME: 01:23

Also display temporary notifications:

    KEY FOUND!
    SPEED BOOST!
    ENEMY DETECTED!
    EXIT UNLOCKED!

---

# 35. Audio

Use raylib audio support.

Add simple sounds for:

- Player movement
- Key pickup
- Power-up pickup
- Enemy detection
- Damage
- Level completion
- Game over

Add background music if practical.

The game must still work if audio assets are missing, or clearly document where the assets must be placed.

Do not make the project dependent on copyrighted external assets.

Prefer placeholder/generated/simple assets.

---

# 36. Project Architecture

Do NOT put the entire project in `main.c`.

Use modular C files.

Recommended structure:

    MazeEscape/
    │
    ├── src/
    │   ├── main.c
    │   ├── game.c
    │   ├── player.c
    │   ├── enemy.c
    │   ├── maze.c
    │   ├── pathfinding.c
    │   ├── ai.c
    │   ├── collision.c
    │   ├── powerup.c
    │   ├── score.c
    │   ├── save.c
    │   └── ui.c
    │
    ├── include/
    │   ├── game.h
    │   ├── player.h
    │   ├── enemy.h
    │   ├── maze.h
    │   ├── pathfinding.h
    │   ├── ai.h
    │   ├── collision.h
    │   ├── powerup.h
    │   ├── score.h
    │   ├── save.h
    │   └── ui.h
    │
    ├── assets/
    │   ├── sounds/
    │   ├── music/
    │   └── fonts/
    │
    ├── saves/
    │   └── save.dat
    │
    ├── Makefile
    ├── README.md
    └── .gitignore

You may modify this architecture if there is a clearly better design, but maintain modularity.

---

# 37. Important C Programming Requirements

The project must demonstrate advanced C concepts.

Use appropriately:

### Structures

For:

- Player
- Enemy
- Maze
- Path nodes
- Score
- Game state

### Enumerations

For:

- Game states
- Enemy types
- Enemy states
- Cell types
- Difficulty

### Pointers

Use pointers appropriately for structures and dynamic data.

### Dynamic Memory

Use:

    malloc()
    calloc()
    realloc()
    free()

where appropriate.

Always check allocation failures.

### File Handling

Use:

    fopen()
    fread()
    fwrite()
    fprintf()
    fscanf()
    fclose()

where appropriate.

### Function Pointers

Use function pointers for selecting enemy AI where this improves the architecture.

For example conceptually:

    void (*updateAI)(Enemy *, Player *, Maze *);

Possible AI functions:

    randomAI()
    patrolAI()
    bfsAI()
    astarAI()

### Recursion

Use recursion for DFS-based maze generation.

### Data Structures

Implement manually where appropriate:

- Queue for BFS
- Stack/recursion for DFS
- Open/closed lists for A*
- Dynamic arrays
- Linked structures if useful

---

# 38. Memory Management

This is very important.

Every dynamically allocated resource must eventually be freed.

Check:

- malloc failure
- calloc failure
- realloc failure
- file opening failure
- texture loading failure
- sound loading failure

Provide cleanup functions such as:

    destroyMaze()
    destroyEnemy()
    unloadGameResources()

Avoid:

- memory leaks
- dangling pointers
- double free
- buffer overflow
- invalid array access

---

# 39. Error Handling

The program should fail gracefully.

For example:

If a save file doesn't exist:

    "No save file found."

If an asset cannot be loaded:

    "Warning: asset not found. Using fallback."

If memory allocation fails:

    "Memory allocation failed."

Do not crash silently.

---

# 40. Code Quality

Follow these rules:

- Use meaningful names.
- Use consistent indentation.
- Keep functions reasonably small.
- Avoid global variables wherever possible.
- Use `const` where appropriate.
- Use header guards.
- Add comments for complex algorithms.
- Separate game logic from rendering.
- Avoid duplicated code.
- Avoid magic numbers.
- Use constants/macros/enums where appropriate.

Do not over-comment obvious code.

Focus comments on explaining algorithms and design decisions.

---

# 41. Separation Between Logic and Rendering

This is very important.

Do not mix everything together.

For example:

Bad:

    if (enemy sees player)
        DrawRectangle(...)
        enemy.x += ...
        PlaySound(...)

Instead separate:

    updateEnemyAI();

    updateEnemy();

    drawEnemy();

    playEnemyDetectionSound();

The game should follow:

    INPUT
      ↓
    UPDATE
      ↓
    COLLISION
      ↓
    AI
      ↓
    RENDER

---

# 42. Game States

Create a game state system.

For example:

    typedef enum {
        STATE_MENU,
        STATE_DIFFICULTY,
        STATE_PLAYING,
        STATE_PAUSED,
        STATE_GAME_OVER,
        STATE_LEVEL_COMPLETE,
        STATE_HIGH_SCORES,
        STATE_INSTRUCTIONS,
        STATE_SETTINGS,
        STATE_AI_DEMO
    } GameState;

The main loop should switch behavior based on the current state.

---

# 43. Configuration

Create configurable constants for:

- Maze dimensions
- Player speed
- Enemy speed
- Detection radius
- Health
- Timer
- Number of enemies
- Number of levels
- Power-up duration

Do not scatter these values throughout the source code.

---

# 44. Performance

Keep the game running smoothly.

Target:

    60 FPS

Do not run expensive pathfinding calculations every frame unnecessarily.

For example, enemy paths can be recalculated:

- When the player moves significantly.
- When the current path becomes invalid.
- When the enemy loses its target.
- At a controlled AI update interval.

Avoid unnecessary memory allocation inside the main game loop.

---

# 45. README

Create a detailed README.md containing:

## Project Title

Maze Escape with Intelligent Enemies

## Description

Explain the game.

## Features

List all major features.

## Technologies

- C
- raylib
- GCC/MinGW

## Algorithms

Explain:

- DFS
- BFS
- A*
- Maze generation
- Sorting

## Data Structures

Explain:

- Arrays
- Structures
- Queue
- Stack
- Dynamic memory
- Graph representation

## Controls

Document all controls.

## Installation

Explain exactly how to install raylib and compile the project on Windows.

## Build Instructions

Provide exact commands.

## Project Architecture

Explain every `.c` and `.h` file.

## AI Explanation

Explain how each enemy works.

## Future Improvements

List possible future features.

---

# 46. Build Instructions

Provide a working Makefile.

Also provide Windows compilation instructions using MinGW/GCC.

For example, explain:

    gcc src/*.c -Iinclude -L<raylib-path> -lraylib -lopengl32 -lgdi32 -lwinmm -o MazeEscape.exe

Adjust the command based on the actual raylib installation.

Do not assume a specific raylib installation directory without explaining how the user should modify it.

---

# 47. Development Approach

Do NOT generate everything blindly in one huge code block.

Develop the project in logical milestones.

Milestone 1:

- raylib setup
- window
- game loop
- player movement

Milestone 2:

- maze representation
- maze rendering
- wall collision

Milestone 3:

- procedural maze generation

Milestone 4:

- player system
- key
- exit

Milestone 5:

- basic enemy

Milestone 6:

- BFS

Milestone 7:

- A*

Milestone 8:

- enemy state machine

Milestone 9:

- fog of war
- line of sight

Milestone 10:

- power-ups
- health
- timer
- scoring

Milestone 11:

- multiple levels
- difficulty

Milestone 12:

- save/load
- high scores

Milestone 13:

- UI
- audio
- polish

Milestone 14:

- AI demonstration mode
- performance measurements
- cleanup
- documentation

After every milestone, ensure the project compiles and runs before moving forward.

---

# 48. Important Rule About Code Generation

When generating code:

1. First explain the architecture.
2. Then provide the required files.
3. Clearly identify where every file belongs.
4. Give complete code rather than pseudo-code.
5. Make sure all functions have prototypes.
6. Make sure headers and source files match.
7. Make sure the project compiles.
8. Do not reference functions that haven't been implemented.
9. Do not leave TODO placeholders for core functionality.
10. Do not silently simplify the requested algorithms.
11. Explain complex sections after providing the code.
12. When modifying existing code, provide the complete modified file or a precise patch.

Do not generate a fake implementation of BFS/A* that simply moves the enemy directly toward the player.

The pathfinding algorithms must actually search the maze.

---

# 49. Viva-Friendly Design

The project should be designed so I can explain these topics during a viva:

### C

- Structures
- Pointers
- Dynamic memory allocation
- File handling
- Function pointers
- Recursion
- Header/source separation
- Modular programming
- Enums
- Memory management

### Data Structures

- Arrays
- Queue
- Stack
- Graphs
- Dynamic arrays

### Algorithms

- DFS
- BFS
- A*
- Maze generation
- Quick Sort
- Collision detection

### AI

- State machines
- Pathfinding
- Line of sight
- Enemy behavior
- Decision making

---

# 50. Final Expected Result

The final game should feel like a small but polished 2D game rather than a programming demo.

The player should be able to:

1. Launch the game.
2. Navigate the main menu.
3. Select difficulty.
4. Enter a randomly generated maze.
5. Explore the maze.
6. Find keys.
7. Avoid enemies.
8. See enemies patrol.
9. Get detected by enemies.
10. Watch enemies intelligently chase them.
11. Use power-ups.
12. Reach the exit.
13. Complete the level.
14. Progress through multiple levels.
15. Lose if health/time reaches zero.
16. Save and load progress.
17. View high scores.
18. Enter AI demonstration mode.
19. Compare BFS and A* behavior.

---

# 51. Priority Order

If implementation becomes too large, prioritize features in this order:

### Tier 1 — Mandatory

- raylib window
- player
- maze
- procedural generation
- collision
- exit
- enemy
- BFS
- A*
- multiple levels
- timer
- score

### Tier 2 — Important

- enemy state machine
- line of sight
- fog of war
- health
- keys
- save/load
- high scores

### Tier 3 — Polish

- power-ups
- animations
- sounds
- music
- advanced UI
- AI visualization
- AI comparison mode

Never sacrifice the correctness of the core algorithms just to add visual features.

---

# 52. First Task

Do NOT immediately generate the entire project.

Start by giving me:

1. A high-level architecture.
2. Complete folder structure.
3. Explanation of each module.
4. Required raylib installation/setup for Windows + MinGW.
5. Build configuration.
6. Development milestones.
7. Explanation of the data structures and algorithms we will use.
8. Then implement **Milestone 1 only**.

Milestone 1 should contain:

- raylib window
- game loop
- basic game state system
- player structure
- smooth WASD movement
- basic camera
- clean project structure
- Makefile
- comments explaining important C concepts

Make sure Milestone 1 compiles and runs before proceeding to Milestone 2.

Do not skip directly to the final game.