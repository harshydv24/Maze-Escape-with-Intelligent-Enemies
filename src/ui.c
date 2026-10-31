/*
 * ui.c — All rendering (menus, HUD, debug overlay, AI demo)
 *
 * Separated from game logic per spec requirement §41.
 */

#include "ui.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- Utility ---- */

void drawCenteredText(const char *t, int y, int fs, Color c)
{
    DrawText(t, (SCREEN_WIDTH - MeasureText(t, fs)) / 2, y, fs, c);
}

static void drawOpt(const char *t, int y, int fs, bool sel)
{
    Color c = sel ? COLOR_MENU_HIGHLIGHT : LIGHTGRAY;
    if (sel) {
        int w = MeasureText(t, fs), x = (SCREEN_WIDTH-w)/2;
        DrawRectangle(x-20, y-5, w+40, fs+10, (Color){80,250,123,30});
        DrawText(">", x-18, y, fs, COLOR_MENU_HIGHLIGHT);
    }
    drawCenteredText(t, y, fs, c);
}

/* ---- Main Menu ---- */

void drawMenu(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("MAZE ESCAPE", 80, 60, COLOR_TITLE);
    drawCenteredText("with Intelligent Enemies", 150, 20, LIGHTGRAY);
    const char *opts[] = {"New Game","Continue","High Scores","Instructions","AI Demo","Quit"};
    for (int i=0;i<6;i++) drawOpt(opts[i], 250+i*50, 28, i==g->menuSelection);
    drawCenteredText("UP/DOWN: Navigate  |  ENTER: Select", SCREEN_HEIGHT-40, 16, DARKGRAY);
    drawNotifications(g);
}

/* ---- Difficulty ---- */

void drawDifficulty(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("SELECT DIFFICULTY", 120, 40, COLOR_TITLE);
    const char *opts[] = {"Easy","Normal","Hard"};
    const char *desc[] = {
        "Small maze | Fewer enemies | Long timer | Large vision",
        "Medium maze | More enemies | Normal timer | Normal vision",
        "Large maze | Many enemies | Short timer | Small vision"
    };
    for (int i=0;i<3;i++) {
        drawOpt(opts[i], 260+i*80, 32, i==g->menuSelection);
        if (i==g->menuSelection) drawCenteredText(desc[i], 260+i*80+38, 16, GRAY);
    }
    drawCenteredText("ESC: Back", SCREEN_HEIGHT-40, 16, DARKGRAY);
}

/* ---- Helper: enemy info ---- */

static Color enemyCol(EnemyType t) { return t==ENEMY_PATROL?COLOR_ENEMY_PATROL:t==ENEMY_HUNTER?COLOR_ENEMY_BFS:COLOR_ENEMY_ASTAR; }
static const char *enemyName(EnemyType t) { return t==ENEMY_PATROL?"Patrol":t==ENEMY_HUNTER?"BFS Hunter":"A* Hunter"; }
static const char *stateName(EnemyAIState s) {
    switch(s){case AI_PATROL:return"PATROL";case AI_INVESTIGATE:return"INVESTIGATE";case AI_CHASE:return"CHASE";case AI_SEARCH:return"SEARCH";case AI_RETURN_TO_PATROL:return"RETURN";default:return"?";}
}

/* ---- Playing: maze + entities + fog ---- */

