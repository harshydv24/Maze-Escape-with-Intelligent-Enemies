/*
 * collision.c — Grid-based collision detection
 */

#include "collision.h"
#include "maze.h"
#include "player.h"
#include "enemy.h"
#include <math.h>

bool checkWallCollision(Maze *maze, float x, float y, float hs)
{
    int minX = (int)((x - hs) / CELL_SIZE);
    int maxX = (int)((x + hs - 0.01f) / CELL_SIZE);
    int minY = (int)((y - hs) / CELL_SIZE);
    int maxY = (int)((y + hs - 0.01f) / CELL_SIZE);
    for (int gy = minY; gy <= maxY; gy++)
        for (int gx = minX; gx <= maxX; gx++) {
            if (gx<0||gx>=maze->width||gy<0||gy>=maze->height) return true;
            if (maze->grid[gy][gx] == CELL_WALL) return true;
        }
    return false;
}

bool checkPlayerEnemyCollision(Player *p, Enemy *e)
{
    float dx=p->x-e->x, dy=p->y-e->y;
    return sqrtf(dx*dx+dy*dy) < (PLAYER_COLLISION_SIZE+ENEMY_SIZE)/2.0f;
}

bool checkPlayerCellOverlap(Player *p, int cx, int cy)
{
    return p->gridX==cx && p->gridY==cy;
}
