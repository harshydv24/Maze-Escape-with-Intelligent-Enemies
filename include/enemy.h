/*
 * enemy.h — Enemy structure, creation, movement
 *
 * Uses a function pointer (AIUpdateFunc) so each enemy type
 * can have its own AI behaviour — demonstrates function pointers.
 */

#ifndef ENEMY_H
#define ENEMY_H

#include "config.h"
#include "pathfinding.h"

struct Player;
struct Maze;

typedef struct Enemy Enemy;
typedef void (*AIUpdateFunc)(Enemy *, struct Player *, struct Maze *, float);

struct Enemy {
    float        x, y;
    int          gridX, gridY;
    EnemyType    type;
    EnemyAIState state;
    float        speed, baseSpeed;
    float        detectionRange;
    float        reactionTime, reactionTimer;
    /* Path */
    GridNode    *path;
    int          pathLength, pathIndex;
    float        pathTimer;
    /* Patrol */
    GridNode     patrolPoints[MAX_PATROL_POINTS];
    int          patrolPointCount, currentPatrolIndex;
    /* AI */
    AIUpdateFunc updateAI;
    float        lastKnownX, lastKnownY;
    bool         playerDetected;
    float        searchTimer;
    /* Debug */
    int          nodesExplored;
};

Enemy *createEnemies(int count, struct Maze *maze, const EnemyType *types, Difficulty diff);
void   destroyEnemies(Enemy *enemies, int count);
void   moveEnemyAlongPath(Enemy *enemy, float dt);
void   setEnemyPath(Enemy *enemy, PathResult result);
void   initPatrolRoute(Enemy *enemy, struct Maze *maze);

#endif /* ENEMY_H */
