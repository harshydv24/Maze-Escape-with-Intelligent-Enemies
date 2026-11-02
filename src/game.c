/*
 * game.c — Game state management, level progression, core gameplay loop
 *
 * Demonstrates: state machine dispatch, level configuration,
 * fog-of-war visibility computation, integration of all subsystems.
 */

#include "game.h"
#include "ui.h"
#include "save.h"
#include "collision.h"
#include "ai.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- Level configuration helper ---- */

static void getLevelCfg(int level, Difficulty d,
    int *mw, int *mh, int *ec, EnemyType et[MAX_ENEMIES],
    int *kc, int *pc, float *timer)
{
    int base; float bt;
    switch (d) {
        case DIFF_EASY: base=MAZE_SIZE_EASY;  bt=TIMER_EASY;  break;
        case DIFF_HARD: base=MAZE_SIZE_HARD;  bt=TIMER_HARD;  break;
        default:        base=MAZE_SIZE_NORMAL; bt=TIMER_NORMAL;break;
    }
    int sz = base + (level-1)*4;
    if (sz%2==0) sz++;
    if (sz > 51) sz = 51;
    *mw = *mh = sz;
    *timer = bt + (level-1)*15.0f;
    *kc = level >= 4 ? 2 : 1;
    *pc = 2 + (level-1);
    if (*pc > MAX_POWERUPS) *pc = MAX_POWERUPS;
    int baseEnemies = d == DIFF_EASY ? 1 : d == DIFF_HARD ? 3 : 2;
    *ec = baseEnemies + (level - 1) / 2;
    if (*ec > MAX_ENEMIES) *ec = MAX_ENEMIES;

    for (int i = 0; i < *ec; i++) {
        if (level<=1) et[i] = ENEMY_PATROL;
        else if (level==2) et[i] = i==0 ? ENEMY_PATROL : ENEMY_HUNTER;
        else if (level==3) et[i] = i==0 ? ENEMY_PATROL : ENEMY_HUNTER;
        else if (level==4) { if(i==0) et[i]=ENEMY_PATROL; else if(i<=2) et[i]=ENEMY_HUNTER; else et[i]=ENEMY_SMART; }
        else { if(i==0) et[i]=ENEMY_PATROL; else if(i==1) et[i]=ENEMY_HUNTER; else et[i]=ENEMY_SMART; }
    }
}

/* Put an optional score cache at a distant dead end to reward exploration. */
static GridNode findBonusDeadEnd(Game *g)
{
    GridNode best = {-1, -1};
    int bestDistance = 7;
    Maze *m = g->maze;
    for (int y = 1; y < m->height - 1; y++) for (int x = 1; x < m->width - 1; x++) {
        if (m->grid[y][x] != CELL_PATH) continue;
        int degree = isCellWalkable(m,x-1,y) + isCellWalkable(m,x+1,y) +
                     isCellWalkable(m,x,y-1) + isCellWalkable(m,x,y+1);
        int distance = abs(x-m->start.x) + abs(y-m->start.y);
        if (degree != 1 || distance < bestDistance) continue;
        bool occupied = false;
        for (int i=0;i<g->powerupCount;i++)
            if (g->powerups[i].gridX==x && g->powerups[i].gridY==y) occupied=true;
        for (int i=0;i<g->enemyCount;i++)
            if (g->enemies[i].gridX==x && g->enemies[i].gridY==y) occupied=true;
        for (int i=0;i<m->keyCount;i++)
            if (m->keys[i].x==x && m->keys[i].y==y) occupied=true;
        if (!occupied) { best=(GridNode){x,y}; bestDistance=distance; }
    }
    return best;
}

