/*
 * save.c — Binary save/load system
 *
 * Demonstrates: fopen, fread, fwrite, fclose, serialization.
 */

#include "save.h"
#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_SAVE_DIMENSION 3
#define MAX_SAVE_DIMENSION 51
typedef struct {
    int magic, version, level, difficulty;
    int score, totalScore; float timer;
    int keysRequired, keysCollected;
    float px, py; int pgx, pgy, pHealth; float pStamina;
    int mw, mh, eCount, puCount;
} SaveHdr;

typedef struct { float x,y; int gx,gy,type,state; } SaveEnemy;
typedef struct { float x,y; int gx,gy,type; bool collected; } SavePU;

static bool writeBlock(FILE *file, const void *data, size_t size, size_t count)
{
    if (count == 0) return true;
    return fwrite(data, size, count, file) == count;
}

static bool readBlock(FILE *file, void *data, size_t size, size_t count)
{
    if (count == 0) return true;
    return fread(data, size, count, file) == count;
}

static bool validDimension(int dimension)
{
    return dimension >= MIN_SAVE_DIMENSION &&
           dimension <= MAX_SAVE_DIMENSION && (dimension % 2) != 0;
}

static bool validGridNode(GridNode node, int width, int height)
{
    return node.x >= 0 && node.x < width && node.y >= 0 && node.y < height;
}

static bool validHeader(const SaveHdr *header)
{
    int cellCount;

    if (header->magic != SAVE_MAGIC || header->version != SAVE_VERSION ||
        !validDimension(header->mw) || !validDimension(header->mh) ||
        header->level < 1 || header->level > MAX_LEVELS ||
        header->difficulty < DIFF_EASY || header->difficulty > DIFF_HARD ||
        header->eCount < 0 || header->eCount > MAX_ENEMIES ||
        header->puCount < 0 || header->puCount > MAX_POWERUPS ||
        !isfinite(header->timer) || header->timer < 0.0f ||
        !isfinite(header->px) || !isfinite(header->py) ||
        !isfinite(header->pStamina))
        return false;

    cellCount = header->mw * header->mh;
    return header->keysRequired >= 0 && header->keysRequired <= cellCount &&
           header->keysCollected >= 0 &&
           header->keysCollected <= header->keysRequired &&
           validGridNode((GridNode){header->pgx, header->pgy},
                         header->mw, header->mh);
}

static bool validCellType(CellType cell)
{
    return cell >= CELL_WALL && cell <= CELL_KEY;
}

static bool validEnemyRecord(const SaveEnemy *enemy, int width, int height)
{
    return isfinite(enemy->x) && isfinite(enemy->y) &&
           validGridNode((GridNode){enemy->gx, enemy->gy}, width, height) &&
           enemy->type >= ENEMY_PATROL && enemy->type <= ENEMY_SMART &&
           enemy->state >= AI_PATROL && enemy->state <= AI_RETURN_TO_PATROL;
}

static bool validPowerUpRecord(const SavePU *powerup, int width, int height)
{
    return isfinite(powerup->x) && isfinite(powerup->y) &&
           validGridNode((GridNode){powerup->gx, powerup->gy}, width, height) &&
           powerup->type >= POWERUP_SPEED && powerup->type < POWERUP_COUNT;
}

bool saveGame(const Game *g)
{
    int vs;
    if (!g || !g->maze || !g->maze->grid ||
        !validDimension(g->maze->width) ||
        !validDimension(g->maze->height) || g->enemyCount < 0 ||
        g->enemyCount > MAX_ENEMIES || g->powerupCount < 0 ||
        g->powerupCount > MAX_POWERUPS || g->maze->keyCount < 0 ||
        g->maze->keyCount > g->maze->width * g->maze->height ||
        (g->maze->keyCount > 0 && !g->maze->keys) ||
        (g->enemyCount > 0 && !g->enemies) ||
        (g->powerupCount > 0 && !g->powerups))
        return false;

    vs = g->maze->width * g->maze->height;
    if (!g->explored) return false;

    FILE *f = fopen(SAVE_FILE_PATH, "wb");
    if (!f) { fprintf(stderr, "Error: cannot create save.\n"); return false; }

    SaveHdr h = {0};
    h.magic=SAVE_MAGIC; h.version=SAVE_VERSION;
    h.level=g->currentLevel; h.difficulty=(int)g->difficulty;
    h.score=g->score; h.totalScore=g->totalScore; h.timer=g->timer;
    h.keysRequired=g->keysRequired; h.keysCollected=g->player.keysCollected;
    h.px=g->player.x; h.py=g->player.y;
    h.pgx=g->player.gridX; h.pgy=g->player.gridY;
    h.pHealth=g->player.health; h.pStamina=g->player.stamina;
    h.mw=g->maze->width; h.mh=g->maze->height;
    h.eCount=g->enemyCount; h.puCount=g->powerupCount;
    if (!writeBlock(f, &h, sizeof(h), 1)) goto write_fail;

    for (int y = 0; y < g->maze->height; y++)
        if (!g->maze->grid[y] ||
            !writeBlock(f, g->maze->grid[y], sizeof(CellType), g->maze->width))
            goto write_fail;

    if (!writeBlock(f, &g->maze->keyCount, sizeof(int), 1) ||
        !writeBlock(f, g->maze->keys, sizeof(GridNode), g->maze->keyCount) ||
        !writeBlock(f, &g->maze->start, sizeof(GridNode), 1) ||
        !writeBlock(f, &g->maze->exitPos, sizeof(GridNode), 1))
        goto write_fail;

    for (int i = 0; i < g->enemyCount; i++) {
        SaveEnemy se = { g->enemies[i].x, g->enemies[i].y,
            g->enemies[i].gridX, g->enemies[i].gridY,
            (int)g->enemies[i].type, (int)g->enemies[i].state };
        if (!writeBlock(f, &se, sizeof(se), 1)) goto write_fail;
    }
    for (int i = 0; i < g->powerupCount; i++) {
        SavePU sp = { g->powerups[i].x, g->powerups[i].y,
            g->powerups[i].gridX, g->powerups[i].gridY,
            (int)g->powerups[i].type, g->powerups[i].collected };
        if (!writeBlock(f, &sp, sizeof(sp), 1)) goto write_fail;
    }

    if (!writeBlock(f, g->explored, sizeof(bool), (size_t)vs))
        goto write_fail;
    fclose(f);
    return true;

write_fail:
    fprintf(stderr, "Error: failed while writing save.\n");
    fclose(f);
    return false;
}