static void drawMazeGrid(Game *g)
{
    Maze *m = g->maze;
    for (int y=0; y<m->height; y++)
        for (int x=0; x<m->width; x++) {
            int idx = y*m->width+x;
            if (g->fogEnabled && !g->explored[idx]) continue;
            float cx=(float)(x*CELL_SIZE), cy=(float)(y*CELL_SIZE);
            CellType c = m->grid[y][x];

            DrawRectangle((int)cx,(int)cy,CELL_SIZE,CELL_SIZE, c==CELL_WALL?COLOR_WALL:COLOR_PATH);
            if (c==CELL_WALL) DrawRectangleLines((int)cx,(int)cy,CELL_SIZE,CELL_SIZE,(Color){55,58,75,255});

            if (c==CELL_KEY) {
                float kx=cx+CELL_SIZE/2.0f, ky=cy+CELL_SIZE/2.0f, s=CELL_SIZE*0.3f;
                DrawTriangle((Vector2){kx,ky-s},(Vector2){kx+s,ky},(Vector2){kx,ky+s},COLOR_KEY);
                DrawTriangle((Vector2){kx,ky-s},(Vector2){kx,ky+s},(Vector2){kx-s,ky},COLOR_KEY);
            }
            if (c==CELL_EXIT) {
                bool ul = g->player.keysCollected>=g->keysRequired;
                Color ec = ul?COLOR_EXIT_UNLOCKED:COLOR_EXIT_LOCKED;
                DrawRectangle((int)cx+4,(int)cy+4,CELL_SIZE-8,CELL_SIZE-8,ec);
                if (ul) { float p=(sinf((float)GetTime()*4)+1)/2; Color gw=ec; gw.a=(unsigned char)(60+(int)(p*80)); DrawRectangle((int)cx,(int)cy,CELL_SIZE,CELL_SIZE,gw); }
            }
            if (c==CELL_START) DrawRectangle((int)cx+6,(int)cy+6,CELL_SIZE-12,CELL_SIZE-12,(Color){80,250,123,60});

            if (g->fogEnabled && !g->visible[idx])
                DrawRectangle((int)cx,(int)cy,CELL_SIZE,CELL_SIZE,COLOR_EXPLORED);
        }
}

static void drawPUs(Game *g)
{
    for (int i=0; i<g->powerupCount; i++) {
        if (g->powerups[i].collected) continue;
        int idx=g->powerups[i].gridY*g->maze->width+g->powerups[i].gridX;
        if (g->fogEnabled && !g->visible[idx]) continue;
        float cx=g->powerups[i].x, cy=g->powerups[i].y;
        Color c;
        switch(g->powerups[i].type){
            case POWERUP_SPEED:c=COLOR_POWERUP_SPEED;break;
            case POWERUP_INVISIBILITY:c=COLOR_POWERUP_INVIS;break;
            case POWERUP_FREEZE:c=COLOR_POWERUP_FREEZE;break;
            default:c=COLOR_POWERUP_HEALTH;break;
        }
        float p=(sinf((float)GetTime()*3)+1)/2, r=CELL_SIZE*0.25f*(0.8f+p*0.4f);
        DrawCircle((int)cx,(int)cy,(int)r,c);
    }
}

static void drawEnemies(Game *g)
{
    for (int i=0; i<g->enemyCount; i++) {
        Enemy *e=&g->enemies[i];
        int idx=e->gridY*g->maze->width+e->gridX;
        if (g->fogEnabled && !g->visible[idx]) continue;
        Color c=enemyCol(e->type);
        DrawCircle((int)e->x,(int)e->y,(int)(ENEMY_SIZE/2),c);
        Color inner=c; inner.a=120;
        DrawCircle((int)e->x,(int)e->y,(int)(ENEMY_SIZE/4),inner);
        if (g->debugMode) {
            const char *sn=stateName(e->state);
            DrawText(sn,(int)e->x-MeasureText(sn,10)/2,(int)e->y-CELL_SIZE,10,c);
        }
    }
}

static void drawPlayerEnt(Game *g)
{
    Player *p=&g->player;
    Color pc=COLOR_PLAYER;
    if (p->invincibilityTimer>0 && sinf(p->invincibilityTimer*20)>0) pc.a=120;
    if (p->invisibilityTimer>0) pc.a=80;
    DrawCircle((int)p->x,(int)p->y,(int)(PLAYER_SIZE/2),pc);
    float tx=p->x+p->dirX*PLAYER_SIZE*0.6f, ty=p->y+p->dirY*PLAYER_SIZE*0.6f;
    DrawCircle((int)tx,(int)ty,3,WHITE);
    if (p->isSprinting) DrawCircleLines((int)p->x,(int)p->y,(int)(PLAYER_SIZE/2+3),YELLOW);
}