void configureMazeCamera(Game *g)
{
    if (!g || !g->maze) return;
    float worldWidth=(float)(g->maze->width*CELL_SIZE);
    float worldHeight=(float)(g->maze->height*CELL_SIZE);
    float availableWidth=(float)GetScreenWidth()-64.0f;
    float availableHeight=(float)GetScreenHeight()-100.0f;
    if (availableWidth<1.0f) availableWidth=1.0f;
    if (availableHeight<1.0f) availableHeight=1.0f;
    float zoomX=availableWidth/worldWidth;
    float zoomY=availableHeight/worldHeight;

    g->camera.target=(Vector2){worldWidth/2.0f,worldHeight/2.0f};
    g->camera.offset=(Vector2){GetScreenWidth()/2.0f,GetScreenHeight()/2.0f};
    g->camera.rotation=0;
    g->camera.zoom=zoomX<zoomY?zoomX:zoomY;
}

/* ---- Init / Cleanup ---- */

void initGame(Game *g)
{
    memset(g, 0, sizeof(Game));
    g->state = STATE_MENU;
    g->maxMenuItems = 6;
    g->fogEnabled = false;
    g->aiSelectedAlgo = -1;
    strncpy(g->playerName, "PLAYER", MAX_NAME_LENGTH-1);
    g->nameLength = (int)strlen(g->playerName);
    g->highScoreCount = loadHighScores(g->highScores, MAX_HIGH_SCORES);
    initAudio(&g->audio);
    loadGameTextures(&g->textures);
}

void cleanupLevel(Game *g)
{
    if (g->maze)     { destroyMaze(g->maze);     g->maze = NULL; }
    if (g->enemies)  { destroyEnemies(g->enemies, g->enemyCount); g->enemies = NULL; }
    if (g->powerups) { destroyPowerUps(g->powerups); g->powerups = NULL; }
    free(g->visible);  g->visible  = NULL;
    free(g->explored); g->explored = NULL;
    g->enemyCount = g->powerupCount = 0;
}

void cleanupGame(Game *g)
{
    cleanupLevel(g);
    if (g->demoMaze) { destroyMaze(g->demoMaze); g->demoMaze = NULL; }
    freeAIComparisonResults(g->aiResults);
    unloadAudio(&g->audio);
    unloadGameTextures(&g->textures);
}

static void recordRunScore(Game *g)
{
    insertHighScore(g->highScores, &g->highScoreCount, MAX_HIGH_SCORES,
                    g->playerName, g->totalScore);
    saveHighScores(g->highScores, g->highScoreCount);
}

/* ---- New game / level management ---- */

void startNewGame(Game *g, Difficulty d)
{
    g->difficulty = d; g->currentLevel = 1;
    g->totalScore = g->score = 0;
    startLevel(g);
}