bool loadGame(Game *g)
{
    int kc, vs;
    EnemyType types[MAX_ENEMIES];
    SaveEnemy savedEnemies[MAX_ENEMIES];

    if (!g) return false;

    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (!f) { fprintf(stderr, "No save file found.\n"); return false; }

    SaveHdr h;
    if (!readBlock(f, &h, sizeof(h), 1) || !validHeader(&h))
    { fprintf(stderr, "Error: invalid save.\n"); fclose(f); return false; }

    cleanupLevel(g);
    g->currentLevel = h.level;
    g->difficulty = (Difficulty)h.difficulty;
    g->score = h.score; g->totalScore = h.totalScore;
    g->timer = h.timer; g->keysRequired = h.keysRequired;

    g->maze = createMaze(h.mw, h.mh);
    if (!g->maze) goto load_fail;
    for (int y = 0; y < h.mh; y++)
        if (!readBlock(f, g->maze->grid[y], sizeof(CellType), h.mw))
            goto load_fail;

    for (int y = 0; y < h.mh; y++)
        for (int x = 0; x < h.mw; x++)
            if (!validCellType(g->maze->grid[y][x])) goto load_fail;

    kc = 0;
    if (!readBlock(f, &kc, sizeof(kc), 1) || kc < 0 ||
        kc > h.mw * h.mh)
        goto load_fail;
    g->maze->keyCount = kc;
    if (kc > 0) {
        g->maze->keys = (GridNode *)malloc(sizeof(GridNode)*kc);
        if (!g->maze->keys ||
            !readBlock(f, g->maze->keys, sizeof(GridNode), (size_t)kc))
            goto load_fail;
    }
    if (!readBlock(f, &g->maze->start, sizeof(GridNode), 1) ||
        !readBlock(f, &g->maze->exitPos, sizeof(GridNode), 1) ||
        !validGridNode(g->maze->start, h.mw, h.mh) ||
        !validGridNode(g->maze->exitPos, h.mw, h.mh))
        goto load_fail;

    initPlayer(&g->player, h.pgx, h.pgy);
    g->player.x = h.px; g->player.y = h.py;
    g->player.health = h.pHealth; g->player.stamina = h.pStamina;
    g->player.keysCollected = h.keysCollected;

    g->enemyCount = h.eCount;
    if (h.eCount > 0) {
        for (int i = 0; i < h.eCount; i++) {
            if (!readBlock(f, &savedEnemies[i], sizeof(SaveEnemy), 1) ||
                !validEnemyRecord(&savedEnemies[i], h.mw, h.mh))
                goto load_fail;
            types[i] = (EnemyType)savedEnemies[i].type;
        }
        g->enemies = createEnemies(h.eCount, g->maze, types, g->difficulty);
        if (!g->enemies) goto load_fail;
        for (int i = 0; i < h.eCount; i++) {
            g->enemies[i].x=savedEnemies[i].x; g->enemies[i].y=savedEnemies[i].y;
            g->enemies[i].gridX=savedEnemies[i].gx; g->enemies[i].gridY=savedEnemies[i].gy;
            g->enemies[i].state=(EnemyAIState)savedEnemies[i].state;
        }
    }

    g->powerupCount = h.puCount;
    if (h.puCount > 0) {
        g->powerups = (PowerUp *)calloc(h.puCount, sizeof(PowerUp));
        if (!g->powerups) goto load_fail;
        for (int i = 0; i < h.puCount; i++) {
            SavePU sp;
            if (!readBlock(f, &sp, sizeof(sp), 1) ||
                !validPowerUpRecord(&sp, h.mw, h.mh))
                goto load_fail;
            g->powerups[i] = (PowerUp){ sp.x,sp.y,sp.gx,sp.gy,
                (PowerUpType)sp.type, sp.collected };
        }
    }

    vs = h.mw * h.mh;
    g->visible  = (bool *)calloc(vs, sizeof(bool));
    g->explored = (bool *)calloc(vs, sizeof(bool));
    if (!g->visible || !g->explored ||
        !readBlock(f, g->explored, sizeof(bool), (size_t)vs))
        goto load_fail;

    switch (g->difficulty) {
        case DIFF_EASY: g->visibilityRadius=VISIBILITY_EASY; break;
        case DIFF_HARD: g->visibilityRadius=VISIBILITY_HARD; break;
        default:        g->visibilityRadius=VISIBILITY_NORMAL; break;
    }
    g->camera.target = (Vector2){ g->player.x, g->player.y };
    g->camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
    g->camera.rotation = 0; g->camera.zoom = 1;
    g->fogEnabled = false;
    g->state = STATE_PLAYING;

    fclose(f);
    return true;

load_fail:
    fprintf(stderr, "Error: invalid or incomplete save.\n");
    fclose(f);
    cleanupLevel(g);
    return false;
}

bool saveFileExists(void)
{
    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (f) { fclose(f); return true; }
    return false;
}
