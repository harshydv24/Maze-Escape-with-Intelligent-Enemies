/*
 * ai.h — Enemy AI state machine, line-of-sight, behaviour dispatch
 */

#ifndef AI_H
#define AI_H

#include "config.h"

struct Enemy;
struct Player;
struct Maze;

void patrolAI(struct Enemy *e, struct Player *p, struct Maze *m, float dt);
void bfsAI   (struct Enemy *e, struct Player *p, struct Maze *m, float dt);
void astarAI (struct Enemy *e, struct Player *p, struct Maze *m, float dt);

void updateEnemyState(struct Enemy *e, struct Player *p, struct Maze *m);
bool checkLineOfSight(struct Maze *m, int x1, int y1, int x2, int y2);

typedef void (*AIUpdateFunc_t)(struct Enemy *, struct Player *, struct Maze *, float);
AIUpdateFunc_t getAIFunction(EnemyType type);

#endif /* AI_H */