static void drawDebugOverlay(Game *g)
{
    if (!g->debugMode) return;
    for (int i=0; i<g->enemyCount; i++) {
        Enemy *e=&g->enemies[i]; Color c=enemyCol(e->type);
        Color rc=c; rc.a=40;
        DrawCircleLines((int)e->x,(int)e->y,(int)(e->detectionRange*CELL_SIZE),rc);
        if (e->path) {
            for (int j=e->pathIndex; j<e->pathLength; j++)
                DrawRectangle(e->path[j].x*CELL_SIZE+4,e->path[j].y*CELL_SIZE+4,CELL_SIZE-8,CELL_SIZE-8,COLOR_DEBUG_PATH);
            for (int j=e->pathIndex; j<e->pathLength-1; j++) {
                float x1=e->path[j].x*CELL_SIZE+CELL_SIZE/2.0f, y1=e->path[j].y*CELL_SIZE+CELL_SIZE/2.0f;
                float x2=e->path[j+1].x*CELL_SIZE+CELL_SIZE/2.0f, y2=e->path[j+1].y*CELL_SIZE+CELL_SIZE/2.0f;
                DrawLineEx((Vector2){x1,y1},(Vector2){x2,y2},2,c);
            }
        }
        DrawLineEx((Vector2){e->x,e->y},(Vector2){g->player.x,g->player.y},1,
            e->playerDetected?c:COLOR_DEBUG_LOS);
    }
}

void drawPlaying(Game *g)
{
    BeginMode2D(g->camera);
    drawMazeGrid(g); drawPUs(g); drawEnemies(g); drawPlayerEnt(g); drawDebugOverlay(g);
    EndMode2D();
    drawHUD(g); drawNotifications(g);
    if (g->debugMode) drawDebugInfo(g);
}

/* ---- HUD ---- */

void drawHUD(Game *g)
{
    DrawRectangle(0,0,SCREEN_WIDTH,45,COLOR_HUD_BG);
    DrawText(TextFormat("LEVEL: %d",g->currentLevel),15,12,20,WHITE);

    /* Health bar */
    float hp=(float)g->player.health/g->player.maxHealth;
    DrawRectangle(150,10,150,20,DARKGRAY);
    Color hc=hp>0.5f?COLOR_PLAYER:hp>0.25f?YELLOW:COLOR_ENEMY_BFS;
    DrawRectangle(150,10,(int)(150*hp),20,hc);
    DrawText(TextFormat("HP:%d",g->player.health),155,12,16,WHITE);

    /* Stamina */
    float st=g->player.stamina/g->player.maxStamina;
    DrawRectangle(310,10,100,20,DARKGRAY);
    DrawRectangle(310,10,(int)(100*st),20,(Color){139,233,253,255});

    DrawText(TextFormat("KEYS:%d/%d",g->player.keysCollected,g->keysRequired),430,12,20,COLOR_KEY);
    DrawText(TextFormat("SCORE:%d",g->totalScore+g->score),600,12,20,WHITE);

    int min=(int)g->timer/60, sec=(int)g->timer%60;
    DrawText(TextFormat("TIME:%02d:%02d",min,sec),800,12,20,g->timer<30?COLOR_ENEMY_BFS:WHITE);

    int px=960;
    if (g->player.speedBoostTimer>0)   { DrawText(TextFormat("SPD %.0f",g->player.speedBoostTimer),  px,12,16,COLOR_POWERUP_SPEED); px+=70; }
    if (g->player.invisibilityTimer>0) { DrawText(TextFormat("INV %.0f",g->player.invisibilityTimer),px,12,16,COLOR_POWERUP_INVIS); px+=70; }
    if (g->player.freezeTimer>0)         DrawText(TextFormat("FRZ %.0f",g->player.freezeTimer),      px,12,16,COLOR_POWERUP_FREEZE);
}