void startLevel(Game *g)
{
    Maze *maze = NULL;
    Enemy *enemies = NULL;
    PowerUp *powerups = NULL;
    bool *visible = NULL;
    bool *explored = NULL;

    cleanupLevel(g);
    int mw,mh,ec,kc,pc; EnemyType et[MAX_ENEMIES]; float tm;
    getLevelCfg(g->currentLevel, g->difficulty, &mw,&mh,&ec,et,&kc,&pc,&tm);

    int att = 0;
    bool placed = false;
    do {
        if (maze) destroyMaze(maze);
        maze = createMaze(mw, mh);
        if (!maze) goto level_fail;
        generateMaze(maze);
        placed = placeMazeObjects(maze, kc);
        att++;
    } while ((!placed || !validateMaze(maze)) && att < 100);

    if (!placed || !validateMaze(maze)) goto level_fail;

    enemies = createEnemies(ec, maze, et, g->difficulty);
    if (ec > 0 && !enemies) goto level_fail;
    powerups = createPowerUps(pc, maze);
    if (pc > 0 && !powerups) goto level_fail;

    int vs = mw * mh;
    visible  = (bool *)calloc(vs, sizeof(bool));
    explored = (bool *)calloc(vs, sizeof(bool));
    if (!visible || !explored) goto level_fail;

    g->maze = maze; maze = NULL;
    g->enemyCount = ec;
    g->enemies = enemies; enemies = NULL;
    g->powerupCount = pc;
    g->powerups = powerups; powerups = NULL;
    g->visible = visible; visible = NULL;
    g->explored = explored; explored = NULL;
    initPlayer(&g->player, g->maze->start.x, g->maze->start.y);
    g->timer = tm; g->score = 0; g->keysRequired = kc;

    switch (g->difficulty) {
        case DIFF_EASY: g->visibilityRadius=VISIBILITY_EASY; break;
        case DIFF_HARD: g->visibilityRadius=VISIBILITY_HARD; break;
        default:        g->visibilityRadius=VISIBILITY_NORMAL; break;
    }
    configureMazeCamera(g);
    g->fogEnabled = false;
    g->decoysAvailable = PLAYER_START_DECOYS;
    g->decoyVisibleTimer = 0;
    g->bonusPosition = findBonusDeadEnd(g);
    g->bonusAvailable = g->bonusPosition.x >= 0;
    g->cameraShakeTimer = g->screenFlashTimer = 0;
    g->state = STATE_PLAYING;
    addNotification(g, TextFormat("Level %d", g->currentLevel), COLOR_TITLE);
    return;

level_fail:
    destroyMaze(maze);
    destroyEnemies(enemies, ec);
    destroyPowerUps(powerups);
    free(visible);
    free(explored);
    g->state = STATE_MENU;
    g->menuSelection = 0;
    addNotification(g, "Unable to start level", COLOR_TITLE);
}

void nextLevel(Game *g)
{
    int ls = calculateLevelScore(g->currentLevel, g->timer,
        g->player.health, g->player.keysCollected, g->player.powerupsCollected) + g->score;
    g->score = ls; g->totalScore += ls;
    if (g->currentLevel >= MAX_LEVELS) {
        recordRunScore(g);
        g->state = STATE_GAME_OVER;
    }
    else { g->currentLevel++; g->state = STATE_LEVEL_COMPLETE; }
}

/* ---- Visibility / Fog of War ---- */

void computeVisibility(Game *g)
{
    if (!g->maze || !g->visible || !g->explored) return;
    int w = g->maze->width, h = g->maze->height;
    memset(g->visible, 0, sizeof(bool)*w*h);
    int px=g->player.gridX, py=g->player.gridY, r=g->visibilityRadius;
    for (int dy=-r; dy<=r; dy++)
        for (int dx=-r; dx<=r; dx++) {
            int cx=px+dx, cy=py+dy;
            if (cx<0||cx>=w||cy<0||cy>=h) continue;
            if (dx*dx+dy*dy > r*r) continue;
            if (checkLineOfSight(g->maze, px, py, cx, cy)) {
                g->visible[cy*w+cx] = true;
                g->explored[cy*w+cx] = true;
            }
        }
}

/* ---- Notifications ---- */

void addNotification(Game *g, const char *text, Color c)
{
    if (g->notificationCount >= MAX_NOTIFICATIONS) {
        for (int i=0;i<MAX_NOTIFICATIONS-1;i++) g->notifications[i]=g->notifications[i+1];
        g->notificationCount = MAX_NOTIFICATIONS-1;
    }
    Notification *n = &g->notifications[g->notificationCount++];
    strncpy(n->text, text, 63); n->text[63]='\0';
    n->timer = NOTIFICATION_DURATION; n->color = c;
}

void updateNotifications(Game *g, float dt)
{
    for (int i=g->notificationCount-1; i>=0; i--) {
        g->notifications[i].timer -= dt;
        if (g->notifications[i].timer <= 0) {
            for (int j=i; j<g->notificationCount-1; j++)
                g->notifications[j] = g->notifications[j+1];
            g->notificationCount--;
        }
    }
}

/* ---- State update dispatch ---- */

