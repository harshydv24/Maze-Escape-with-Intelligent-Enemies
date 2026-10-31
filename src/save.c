/*
 * save.c — Binary save/load system
 *
 * Demonstrates: fopen, fread, fwrite, fclose, serialization.
 */

#include "save.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int magic, version, level, difficulty;
    int score, totalScore; float timer;
    int keysRequired, keysCollected;
    float px, py; int pgx, pgy, pHealth; float pStamina;
    int mw, mh, eCount, puCount;
} SaveHdr;

typedef struct { float x,y; int gx,gy,type,state; } SaveEnemy;
typedef struct { float x,y; int gx,gy,type; bool collected; } SavePU;

bool saveGame(const Game *g)
{
    if (!g || !g->maze) return false;
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
    fwrite(&h, sizeof(SaveHdr), 1, f);

    for (int y = 0; y < g->maze->height; y++)
        fwrite(g->maze->grid[y], sizeof(CellType), g->maze->width, f);

    fwrite(&g->maze->keyCount, sizeof(int), 1, f);
    if (g->maze->keyCount > 0 && g->maze->keys)
        fwrite(g->maze->keys, sizeof(GridNode), g->maze->keyCount, f);
    fwrite(&g->maze->start, sizeof(GridNode), 1, f);
    fwrite(&g->maze->exitPos, sizeof(GridNode), 1, f);

    for (int i = 0; i < g->enemyCount; i++) {
        SaveEnemy se = { g->enemies[i].x, g->enemies[i].y,
            g->enemies[i].gridX, g->enemies[i].gridY,
            (int)g->enemies[i].type, (int)g->enemies[i].state };
        fwrite(&se, sizeof(SaveEnemy), 1, f);
    }
    for (int i = 0; i < g->powerupCount; i++) {
        SavePU sp = { g->powerups[i].x, g->powerups[i].y,
            g->powerups[i].gridX, g->powerups[i].gridY,
            (int)g->powerups[i].type, g->powerups[i].collected };
        fwrite(&sp, sizeof(SavePU), 1, f);
    }

    int vs = g->maze->width * g->maze->height;
    fwrite(g->explored, sizeof(bool), vs, f);
    fclose(f);
    return true;
}

bool loadGame(Game *g)
{
    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (!f) { fprintf(stderr, "No save file found.\n"); return false; }

    SaveHdr h;
    if (fread(&h, sizeof(SaveHdr), 1, f) != 1 ||
        h.magic != SAVE_MAGIC || h.version != SAVE_VERSION)
    { fprintf(stderr, "Error: invalid save.\n"); fclose(f); return false; }

    cleanupLevel(g);
    g->currentLevel = h.level;
    g->difficulty = (Difficulty)h.difficulty;
    g->score = h.score; g->totalScore = h.totalScore;
    g->timer = h.timer; g->keysRequired = h.keysRequired;

    g->maze = createMaze(h.mw, h.mh);
    if (!g->maze) { fclose(f); return false; }
    for (int y = 0; y < h.mh; y++)
        fread(g->maze->grid[y], sizeof(CellType), h.mw, f);

    int kc = 0; fread(&kc, sizeof(int), 1, f);
    g->maze->keyCount = kc;
    if (kc > 0) {
        g->maze->keys = (GridNode *)malloc(sizeof(GridNode)*kc);
        if (g->maze->keys) fread(g->maze->keys, sizeof(GridNode), kc, f);
    }
    fread(&g->maze->start, sizeof(GridNode), 1, f);
    fread(&g->maze->exitPos, sizeof(GridNode), 1, f);

    initPlayer(&g->player, h.pgx, h.pgy);
    g->player.x = h.px; g->player.y = h.py;
    g->player.health = h.pHealth; g->player.stamina = h.pStamina;
    g->player.keysCollected = h.keysCollected;

    g->enemyCount = h.eCount;
    if (h.eCount > 0) {
        EnemyType types[MAX_ENEMIES];
        SaveEnemy se[MAX_ENEMIES];
        for (int i = 0; i < h.eCount; i++) {
            fread(&se[i], sizeof(SaveEnemy), 1, f);
            types[i] = (EnemyType)se[i].type;
        }
        g->enemies = createEnemies(h.eCount, g->maze, types, g->difficulty);
        if (g->enemies)
            for (int i = 0; i < h.eCount; i++) {
                g->enemies[i].x=se[i].x; g->enemies[i].y=se[i].y;
                g->enemies[i].gridX=se[i].gx; g->enemies[i].gridY=se[i].gy;
                g->enemies[i].state=(EnemyAIState)se[i].state;
            }
    }

    g->powerupCount = h.puCount;
    if (h.puCount > 0) {
        g->powerups = (PowerUp *)calloc(h.puCount, sizeof(PowerUp));
        if (g->powerups)
            for (int i = 0; i < h.puCount; i++) {
                SavePU sp; fread(&sp, sizeof(SavePU), 1, f);
                g->powerups[i] = (PowerUp){ sp.x,sp.y,sp.gx,sp.gy,
                    (PowerUpType)sp.type, sp.collected };
            }
    }

    int vs = h.mw * h.mh;
    g->visible  = (bool *)calloc(vs, sizeof(bool));
    g->explored = (bool *)calloc(vs, sizeof(bool));
    if (g->explored) fread(g->explored, sizeof(bool), vs, f);

    switch (g->difficulty) {
        case DIFF_EASY: g->visibilityRadius=VISIBILITY_EASY; break;
        case DIFF_HARD: g->visibilityRadius=VISIBILITY_HARD; break;
        default:        g->visibilityRadius=VISIBILITY_NORMAL; break;
    }
    g->camera.target = (Vector2){ g->player.x, g->player.y };
    g->camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
    g->camera.rotation = 0; g->camera.zoom = 1;
    g->fogEnabled = true;
    g->state = STATE_PLAYING;

    fclose(f);
    return true;
}

bool saveFileExists(void)
{
    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (f) { fclose(f); return true; }
    return false;
}
