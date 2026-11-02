#include "visuals.h"
#include <string.h>

static Texture2D loadOptionalTexture(const char *path)
{
    Texture2D texture = {0};
    if (FileExists(path)) texture = LoadTexture(path);
    return texture;
}

void loadGameTextures(GameTextures *t)
{
    memset(t, 0, sizeof(*t));
    t->player = loadOptionalTexture("assets/images/player.png");
    t->enemyPatrol = loadOptionalTexture("assets/images/enemy_patrol.png");
    t->enemyBfs = loadOptionalTexture("assets/images/enemy_bfs.png");
    t->enemyAstar = loadOptionalTexture("assets/images/enemy_astar.png");
    t->powerupSpeed = loadOptionalTexture("assets/images/powerup_speed.png");
    t->powerupInvisibility = loadOptionalTexture("assets/images/powerup_invisibility.png");
    t->powerupFreeze = loadOptionalTexture("assets/images/powerup_freeze.png");
    t->powerupHealth = loadOptionalTexture("assets/images/powerup_health.png");
    t->key = loadOptionalTexture("assets/images/key.png");
}

void unloadGameTextures(GameTextures *t)
{
    if (t->player.id) UnloadTexture(t->player);
    if (t->enemyPatrol.id) UnloadTexture(t->enemyPatrol);
    if (t->enemyBfs.id) UnloadTexture(t->enemyBfs);
    if (t->enemyAstar.id) UnloadTexture(t->enemyAstar);
    if (t->powerupSpeed.id) UnloadTexture(t->powerupSpeed);
    if (t->powerupInvisibility.id) UnloadTexture(t->powerupInvisibility);
    if (t->powerupFreeze.id) UnloadTexture(t->powerupFreeze);
    if (t->powerupHealth.id) UnloadTexture(t->powerupHealth);
    if (t->key.id) UnloadTexture(t->key);
    memset(t, 0, sizeof(*t));
}