void drawNotifications(Game *g)
{
    int y=60;
    for (int i=0; i<g->notificationCount; i++) {
        float a=g->notifications[i].timer/NOTIFICATION_DURATION;
        Color c=g->notifications[i].color; c.a=(unsigned char)(255*a);
        drawCenteredText(g->notifications[i].text,y,24,c);
        y+=30;
    }
}

void drawDebugInfo(Game *g)
{
    int pw=280,ph=30+g->enemyCount*36,px=SCREEN_WIDTH-pw-10,py=SCREEN_HEIGHT-ph-10;
    DrawRectangle(px,py,pw,ph,(Color){20,20,30,220});
    DrawRectangleLines(px,py,pw,ph,COLOR_TITLE);
    int tx=px+10,ty=py+8;
    DrawText("DEBUG (F1)",tx,ty,14,COLOR_TITLE); ty+=20;
    for (int i=0; i<g->enemyCount; i++) {
        Enemy *e=&g->enemies[i]; Color c=enemyCol(e->type);
        DrawText(TextFormat("E%d [%s] %s",i+1,enemyName(e->type),stateName(e->state)),tx,ty,12,c); ty+=16;
        DrawText(TextFormat("  Pos(%d,%d) Explored:%d Path:%d",e->gridX,e->gridY,e->nodesExplored,e->pathLength-e->pathIndex),tx,ty,11,GRAY); ty+=18;
    }
}

/* ---- Pause ---- */

void drawPaused(Game *g)
{
    drawPlaying(g);
    DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,(Color){0,0,0,180});
    drawCenteredText("PAUSED",150,50,COLOR_TITLE);
    const char *opts[]={"Resume","Restart Level","Settings","Save Game","Main Menu"};
    for (int i=0;i<5;i++) drawOpt(opts[i],280+i*50,28,i==g->menuSelection);
}

/* ---- Game Over ---- */

void drawGameOver(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("GAME OVER",150,60,COLOR_ENEMY_BFS);
    drawCenteredText(TextFormat("Level Reached: %d",g->currentLevel),280,24,LIGHTGRAY);
    drawCenteredText(TextFormat("Final Score: %d",g->totalScore),320,28,COLOR_KEY);
    drawCenteredText("Press ENTER to continue",SCREEN_HEIGHT-80,20,DARKGRAY);
}

/* ---- Level Complete ---- */

void drawLevelComplete(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("LEVEL COMPLETE!",150,50,COLOR_PLAYER);
    drawCenteredText(TextFormat("Level %d cleared",g->currentLevel),220,24,LIGHTGRAY);
    drawCenteredText(TextFormat("Level Score: %d",g->score),270,28,COLOR_KEY);
    drawCenteredText(TextFormat("Total Score: %d",g->totalScore),310,24,WHITE);
    drawCenteredText("ENTER: Next Level  |  ESC: Menu",SCREEN_HEIGHT-60,18,DARKGRAY);
}

/* ---- High Scores ---- */

void drawHighScores(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("HIGH SCORES",80,40,COLOR_TITLE);
    if (g->highScoreCount==0) { drawCenteredText("No scores yet!",250,20,GRAY); }
    else {
        int y=160;
        DrawText("RANK",350,y,18,GRAY); DrawText("NAME",430,y,18,GRAY); DrawText("SCORE",700,y,18,GRAY);
        y+=30;
        for (int i=0;i<g->highScoreCount;i++) {
            Color rc=i==0?COLOR_KEY:i==1?LIGHTGRAY:i==2?(Color){205,127,50,255}:GRAY;
            DrawText(TextFormat("%d.",i+1),360,y,20,rc);
            DrawText(g->highScores[i].name,430,y,20,rc);
            DrawText(TextFormat("%d",g->highScores[i].score),700,y,20,rc);
            y+=35;
        }
    }
    drawCenteredText("ESC: Back",SCREEN_HEIGHT-40,16,DARKGRAY);
}

/* ---- Instructions ---- */

