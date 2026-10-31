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
    p->moveTimer = 0;
    p->stamina = PLAYER_MAX_STAMINA;  p->maxStamina = PLAYER_MAX_STAMINA;
    p->invincibilityTimer = 0;  p->isSprinting = false; p->isMoving = false;
    p->dirX = 0; p->dirY = 1;
    p->speedBoostTimer = p->invisibilityTimer = p->freezeTimer = 0;
    p->powerupsCollected = 0;
}

void updatePlayerGridPos(Player *p)
{
    p->gridX = (int)(p->x / CELL_SIZE);
    p->gridY = (int)(p->y / CELL_SIZE);
}

static bool isJunctionAhead(Maze *maze, int gx, int gy, float dirX, float dirY)
{
    int choices = 0;
    const int stepX[] = { 0, 0, -1, 1 };
    const int stepY[] = { -1, 1, 0, 0 };

    for (int i = 0; i < 4; i++) {
        /* Ignore the direction back along the path the player came from. */
        if (stepX[i] == -(int)dirX && stepY[i] == -(int)dirY) continue;
        if (isCellWalkable(maze, gx + stepX[i], gy + stepY[i])) choices++;
    }
    return choices >= 2;
}

void updatePlayer(Player *p, Maze *maze, float dt)
{
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        p->dirX = 0; p->dirY = -1; p->isMoving = true;
    } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        p->dirX = 0; p->dirY = 1; p->isMoving = true;
    } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        p->dirX = -1; p->dirY = 0; p->isMoving = true;
    } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        p->dirX = 1; p->dirY = 0; p->isMoving = true;
    }

    p->isSprinting = false;
    float moveSpeed = p->speed;
    if (p->speedBoostTimer > 0) moveSpeed *= 1.5f;
    if (IsKeyDown(KEY_LEFT_SHIFT) && p->stamina > 0 && p->isMoving) {
        p->isSprinting = true;
        moveSpeed = p->sprintSpeed;
        if (p->speedBoostTimer > 0) moveSpeed *= 1.3f;
        p->stamina -= PLAYER_STAMINA_DRAIN * dt;
        if (p->stamina < 0) p->stamina = 0;
    } else {
        p->stamina += PLAYER_STAMINA_REGEN * dt;
        if (p->stamina > p->maxStamina) p->stamina = p->maxStamina;
    }

    if (p->isMoving && maze) {
        /* Keep traveling in the last pressed direction until a wall blocks us. */
        float distance = moveSpeed * dt;
        float halfSize = PLAYER_COLLISION_SIZE / 2.0f;
        int oldGridX = p->gridX, oldGridY = p->gridY;
        float nextX = p->x + p->dirX * distance;
        float nextY = p->y + p->dirY * distance;
        if (checkWallCollision(maze, nextX, nextY, halfSize)) {
            p->isMoving = false;
            /* Land at the center of the last open cell for a crisp stop. */
            float halfCell = CELL_SIZE / 2.0f;
            if (p->dirX != 0.0f)
                p->x = roundf((p->x - halfCell) / CELL_SIZE) * CELL_SIZE + halfCell;
            if (p->dirY != 0.0f)
                p->y = roundf((p->y - halfCell) / CELL_SIZE) * CELL_SIZE + halfCell;
        } else {
            p->x = nextX;
            p->y = nextY;
            updatePlayerGridPos(p);
            if ((p->gridX != oldGridX || p->gridY != oldGridY) &&
                isJunctionAhead(maze, p->gridX, p->gridY, p->dirX, p->dirY)) {
                /* Center on the branch so the player can choose a direction. */
                p->x = p->gridX * CELL_SIZE + CELL_SIZE / 2.0f;
                p->y = p->gridY * CELL_SIZE + CELL_SIZE / 2.0f;
                p->isMoving = false;
            }
        }
        updatePlayerGridPos(p);
    }

    p->stamina += PLAYER_STAMINA_REGEN * dt;
    if (p->stamina > p->maxStamina) p->stamina = p->maxStamina;

    if (p->speedBoostTimer   > 0) p->speedBoostTimer   -= dt;
    if (p->invisibilityTimer > 0) p->invisibilityTimer -= dt;
    if (p->freezeTimer       > 0) p->freezeTimer       -= dt;
    if (p->invincibilityTimer> 0) p->invincibilityTimer-= dt;
}