void updateGame(Game *g, float dt)
{
    updateMusic(&g->audio);
    updateNotifications(g, dt);
    switch (g->state) {
        case STATE_MENU:           updateMenu(g);           break;
        case STATE_DIFFICULTY:     updateDifficulty(g);     break;
        case STATE_PLAYING:        updatePlaying(g, dt);    break;
        case STATE_PAUSED:         updatePaused(g);         break;
        case STATE_GAME_OVER:      updateGameOver(g);       break;
        case STATE_LEVEL_COMPLETE: updateLevelComplete(g);  break;
        case STATE_HIGH_SCORES:    updateHighScores(g);     break;
        case STATE_INSTRUCTIONS:   updateInstructions(g);   break;
        case STATE_SETTINGS:       updateSettings(g);       break;
        case STATE_AI_DEMO:        updateAIDemo(g);         break;
        case STATE_NAME_ENTRY:     updateNameEntry(g);      break;
    }
}

void drawGameFrame(Game *g)
{
    switch (g->state) {
        case STATE_MENU:           drawMenu(g);           break;
        case STATE_DIFFICULTY:     drawDifficulty(g);     break;
        case STATE_PLAYING:        drawPlaying(g);        break;
        case STATE_PAUSED:         drawPaused(g);         break;
        case STATE_GAME_OVER:      drawGameOver(g);       break;
        case STATE_LEVEL_COMPLETE: drawLevelComplete(g);  break;
        case STATE_HIGH_SCORES:    drawHighScores(g);     break;
        case STATE_INSTRUCTIONS:   drawInstructions(g);   break;
        case STATE_SETTINGS:       drawSettings(g);       break;
        case STATE_AI_DEMO:        drawAIDemo(g);         break;
        case STATE_NAME_ENTRY:     drawNameEntry(g);      break;
    }
    if (g->showFPS) DrawFPS(10, GetScreenHeight()-30);
}

/* ---- Menu ---- */

void updateMenu(Game *g)
{
    g->maxMenuItems = 7;
    if (IsKeyPressed(KEY_UP))   { g->menuSelection--; if (g->menuSelection<0) g->menuSelection=g->maxMenuItems-1; playGameSound(&g->audio,g->audio.sndMenuMove); }
    if (IsKeyPressed(KEY_DOWN)) { g->menuSelection++; if (g->menuSelection>=g->maxMenuItems) g->menuSelection=0; playGameSound(&g->audio,g->audio.sndMenuMove); }
    if (IsKeyPressed(KEY_ENTER)) {
        playGameSound(&g->audio,g->audio.sndMenuSelect);
        switch (g->menuSelection) {
            case 0: g->state=STATE_DIFFICULTY; g->menuSelection=0; break;
            case 1: if (loadGame(g)) { g->state=STATE_PLAYING; }
                    else { addNotification(g,"No save file found.",COLOR_ENEMY_BFS); } break;
            case 2: g->state=STATE_HIGH_SCORES;
                    g->highScoreCount=loadHighScores(g->highScores,MAX_HIGH_SCORES); break;
            case 3: g->state=STATE_INSTRUCTIONS; break;
                case 4: g->previousState=STATE_MENU; g->state=STATE_SETTINGS; g->menuSelection=0; break;
                case 5: g->state=STATE_AI_DEMO; g->aiResultsComputed=false;
                    g->aiSelectedAlgo=-1; g->menuSelection=0; break;
                case 6: CloseWindow(); break;
        }
    }
}

void updateDifficulty(Game *g)
{
    g->maxMenuItems = 3;
    if (IsKeyPressed(KEY_UP))   { g->menuSelection--; if (g->menuSelection<0) g->menuSelection=2; }
    if (IsKeyPressed(KEY_DOWN)) { g->menuSelection++; if (g->menuSelection>2) g->menuSelection=0; }
    if (IsKeyPressed(KEY_ENTER)) {
        playGameSound(&g->audio,g->audio.sndMenuSelect);
        g->difficulty=(Difficulty)g->menuSelection;
        g->playerName[0]='\0';
        g->nameLength=0;
        g->state=STATE_NAME_ENTRY;
    }
    if (IsKeyPressed(KEY_ESCAPE)) { g->state=STATE_MENU; g->menuSelection=0; }
}

