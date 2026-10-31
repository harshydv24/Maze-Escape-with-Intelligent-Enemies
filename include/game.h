/*
 * game.h — Master Game structure and lifecycle functions
 */

#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "maze.h"
#include "pathfinding.h"
#include "player.h"
#include "enemy.h"
#include "powerup.h"
#include "score.h"
#include "audio.h"

typedef struct Game {
    GameState  state, previousState;
    Difficulty difficulty;
    Maze      *maze;
    Player     player;
    Enemy     *enemies;
    int        enemyCount;
    PowerUp   *powerups;
    int        powerupCount;
    int        currentLevel;
    float      timer;
    int        score, totalScore;
    int        keysRequired;
    bool      *visible;
    bool      *explored;
    int        visibilityRadius;
    Camera2D   camera;
    bool       debugMode, fogEnabled, showFPS;
    int        menuSelection, maxMenuItems;
    Notification notifications[MAX_NOTIFICATIONS];
    int        notificationCount;
    ScoreEntry highScores[MAX_HIGH_SCORES];
    int        highScoreCount;
    char       playerName[MAX_NAME_LENGTH];
    int        nameLength;
    GameAudio  audio;
    /* AI demo */
    Maze      *demoMaze;
    GridNode   demoStart, demoEnd;
    AIComparisonResult aiResults[5];
    bool       aiResultsComputed;
    int        aiSelectedAlgo;
} Game;

void initGame(Game *g);
void cleanupGame(Game *g);
void updateGame(Game *g, float dt);
void drawGameFrame(Game *g);

void updateMenu(Game *g);
void updateDifficulty(Game *g);
void updatePlaying(Game *g, float dt);
void updatePaused(Game *g);
void updateGameOver(Game *g);
void updateLevelComplete(Game *g);
void updateHighScores(Game *g);
void updateInstructions(Game *g);
void updateSettings(Game *g);
void updateAIDemo(Game *g);
void updateNameEntry(Game *g);

void startNewGame(Game *g, Difficulty d);
void startLevel(Game *g);
void nextLevel(Game *g);
void cleanupLevel(Game *g);
void computeVisibility(Game *g);
void addNotification(Game *g, const char *text, Color color);
void updateNotifications(Game *g, float dt);

#endif /* GAME_H */
