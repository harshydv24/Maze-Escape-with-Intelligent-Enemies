/*
 * ai.c — Enemy AI state machine, line-of-sight, behaviour functions
 *
 * Demonstrates: finite state machine, Bresenham LOS,
 * function pointers for polymorphic AI dispatch.
 */

#include "ai.h"
#include "enemy.h"
#include "player.h"
#include "maze.h"
#include "pathfinding.h"
#include <stdlib.h>
#include <math.h>

/* ---- Line of Sight (Bresenham grid traversal) ---- */

bool checkLineOfSight(Maze *m, int x1, int y1, int x2, int y2)
{
    int dx = abs(x2-x1), dy = abs(y2-y1);
    int sx = (x1<x2)?1:-1, sy = (y1<y2)?1:-1;
    int err = dx - dy, cx = x1, cy = y1;
    while (!(cx==x2 && cy==y2)) {
        if (cx<0||cx>=m->width||cy<0||cy>=m->height) return false;
        if (m->grid[cy][cx] == CELL_WALL) return false;
        int e2 = 2*err;
        if (e2 > -dy) { err -= dy; cx += sx; }
        if (e2 <  dx) { err += dx; cy += sy; }
    }
    return true;
}

static float gridDist(int x1,int y1,int x2,int y2)
{
    float dx=(float)(x1-x2), dy=(float)(y1-y2);
    return sqrtf(dx*dx+dy*dy);
}

/* ---- State machine ---- */

void updateEnemyState(Enemy *e, Player *p, Maze *m)
{
    float dist = gridDist(e->gridX,e->gridY,p->gridX,p->gridY);
    bool inRange = dist <= e->detectionRange;
    bool invis = p->invisibilityTimer > 0;
    bool hasLOS = false;
    if (inRange && !invis)
        hasLOS = checkLineOfSight(m, e->gridX, e->gridY, p->gridX, p->gridY);
    bool detected = inRange && hasLOS && !invis;

    switch (e->state) {
    case AI_PATROL:
        if (detected) {
            e->state=AI_CHASE; e->lastKnownX=p->x; e->lastKnownY=p->y;
            e->playerDetected=true; e->searchTimer=0;
        } break;
    case AI_INVESTIGATE:
        if (detected) { e->state=AI_CHASE; e->lastKnownX=p->x; e->lastKnownY=p->y; e->playerDetected=true; }
        else if (!e->path || e->pathIndex>=e->pathLength) e->state=AI_RETURN_TO_PATROL;
        break;
    case AI_CHASE:
        if (detected) { e->lastKnownX=p->x; e->lastKnownY=p->y; }
        else { e->state=AI_SEARCH; e->searchTimer=5.0f; e->playerDetected=false; }
        break;
    case AI_SEARCH:
        if (detected) { e->state=AI_CHASE; e->lastKnownX=p->x; e->lastKnownY=p->y; e->playerDetected=true; }
        else { e->searchTimer -= GetFrameTime(); if (e->searchTimer<=0) e->state=AI_RETURN_TO_PATROL; }
        break;
    case AI_RETURN_TO_PATROL:
        if (detected) { e->state=AI_CHASE; e->lastKnownX=p->x; e->lastKnownY=p->y; e->playerDetected=true; }
        else if (!e->path || e->pathIndex>=e->pathLength) e->state=AI_PATROL;
        break;
    }
}

/* ---- Helper: common AI behaviour per state ---- */

static void doAI(Enemy *e, Player *p, Maze *m, float dt,
                 PathResult (*findPath)(Maze*,GridNode,GridNode))
{
    updateEnemyState(e, p, m);
    if (p->freezeTimer > 0) return;
    e->pathTimer += dt;

    GridNode start = { e->gridX, e->gridY };

    switch (e->state) {
    case AI_PATROL:
        if (!e->path || e->pathIndex>=e->pathLength) {
            if (e->patrolPointCount > 0) {
                e->currentPatrolIndex = (e->currentPatrolIndex+1) % e->patrolPointCount;
                setEnemyPath(e, findPath(m, start, e->patrolPoints[e->currentPatrolIndex]));
            }
        }
        moveEnemyAlongPath(e, dt);
        break;
    case AI_CHASE:
        if (e->pathTimer >= AI_UPDATE_INTERVAL) {
            e->pathTimer = 0;
            GridNode end = { p->gridX, p->gridY };
            setEnemyPath(e, findPath(m, start, end));
        }
        moveEnemyAlongPath(e, dt);
        break;
    case AI_SEARCH:
        if (!e->path || e->pathIndex>=e->pathLength) {
            int lx=(int)(e->lastKnownX/CELL_SIZE), ly=(int)(e->lastKnownY/CELL_SIZE);
            setEnemyPath(e, findPath(m, start, (GridNode){lx,ly}));
        }
        moveEnemyAlongPath(e, dt);
        break;
    case AI_INVESTIGATE:
    case AI_RETURN_TO_PATROL:
        if (!e->path || e->pathIndex>=e->pathLength) {
            if (e->patrolPointCount > 0)
                setEnemyPath(e, findPath(m, start, e->patrolPoints[e->currentPatrolIndex]));
        }
        moveEnemyAlongPath(e, dt);
        break;
    }
}

void patrolAI(Enemy *e, Player *p, Maze *m, float dt) { doAI(e,p,m,dt,findPathBFS); }
void bfsAI   (Enemy *e, Player *p, Maze *m, float dt) { doAI(e,p,m,dt,findPathBFS); }
void astarAI (Enemy *e, Player *p, Maze *m, float dt) { doAI(e,p,m,dt,findPathAStar); }

AIUpdateFunc_t getAIFunction(EnemyType t)
{
    switch (t) {
        case ENEMY_PATROL: return patrolAI;
        case ENEMY_HUNTER: return bfsAI;
        case ENEMY_SMART:  return astarAI;
        default:           return patrolAI;
    }
}
