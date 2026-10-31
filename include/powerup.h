#ifndef POWERUP_H
#define POWERUP_H

#include "config.h"

typedef struct {
    float      x, y;
    int        gridX, gridY;
    PowerUpType type;
    bool       collected;
} PowerUp;

struct Maze;
PowerUp *createPowerUps(int count, struct Maze *maze);
void     destroyPowerUps(PowerUp *p);

#endif /* POWERUP_H */
