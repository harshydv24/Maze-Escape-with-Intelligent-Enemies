/*
 * player.h — Player structure and movement
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"

typedef struct Player {
    float x, y;           /* world-space centre */
    int   gridX, gridY;
    int   health, maxHealth;
    int   keysCollected;
    float speed, sprintSpeed, currentMoveSpeed;
    float moveTimer;
    float stamina, maxStamina;
    float invincibilityTimer;
    bool  isSprinting, isMoving;
    float dirX, dirY;     /* facing direction */
    /* Power-up timers */
    float speedBoostTimer;
    float invisibilityTimer;
    float freezeTimer;
    int   powerupsCollected;
} Player;

struct Maze;
void initPlayer(Player *p, int gridX, int gridY);
void updatePlayer(Player *p, struct Maze *maze, float dt);
void updatePlayerGridPos(Player *p);

#endif /* PLAYER_H */