/* ---- Main gameplay update ---- */

void updatePlaying(Game *g, float dt)
{
    if (IsKeyPressed(KEY_ESCAPE)) { g->previousState=STATE_PLAYING; g->state=STATE_PAUSED; g->menuSelection=0; return; }
    if (IsKeyPressed(KEY_F1)) g->debugMode = !g->debugMode;
    if (IsKeyPressed(KEY_F2)) g->fogEnabled = !g->fogEnabled;

    int oldGridX = g->player.gridX, oldGridY = g->player.gridY;
    updatePlayer(&g->player, g->maze, dt);
    g->worldTime += dt;
    if (g->footstepTimer > 0) g->footstepTimer -= dt;
    if ((oldGridX != g->player.gridX || oldGridY != g->player.gridY) && g->footstepTimer <= 0) {
        playGameSound(&g->audio, g->audio.sndFootstep);
        g->footstepTimer = g->player.isSprinting ? 0.17f : 0.25f;
    }
    if (g->decoyVisibleTimer > 0) g->decoyVisibleTimer -= dt;
    if (g->screenFlashTimer > 0) g->screenFlashTimer -= dt;
    if (g->pickupEffectTimer > 0) g->pickupEffectTimer -= dt;
    if (g->cameraShakeTimer > 0) g->cameraShakeTimer -= dt;
    for (int i=0;i<g->enemyCount;i++) if (g->enemies[i].alertTimer > 0) g->enemies[i].alertTimer -= dt;

    /* Q throws a sound decoy a few cells ahead; enemies path toward the noise. */
    if (IsKeyPressed(KEY_Q) && g->decoysAvailable > 0) {
        int dx=(int)g->player.dirX, dy=(int)g->player.dirY;
        if (dx==0 && dy==0) dy=1;
        GridNode target={g->player.gridX,g->player.gridY};
        for (int step=0;step<3;step++) {
            if (!isCellWalkable(g->maze,target.x+dx,target.y+dy)) break;
            target.x+=dx; target.y+=dy;
        }
        g->decoyPosition=target; g->decoyVisibleTimer=DECOY_VISIBLE_DURATION;
        g->decoysAvailable--;
        for (int i=0;i<g->enemyCount;i++) {
            Enemy *e=&g->enemies[i];
            e->state=AI_INVESTIGATE; e->lastKnownX=target.x*CELL_SIZE+CELL_SIZE/2.0f;
            e->lastKnownY=target.y*CELL_SIZE+CELL_SIZE/2.0f;
            e->searchTimer=DECOY_LURE_DURATION;
            e->playerDetected=false; e->pathTimer=AI_UPDATE_INTERVAL;
            free(e->path); e->path=NULL; e->pathLength=e->pathIndex=0;
        }
        addNotification(g,"DECOY THROWN!",COLOR_POWERUP_INVIS);
        playGameSound(&g->audio,g->audio.sndDecoy);
    }

    /* Key pickup */
    for (int i=0; i<g->maze->keyCount; i++) {
        GridNode k = g->maze->keys[i];
        if (g->maze->grid[k.y][k.x]==CELL_KEY && checkPlayerCellOverlap(&g->player,k.x,k.y)) {
            g->maze->grid[k.y][k.x] = CELL_PATH;
            g->player.keysCollected++;
            g->pickupEffectPosition=(Vector2){k.x*CELL_SIZE+CELL_SIZE/2.0f,k.y*CELL_SIZE+CELL_SIZE/2.0f};
            g->pickupEffectColor=COLOR_KEY; g->pickupEffectTimer=0.38f;
            addNotification(g,"KEY FOUND!",COLOR_KEY);
            g->screenFlashTimer=0.12f; g->screenFlashColor=(Color){241,250,140,70};
            playGameSound(&g->audio,g->audio.sndPickupKey);
            if (g->player.keysCollected>=g->keysRequired) {
                addNotification(g,"EXIT UNLOCKED!",COLOR_EXIT_UNLOCKED);
                g->screenFlashTimer=0.28f; g->screenFlashColor=(Color){80,250,123,85};
                playGameSound(&g->audio,g->audio.sndExitUnlock);
            }
        }
    }

    /* Power-up pickup */
    for (int i=0; i<g->powerupCount; i++) {
        if (g->powerups[i].collected) continue;
        if (checkPlayerCellOverlap(&g->player,g->powerups[i].gridX,g->powerups[i].gridY)) {
            g->powerups[i].collected = true;
            g->player.powerupsCollected++;
            g->pickupEffectPosition=(Vector2){g->powerups[i].x,g->powerups[i].y};
            g->pickupEffectColor=COLOR_POWERUP_INVIS; g->pickupEffectTimer=0.38f;
            g->screenFlashTimer=0.16f; g->screenFlashColor=(Color){139,233,253,55};
            playGameSound(&g->audio,g->audio.sndPickupPowerup);
            switch (g->powerups[i].type) {
                case POWERUP_SPEED:        g->player.speedBoostTimer=POWERUP_SPEED_DURATION; addNotification(g,"SPEED BOOST!",COLOR_POWERUP_SPEED); break;
                case POWERUP_INVISIBILITY: g->player.invisibilityTimer=POWERUP_INVIS_DURATION; addNotification(g,"INVISIBILITY!",COLOR_POWERUP_INVIS); break;
                case POWERUP_FREEZE:       g->player.freezeTimer=POWERUP_FREEZE_DURATION; addNotification(g,"ENEMIES FROZEN!",COLOR_POWERUP_FREEZE); break;
                case POWERUP_HEALTH:       g->player.health+=POWERUP_HEALTH_AMOUNT;
                    if(g->player.health>g->player.maxHealth)g->player.health=g->player.maxHealth;
                    addNotification(g,"HEALTH RESTORED!",COLOR_POWERUP_HEALTH); break;
                default: break;
            }
        }
    }

    /* Optional cache is tucked into a dead end; reaching it pays a score bonus. */
    if (g->bonusAvailable && checkPlayerCellOverlap(&g->player,g->bonusPosition.x,g->bonusPosition.y)) {
        g->bonusAvailable=false; g->score+=BONUS_CACHE_SCORE;
        g->pickupEffectPosition=(Vector2){g->bonusPosition.x*CELL_SIZE+CELL_SIZE/2.0f,
                                           g->bonusPosition.y*CELL_SIZE+CELL_SIZE/2.0f};
        g->pickupEffectColor=COLOR_POWERUP_SPEED; g->pickupEffectTimer=0.45f;
        addNotification(g,"SECRET CACHE! +500",COLOR_KEY);
        g->screenFlashTimer=0.24f; g->screenFlashColor=(Color){255,184,108,85};
        playGameSound(&g->audio,g->audio.sndPickupPowerup);
    }

    /* Exit check */
    if (checkPlayerCellOverlap(&g->player,g->maze->exitPos.x,g->maze->exitPos.y)) {
        if (g->player.keysCollected>=g->keysRequired) {
            playGameSound(&g->audio,g->audio.sndLevelComplete);
            nextLevel(g); return;
        }
    }

    /* Enemies */
    for (int i=0; i<g->enemyCount; i++) {
        EnemyAIState oldState=g->enemies[i].state;
        if (g->enemies[i].updateAI)
            g->enemies[i].updateAI(&g->enemies[i],&g->player,g->maze,dt);
        if (oldState!=AI_CHASE && g->enemies[i].state==AI_CHASE) {
            g->enemies[i].alertTimer=1.3f;
            addNotification(g,"ENEMY SPOTTED!",COLOR_ENEMY_BFS);
            playGameSound(&g->audio,g->audio.sndEnemyDetect);
        }
        if (checkPlayerEnemyCollision(&g->player,&g->enemies[i])) {
            if (g->player.invincibilityTimer<=0 && g->player.invisibilityTimer<=0) {
                g->player.health -= ENEMY_DAMAGE;
                g->player.invincibilityTimer = PLAYER_INVINCIBILITY;
                g->screenFlashTimer=0.22f; g->screenFlashColor=(Color){255,85,85,100};
                g->cameraShakeTimer=0.22f; g->cameraShakeIntensity=6.0f;
                addNotification(g,"DAMAGE!",COLOR_ENEMY_BFS);
                playGameSound(&g->audio,g->audio.sndDamage);
                if (g->player.health<=0) {
                    g->player.health=0;
                    g->score+=calculateLevelScore(g->currentLevel,g->timer,0,g->player.keysCollected,g->player.powerupsCollected);
                    g->totalScore+=g->score;
                    playGameSound(&g->audio,g->audio.sndGameOver);
                    recordRunScore(g);
                    g->state=STATE_GAME_OVER; return;
                }
            }
        }
    }

    /* Timer */
    g->timer -= dt;
    if (g->timer<=0) {
        g->timer=0;
        g->score+=calculateLevelScore(g->currentLevel,0,g->player.health,g->player.keysCollected,g->player.powerupsCollected);
        g->totalScore+=g->score;
        playGameSound(&g->audio,g->audio.sndGameOver);
        recordRunScore(g);
        g->state=STATE_GAME_OVER; return;
    }

    computeVisibility(g);
    float shake=g->cameraShakeTimer>0?g->cameraShakeIntensity*(g->cameraShakeTimer/0.22f):0;
    g->camera.offset=(Vector2){GetScreenWidth()/2.0f+sinf(g->worldTime*91.0f)*shake,
                               GetScreenHeight()/2.0f+cosf(g->worldTime*79.0f)*shake};
}

