/*
 * enemy.c — Enemy creation, destruction, path movement
 *
 * Demonstrates: dynamic allocation, function pointers, structure init.
 */

#include "enemy.h"
#include "maze.h"
#include "ai.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

Enemy *createEnemies(int count, Maze *maze, const EnemyType *types, Difficulty diff)
{
    Enemy *enemies = (Enemy *)calloc(count, sizeof(Enemy));
    if (!enemies) { fprintf(stderr, "Error: alloc enemies\n"); return NULL; }

    float spd, det, react;
    switch (diff) {
        case DIFF_EASY: spd=ENEMY_SPEED_EASY; det=ENEMY_DETECT_EASY; react=ENEMY_REACT_EASY; break;
        case DIFF_HARD: spd=ENEMY_SPEED_HARD; det=ENEMY_DETECT_HARD; react=ENEMY_REACT_HARD; break;
        default:        spd=ENEMY_SPEED_NORMAL;det=ENEMY_DETECT_NORMAL;react=ENEMY_REACT_NORMAL;break;
    }

    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        e->type = types[i];
        e->state = AI_PATROL;
        e->speed = e->baseSpeed = spd;
        e->detectionRange = det;
        e->reactionTime = react;
        e->updateAI = getAIFunction(types[i]);

        GridNode pos; int att = 0;
        do { pos = randomPathCell(maze); att++; }
        while (abs(pos.x - maze->start.x) + abs(pos.y - maze->start.y) < 10 && att < 10000);

        e->gridX = pos.x; e->gridY = pos.y;
        e->x = pos.x * CELL_SIZE + CELL_SIZE / 2.0f;
        e->y = pos.y * CELL_SIZE + CELL_SIZE / 2.0f;
        initPatrolRoute(e, maze);
    }
    return enemies;
}

void destroyEnemies(Enemy *enemies, int count)
{
    if (!enemies) return;
    for (int i = 0; i < count; i++) free(enemies[i].path);
    free(enemies);
}

void initPatrolRoute(Enemy *e, Maze *maze)
{
    e->patrolPointCount = 0;
    e->currentPatrolIndex = 0;
    e->patrolPoints[e->patrolPointCount++] = (GridNode){ e->gridX, e->gridY };

    for (int i = 1; i < 4; i++) {
        GridNode pt; int att = 0;
        do { pt = randomPathCell(maze); att++; }
        while (abs(pt.x-e->gridX)+abs(pt.y-e->gridY) > 20 && att < 500);
        if (att < 500) e->patrolPoints[e->patrolPointCount++] = pt;
    }
}

void moveEnemyAlongPath(Enemy *e, float dt)
{
    if (!e->path || e->pathIndex >= e->pathLength) return;
    GridNode tgt = e->path[e->pathIndex];
    float tx = tgt.x * CELL_SIZE + CELL_SIZE / 2.0f;
    float ty = tgt.y * CELL_SIZE + CELL_SIZE / 2.0f;
    float dx = tx - e->x, dy = ty - e->y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 2.0f) {
        e->x = tx; e->y = ty; e->gridX = tgt.x; e->gridY = tgt.y;
        e->pathIndex++;
    } else {
        float mv = e->speed * dt;
        if (mv > dist) mv = dist;
        e->x += (dx/dist)*mv;  e->y += (dy/dist)*mv;
        e->gridX = (int)(e->x / CELL_SIZE);
        e->gridY = (int)(e->y / CELL_SIZE);
    }
}

void setEnemyPath(Enemy *e, PathResult r)
{
    free(e->path); e->path = NULL;
    if (r.found && r.length > 0) {
        e->path = r.nodes; e->pathLength = r.length;
        e->pathIndex = 1; e->nodesExplored = r.nodesExplored;
    } else {
        e->pathLength = e->pathIndex = 0;
        free(r.nodes);
    }
}
