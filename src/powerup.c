/*
 * powerup.c — Power-up creation and management
 */

#include "powerup.h"
#include "maze.h"
#include <stdlib.h>
#include <stdio.h>

PowerUp *createPowerUps(int count, Maze *maze)
{
    if (count <= 0) return NULL;
    PowerUp *pu = (PowerUp *)calloc(count, sizeof(PowerUp));
    if (!pu) { fprintf(stderr, "Error: alloc powerups\n"); return NULL; }

    for (int i = 0; i < count; i++) {
        GridNode pos; int att = 0;
        do { pos = randomPathCell(maze); att++; }
        while (maze->grid[pos.y][pos.x] != CELL_PATH && att < 1000);

        pu[i].gridX = pos.x; pu[i].gridY = pos.y;
        pu[i].x = pos.x * CELL_SIZE + CELL_SIZE / 2.0f;
        pu[i].y = pos.y * CELL_SIZE + CELL_SIZE / 2.0f;
        pu[i].type = (PowerUpType)(rand() % POWERUP_COUNT);
        pu[i].collected = false;
    }
    return pu;
}

void destroyPowerUps(PowerUp *p) { free(p); }