void drawInstructions(Game *g)
{
    (void)g;
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("INSTRUCTIONS",60,40,COLOR_TITLE);
    int y=130,x=200,fs=18; Color c=LIGHTGRAY;
    DrawText("OBJECTIVE:",x,y,fs+2,COLOR_KEY); y+=30;
    DrawText("Find the key(s), reach the exit, escape!",x,y,fs,c); y+=40;
    DrawText("CONTROLS:",x,y,fs+2,COLOR_KEY); y+=28;
    DrawText("W/A/S/D or Arrows  -  Move",x,y,fs,c); y+=24;
    DrawText("Left Shift         -  Sprint",x,y,fs,c); y+=24;
    DrawText("ESC                -  Pause",x,y,fs,c); y+=24;
    DrawText("F1                 -  Debug Mode",x,y,fs,c); y+=24;
    DrawText("F2                 -  Toggle Fog",x,y,fs,c); y+=36;
    DrawText("ENEMIES:",x,y,fs+2,COLOR_ENEMY_BFS); y+=28;
    DrawText("Blue   = Patrol Guard",x,y,fs,COLOR_ENEMY_PATROL); y+=24;
    DrawText("Red    = BFS Hunter (shortest path)",x,y,fs,COLOR_ENEMY_BFS); y+=24;
    DrawText("Purple = A* Hunter (smart pathfinding)",x,y,fs,COLOR_ENEMY_ASTAR); y+=36;
    DrawText("POWER-UPS:",x,y,fs+2,COLOR_POWERUP_SPEED); y+=28;
    DrawText("Orange = Speed Boost",x,y,fs,COLOR_POWERUP_SPEED); y+=24;
    DrawText("Cyan   = Invisibility",x,y,fs,COLOR_POWERUP_INVIS); y+=24;
    DrawText("Blue   = Freeze Enemies",x,y,fs,COLOR_POWERUP_FREEZE); y+=24;
    DrawText("Pink   = Health Restore",x,y,fs,COLOR_POWERUP_HEALTH);
    drawCenteredText("ESC: Back",SCREEN_HEIGHT-40,16,DARKGRAY);
}

/* ---- Settings ---- */

void drawSettings(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("SETTINGS",120,40,COLOR_TITLE);
    const char *lbl[]={"Sound Volume","Music Volume","Show FPS","Fog of War"};
    float vals[]={g->audio.soundVolume,g->audio.musicVolume,g->showFPS?1:0,g->fogEnabled?1:0};
    for (int i=0;i<4;i++){
        bool sel=i==g->menuSelection; Color c=sel?COLOR_MENU_HIGHLIGHT:LIGHTGRAY;
        int y=260+i*60;
        if (sel) DrawRectangle(300,y-5,680,40,(Color){80,250,123,20});
        DrawText(lbl[i],320,y,24,c);
        if (i<2) {
            DrawRectangle(600,y+4,300,20,DARKGRAY);
            DrawRectangle(600,y+4,(int)(300*vals[i]),20,c);
            DrawText(TextFormat("%.0f%%",vals[i]*100),910,y,20,c);
            if (sel) DrawText("<  >",560,y,20,COLOR_MENU_HIGHLIGHT);
        } else {
            DrawText(vals[i]>0.5f?"ON":"OFF",700,y,24,vals[i]>0.5f?COLOR_PLAYER:GRAY);
        }
    }
    drawCenteredText("LEFT/RIGHT: Adjust | ENTER: Toggle | ESC: Back",SCREEN_HEIGHT-40,16,DARKGRAY);
}

/* ---- AI Demo ---- */

