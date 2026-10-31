/*
 * collision.h — Player-wall, player-enemy, player-item collision
 */

#ifndef COLLISION_H
#define COLLISION_H

#include "config.h"

struct Maze;
struct Player;
struct Enemy;

bool checkWallCollision(struct Maze *maze, float x, float y, float halfSize);
bool checkPlayerEnemyCollision(struct Player *p, struct Enemy *e);
bool checkPlayerCellOverlap(struct Player *p, int cellX, int cellY);

#endif /* COLLISION_H */