/* ---- Pause ---- */

void updatePaused(Game *g)
{
    g->maxMenuItems = 5;
    if (IsKeyPressed(KEY_UP))   { g->menuSelection--; if (g->menuSelection<0) g->menuSelection=4; }
    if (IsKeyPressed(KEY_DOWN)) { g->menuSelection++; if (g->menuSelection>4) g->menuSelection=0; }
    if (IsKeyPressed(KEY_ESCAPE)) g->state=STATE_PLAYING;
    if (IsKeyPressed(KEY_ENTER)) {
        switch (g->menuSelection) {
            case 0: g->state=STATE_PLAYING; break;
            case 1: startLevel(g); break;
            case 2: g->previousState=STATE_PAUSED; g->state=STATE_SETTINGS; g->menuSelection=0; break;
            case 3: if(saveGame(g)) addNotification(g,"Game Saved!",COLOR_PLAYER);
                    else addNotification(g,"Save Failed!",COLOR_ENEMY_BFS);
                    g->state=STATE_PLAYING; break;
            case 4: cleanupLevel(g); g->state=STATE_MENU; g->menuSelection=0; break;
        }
    }
}

void updateGameOver(Game *g)    { if (IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_ESCAPE)) { g->state=STATE_MENU; g->menuSelection=0; } }
void updateLevelComplete(Game *g) { if (IsKeyPressed(KEY_ENTER)) startLevel(g); if (IsKeyPressed(KEY_ESCAPE)) { g->state=STATE_MENU; g->menuSelection=0; } }
void updateHighScores(Game *g)  { if (IsKeyPressed(KEY_ESCAPE)) { g->state=STATE_MENU; g->menuSelection=0; } }
void updateInstructions(Game *g){ if (IsKeyPressed(KEY_ESCAPE)) { g->state=STATE_MENU; g->menuSelection=0; } }

