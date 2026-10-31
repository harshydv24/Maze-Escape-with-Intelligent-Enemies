/*
 * config.h — All configurable constants, shared types, and enumerations
 *
 * Centralises every tuneable value so nothing is scattered across source files.
 * Demonstrates: enumerations, typedef, #define constants, header guards.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"
#include <stdbool.h>

/* ---- Screen ---- */
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define TARGET_FPS      60

/* ---- Cell / Grid ---- */
#define CELL_SIZE       32

/* ---- Maze dimensions per difficulty (must be odd) ---- */
#define MAZE_SIZE_EASY      21
#define MAZE_SIZE_NORMAL    31
#define MAZE_SIZE_HARD      41

/* ---- Player ---- */
#define PLAYER_SIZE             (CELL_SIZE * 0.6f)
#define PLAYER_SPEED            150.0f
#define PLAYER_SPRINT_SPEED     250.0f
#define PLAYER_MAX_HEALTH       100
#define PLAYER_MAX_STAMINA      100.0f
#define PLAYER_STAMINA_DRAIN    30.0f
#define PLAYER_STAMINA_REGEN    15.0f
#define PLAYER_INVINCIBILITY    1.5f

/* ---- Enemy ---- */
#define ENEMY_SIZE              (CELL_SIZE * 0.6f)
#define ENEMY_DAMAGE            20
#define MAX_ENEMIES             4
#define MAX_PATROL_POINTS       8
#define AI_UPDATE_INTERVAL      0.3f

#define ENEMY_SPEED_EASY        60.0f
#define ENEMY_SPEED_NORMAL      90.0f
#define ENEMY_SPEED_HARD        120.0f

#define ENEMY_DETECT_EASY       4.0f
#define ENEMY_DETECT_NORMAL     6.0f
#define ENEMY_DETECT_HARD       8.0f

#define ENEMY_REACT_EASY        1.0f
#define ENEMY_REACT_NORMAL      0.5f
#define ENEMY_REACT_HARD        0.2f

/* ---- Visibility (fog-of-war radius in cells) ---- */
#define VISIBILITY_EASY         8
#define VISIBILITY_NORMAL       6
#define VISIBILITY_HARD         4

/* ---- Timer (seconds) ---- */
#define TIMER_EASY              180.0f
#define TIMER_NORMAL            120.0f
#define TIMER_HARD              90.0f

/* ---- Scoring ---- */
#define SCORE_BASE_LEVEL        1000
#define SCORE_TIME_BONUS        5
#define SCORE_HEALTH_BONUS      10
#define SCORE_KEY_BONUS         500
#define SCORE_POWERUP_BONUS     100

/* ---- Power-ups ---- */
#define MAX_POWERUPS            5
#define POWERUP_SPEED_DURATION  5.0f
#define POWERUP_INVIS_DURATION  4.0f
#define POWERUP_FREEZE_DURATION 3.0f
#define POWERUP_HEALTH_AMOUNT   30

/* ---- Levels ---- */
#define MAX_LEVELS              5

/* ---- High Scores ---- */
#define MAX_HIGH_SCORES         10
#define MAX_NAME_LENGTH         32

/* ---- Notifications ---- */
#define MAX_NOTIFICATIONS       5
#define NOTIFICATION_DURATION   2.0f

/* ---- Save Files ---- */
#define SAVE_FILE_PATH          "saves/save.dat"
#define HIGHSCORE_FILE_PATH     "saves/highscores.dat"
#define SAVE_MAGIC              0x4D415A45
#define SAVE_VERSION            1

/* ---- Colours (Dracula-inspired palette) ---- */
#define COLOR_BG                (Color){30, 31, 38, 255}
#define COLOR_WALL              (Color){68, 71, 90, 255}
#define COLOR_PATH              (Color){40, 42, 54, 255}
#define COLOR_PLAYER            (Color){80, 250, 123, 255}
#define COLOR_KEY               (Color){241, 250, 140, 255}
#define COLOR_EXIT_LOCKED       (Color){139, 133, 137, 255}
#define COLOR_EXIT_UNLOCKED     (Color){80, 250, 123, 255}
#define COLOR_ENEMY_PATROL      (Color){98, 114, 164, 255}
#define COLOR_ENEMY_BFS         (Color){255, 85, 85, 255}
#define COLOR_ENEMY_ASTAR       (Color){189, 147, 249, 255}
#define COLOR_FOG               (Color){0, 0, 0, 220}
#define COLOR_EXPLORED          (Color){0, 0, 0, 160}
#define COLOR_POWERUP_SPEED     (Color){255, 184, 108, 255}
#define COLOR_POWERUP_INVIS     (Color){139, 233, 253, 255}
#define COLOR_POWERUP_FREEZE    (Color){98, 114, 164, 255}
#define COLOR_POWERUP_HEALTH    (Color){255, 121, 198, 255}
#define COLOR_HUD_BG            (Color){30, 30, 40, 220}
#define COLOR_MENU_BG           (Color){22, 23, 29, 255}
#define COLOR_MENU_HIGHLIGHT    (Color){80, 250, 123, 255}
#define COLOR_TITLE             (Color){189, 147, 249, 255}
#define COLOR_DEBUG_PATH        (Color){255, 184, 108, 120}
#define COLOR_DEBUG_LOS         (Color){139, 233, 253, 80}

/* ---- Shared simple types ---- */

typedef struct {
    int x;
    int y;
} GridNode;

typedef struct {
    char  text[64];
    float timer;
    Color color;
} Notification;

/* ---- Enumerations ---- */

typedef enum {
    CELL_WALL,
    CELL_PATH,
    CELL_START,
    CELL_EXIT,
    CELL_KEY
} CellType;

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
    STATE_AI_DEMO,
    STATE_NAME_ENTRY
} GameState;

typedef enum {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD
} Difficulty;

typedef enum {
    ENEMY_PATROL,
    ENEMY_HUNTER,
    ENEMY_SMART
} EnemyType;

typedef enum {
    AI_PATROL,
    AI_INVESTIGATE,
    AI_CHASE,
    AI_SEARCH,
    AI_RETURN_TO_PATROL
} EnemyAIState;

typedef enum {
    POWERUP_SPEED,
    POWERUP_INVISIBILITY,
    POWERUP_FREEZE,
    POWERUP_HEALTH,
    POWERUP_COUNT
} PowerUpType;

#endif /* CONFIG_H */
