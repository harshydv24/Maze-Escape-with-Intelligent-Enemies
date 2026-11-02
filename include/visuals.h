#ifndef VISUALS_H
#define VISUALS_H

#include "config.h"

/* Each optional PNG has its own Texture2D so the renderer can swap art by type. */
typedef struct {
    Texture2D player;
    Texture2D enemyPatrol, enemyBfs, enemyAstar;
    Texture2D powerupSpeed, powerupInvisibility, powerupFreeze, powerupHealth;
    Texture2D key;
} GameTextures;

void loadGameTextures(GameTextures *textures);
void unloadGameTextures(GameTextures *textures);

#endif /* VISUALS_H */