void updateSettings(Game *g)
{
    g->maxMenuItems = 4;
    if (IsKeyPressed(KEY_UP))   { g->menuSelection--; if (g->menuSelection<0) g->menuSelection=3; }
    if (IsKeyPressed(KEY_DOWN)) { g->menuSelection++; if (g->menuSelection>3) g->menuSelection=0; }
    if (IsKeyPressed(KEY_LEFT)||IsKeyPressed(KEY_RIGHT)) {
        float d = IsKeyPressed(KEY_RIGHT)?0.1f:-0.1f;
        switch (g->menuSelection) {
            case 0: g->audio.soundVolume+=d; if(g->audio.soundVolume<0)g->audio.soundVolume=0; if(g->audio.soundVolume>1)g->audio.soundVolume=1; break;
            case 1: g->audio.musicVolume+=d; if(g->audio.musicVolume<0)g->audio.musicVolume=0; if(g->audio.musicVolume>1)g->audio.musicVolume=1;
                    setMusicVolume(&g->audio,g->audio.musicVolume); break;
            case 2: g->showFPS=!g->showFPS; break;
            case 3: g->fogEnabled=!g->fogEnabled; break;
        }
    }
    if (IsKeyPressed(KEY_ENTER)) { if(g->menuSelection==2) g->showFPS=!g->showFPS; if(g->menuSelection==3) g->fogEnabled=!g->fogEnabled; }
    if (IsKeyPressed(KEY_ESCAPE)) { g->state=g->previousState; g->menuSelection=0; }
}

