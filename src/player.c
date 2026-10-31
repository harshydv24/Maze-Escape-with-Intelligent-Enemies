/*
 * player.c — Player movement, collision response, sprint/stamina
 *
 * Demonstrates: structures, smooth delta-time movement,
 * axis-separated collision sliding.
 */

#include "player.h"
#include "maze.h"
#include "collision.h"
#include <math.h>

void initPlayer(Player *p, int gx, int gy)
{
    p->gridX = gx;  p->gridY = gy;
    p->x = gx * CELL_SIZE + CELL_SIZE / 2.0f;
    p->y = gy * CELL_SIZE + CELL_SIZE / 2.0f;
    p->health = PLAYER_MAX_HEALTH;  p->maxHealth = PLAYER_MAX_HEALTH;
    p->keysCollected = 0;
    p->speed = PLAYER_SPEED;  p->sprintSpeed = PLAYER_SPRINT_SPEED;
    p->stamina = PLAYER_MAX_STAMINA;  p->maxStamina = PLAYER_MAX_STAMINA;
    p->invincibilityTimer = 0;  p->isSprinting = false;
    p->dirX = 0; p->dirY = 1;
    p->speedBoostTimer = p->invisibilityTimer = p->freezeTimer = 0;
    p->powerupsCollected = 0;
}

void updatePlayerGridPos(Player *p)
{
    p->gridX = (int)(p->x / CELL_SIZE);
    p->gridY = (int)(p->y / CELL_SIZE);
}

void updatePlayer(Player *p, Maze *maze, float dt)
{
    float dx = 0, dy = 0;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dy += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dx -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1;

    float len = sqrtf(dx*dx + dy*dy);
    if (len > 0) { dx /= len; dy /= len; p->dirX = dx; p->dirY = dy; }

    p->isSprinting = false;
    float spd = p->speed;
    if (p->speedBoostTimer > 0) spd *= 1.5f;

    if (IsKeyDown(KEY_LEFT_SHIFT) && p->stamina > 0 && len > 0) {
        p->isSprinting = true;
        spd = p->sprintSpeed;
        if (p->speedBoostTimer > 0) spd *= 1.3f;
        p->stamina -= PLAYER_STAMINA_DRAIN * dt;
        if (p->stamina < 0) p->stamina = 0;
    } else {
        p->stamina += PLAYER_STAMINA_REGEN * dt;
        if (p->stamina > p->maxStamina) p->stamina = p->maxStamina;
    }

    float hs = PLAYER_SIZE / 2.0f;
    float nx = p->x + dx * spd * dt;
    if (!checkWallCollision(maze, nx, p->y, hs)) p->x = nx;
    float ny = p->y + dy * spd * dt;
    if (!checkWallCollision(maze, p->x, ny, hs)) p->y = ny;

    updatePlayerGridPos(p);

    if (p->speedBoostTimer   > 0) p->speedBoostTimer   -= dt;
    if (p->invisibilityTimer > 0) p->invisibilityTimer -= dt;
    if (p->freezeTimer       > 0) p->freezeTimer       -= dt;
    if (p->invincibilityTimer> 0) p->invincibilityTimer-= dt;
}