void drawAIDemo(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("AI COMPARISON MODE",20,30,COLOR_TITLE);
    drawCenteredText("ENTER: Generate | 1-5: Select Algo | 0: All | ESC: Back",55,14,DARKGRAY);

    if (!g->aiResultsComputed || !g->demoMaze) {
        drawCenteredText("Press ENTER to generate a maze and compare algorithms",SCREEN_HEIGHT/2,20,GRAY);
        return;
    }

    Maze *m=g->demoMaze;
    int cs=6, ox=30, oy=90;
    for (int y=0;y<m->height;y++)
        for (int x=0;x<m->width;x++)
            DrawRectangle(ox+x*cs,oy+y*cs,cs,cs,m->grid[y][x]==CELL_WALL?COLOR_WALL:COLOR_PATH);

    /* Draw selected algorithm path(s) */
    Color algoCols[]={ORANGE,YELLOW,MAGENTA,COLOR_ENEMY_BFS,COLOR_ENEMY_ASTAR};
    for (int a=0;a<5;a++) {
        if (g->aiSelectedAlgo>=0 && g->aiSelectedAlgo!=a) continue;
        if (!g->aiResults[a].found || !g->aiResults[a].path) continue;
        Color pc=algoCols[a]; pc.a=g->aiSelectedAlgo>=0?200:100;
        for (int j=0;j<g->aiResults[a].pathCount;j++) {
            int px=ox+g->aiResults[a].path[j].x*cs;
            int py=oy+g->aiResults[a].path[j].y*cs;
            DrawRectangle(px,py,cs,cs,pc);
        }
    }

    /* Start/end markers */
    DrawRectangle(ox+g->demoStart.x*cs,oy+g->demoStart.y*cs,cs,cs,COLOR_PLAYER);
    DrawRectangle(ox+g->demoEnd.x*cs,oy+g->demoEnd.y*cs,cs,cs,COLOR_EXIT_UNLOCKED);

    /* Results table */
    int tx=280,ty=100;
    DrawText("Algorithm",tx,ty,20,COLOR_TITLE);
    DrawText("Path Length",tx+160,ty,20,COLOR_TITLE);
    DrawText("Nodes Explored",tx+340,ty,20,COLOR_TITLE);
    ty+=35;
    DrawLine(tx,ty-5,tx+520,ty-5,GRAY);
    for (int i=0;i<5;i++) {
        Color rc=algoCols[i];
        bool sel=g->aiSelectedAlgo==i;
        if (sel) DrawRectangle(tx-5,ty-3,530,26,(Color){80,250,123,30});
        DrawText(TextFormat("%d. %s",i+1,g->aiResults[i].name),tx,ty,18,rc);
        if (g->aiResults[i].found)
            DrawText(TextFormat("%d",g->aiResults[i].pathLength),tx+200,ty,18,rc);
        else DrawText("N/A",tx+200,ty,18,GRAY);
        DrawText(TextFormat("%d",g->aiResults[i].nodesExplored),tx+380,ty,18,rc);
        ty+=30;
    }
    ty+=20;
    DrawText("BFS and A* find the shortest path.",tx,ty,16,GRAY); ty+=22;
    DrawText("A* typically explores fewer nodes than BFS.",tx,ty,16,GRAY); ty+=22;
    DrawText("DFS may find a longer path.",tx,ty,16,GRAY); ty+=22;
    DrawText("Random walk is unpredictable.",tx,ty,16,GRAY);
}

/* ---- Name Entry ---- */

void drawNameEntry(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    drawCenteredText("ENTER YOUR NAME",150,40,COLOR_TITLE);
    drawCenteredText(TextFormat("Score: %d",g->totalScore),220,28,COLOR_KEY);

    int bw=400,bh=50,bx=(SCREEN_WIDTH-bw)/2,by=300;
    DrawRectangle(bx,by,bw,bh,(Color){40,42,54,255});
    DrawRectangleLines(bx,by,bw,bh,COLOR_MENU_HIGHLIGHT);
    DrawText(g->playerName,bx+15,by+12,28,WHITE);

    /* Blinking cursor */
    if ((int)(GetTime()*2)%2==0) {
        int cw=MeasureText(g->playerName,28);
        DrawRectangle(bx+15+cw+2,by+10,2,30,COLOR_MENU_HIGHLIGHT);
    }

    drawCenteredText("Press ENTER to submit",400,18,DARKGRAY);
}
