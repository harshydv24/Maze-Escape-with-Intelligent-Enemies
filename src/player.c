/*
 * player.c — Player movement, collision response, sprint/stamina
 *
 * Demonstrates: structures, smooth delta-time movement,
 * axis-separated collision sliding.
 */

#include "player.h"
#include "maze.h"
#include "collision.h"

void initPlayer(Player *p, int gx, int gy)
{
    p->gridX = gx;  p->gridY = gy;
    p->x = gx * CELL_SIZE + CELL_SIZE / 2.0f;
    p->y = gy * CELL_SIZE + CELL_SIZE / 2.0f;
    p->health = PLAYER_MAX_HEALTH;  p->maxHealth = PLAYER_MAX_HEALTH;
    p->keysCollected = 0;
    p->speed = PLAYER_SPEED;  p->sprintSpeed = PLAYER_SPRINT_SPEED;
    p->currentMoveSpeed = 0;
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
    float requestedDirX=0, requestedDirY=0;
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        requestedDirY = -1;
    } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        requestedDirY = 1;
    } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        requestedDirX = -1;
    } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        requestedDirX = 1;
    }

    if (requestedDirX != 0 || requestedDirY != 0) {
        if (!p->isMoving) {
            p->dirX = requestedDirX; p->dirY = requestedDirY;
            p->isMoving = true;
        } else {
            /* Turn immediately only if the player has clear space in that direction. */
            float probeDistance=p->currentMoveSpeed*dt;
            if (probeDistance<1.0f) probeDistance=1.0f;
            float halfSize=PLAYER_COLLISION_SIZE/2.0f;
            float probeX=p->x+requestedDirX*probeDistance;
            float probeY=p->y+requestedDirY*probeDistance;
            if (isCellWalkable(maze,p->gridX+(int)requestedDirX,
                                    p->gridY+(int)requestedDirY) &&
                !checkWallCollision(maze,probeX,probeY,halfSize)) {
                p->dirX=requestedDirX;
                p->dirY=requestedDirY;
            }
        }
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

    /* Ease into movement instead of instantly applying full walking speed. */
    float targetSpeed = p->isMoving ? moveSpeed : 0.0f;
    if (p->currentMoveSpeed < targetSpeed) {
        float speedRatio=p->currentMoveSpeed/targetSpeed;
        if (speedRatio<0.0f) speedRatio=0.0f;
        if (speedRatio>1.0f) speedRatio=1.0f;
        /* Acceleration grows with speed: a softer launch and a faster finish. */
        float easeFactor=PLAYER_ACCEL_START_FACTOR+
                         (1.0f-PLAYER_ACCEL_START_FACTOR)*speedRatio;
        p->currentMoveSpeed += PLAYER_ACCELERATION*easeFactor*dt;
        if (p->currentMoveSpeed > targetSpeed) p->currentMoveSpeed = targetSpeed;
    } else if (p->currentMoveSpeed > targetSpeed) {
        p->currentMoveSpeed -= PLAYER_DECELERATION*dt;
        if (p->currentMoveSpeed < targetSpeed) p->currentMoveSpeed = targetSpeed;
    }

    if (p->isMoving && maze) {
        /* Substeps prevent a slow frame from skipping over the final open cell. */
        float remaining = p->currentMoveSpeed * dt;
        float halfSize = PLAYER_COLLISION_SIZE / 2.0f;
        while (remaining > 0.0f && p->isMoving) {
            float step = remaining > 4.0f ? 4.0f : remaining;
            int oldGridX = p->gridX, oldGridY = p->gridY;
            float nextX = p->x + p->dirX * step;
            float nextY = p->y + p->dirY * step;

            if (checkWallCollision(maze, nextX, nextY, halfSize)) {
                p->isMoving = false;
                p->currentMoveSpeed = 0;
                /* Stop at the center of the last cell that was actually reached. */
                p->x = p->gridX * CELL_SIZE + CELL_SIZE / 2.0f;
                p->y = p->gridY * CELL_SIZE + CELL_SIZE / 2.0f;
                break;
            }

            p->x = nextX;
            p->y = nextY;
            updatePlayerGridPos(p);
            if ((p->gridX != oldGridX || p->gridY != oldGridY) &&
                isJunctionAhead(maze, p->gridX, p->gridY, p->dirX, p->dirY)) {
                /* Center on the branch so the player can choose a direction. */
                p->x = p->gridX * CELL_SIZE + CELL_SIZE / 2.0f;
                p->y = p->gridY * CELL_SIZE + CELL_SIZE / 2.0f;
                p->isMoving = false;
                p->currentMoveSpeed = 0;
                break;
            }
            remaining -= step;
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