void updateAIDemo(Game *g)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (g->demoMaze) { destroyMaze(g->demoMaze); g->demoMaze=NULL; }
        freeAIComparisonResults(g->aiResults);
        g->aiResultsComputed=false; g->state=STATE_MENU; g->menuSelection=0; return;
    }
    if (IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE)) {
        if (g->demoMaze) { destroyMaze(g->demoMaze); g->demoMaze=NULL; }
        freeAIComparisonResults(g->aiResults);
        g->demoMaze = createMaze(31,31);
        if (g->demoMaze) {
            generateMaze(g->demoMaze);
            g->demoStart=(GridNode){1,1};
            g->demoEnd=(GridNode){g->demoMaze->width-2,g->demoMaze->height-2};
            runAIComparison(g->demoMaze,g->demoStart,g->demoEnd,g->aiResults);
            g->aiResultsComputed=true; g->aiSelectedAlgo=-1;
        }
    }
    if (IsKeyPressed(KEY_ONE))   g->aiSelectedAlgo=0;
    if (IsKeyPressed(KEY_TWO))   g->aiSelectedAlgo=1;
    if (IsKeyPressed(KEY_THREE)) g->aiSelectedAlgo=2;
    if (IsKeyPressed(KEY_FOUR))  g->aiSelectedAlgo=3;
    if (IsKeyPressed(KEY_FIVE))  g->aiSelectedAlgo=4;
    if (IsKeyPressed(KEY_ZERO))  g->aiSelectedAlgo=-1;
}

void updateNameEntry(Game *g)
{
    int k = GetCharPressed();
    while (k>0) { if (k>=32&&k<=125&&g->nameLength<MAX_NAME_LENGTH-1) { g->playerName[g->nameLength++]=(char)k; g->playerName[g->nameLength]='\0'; } k=GetCharPressed(); }
    if (IsKeyPressed(KEY_BACKSPACE)&&g->nameLength>0) { g->nameLength--; g->playerName[g->nameLength]='\0'; }
    if (IsKeyPressed(KEY_ENTER)&&g->nameLength>0) {
        playGameSound(&g->audio,g->audio.sndMenuSelect);
        startNewGame(g,g->difficulty);
    }
    if (IsKeyPressed(KEY_ESCAPE)) { g->state=STATE_DIFFICULTY; g->menuSelection=(int)g->difficulty; }
}
