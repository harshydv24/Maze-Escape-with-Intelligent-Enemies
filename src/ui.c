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

static Font uiFont;
static bool uiFontLoaded;

void initUI(void)
{
    uiFontLoaded=false;
    if (FileExists("assets/fonts/game.ttf")) {
        uiFont=LoadFontEx("assets/fonts/game.ttf",64,NULL,0);
        uiFontLoaded=uiFont.texture.id!=0;
    }
}

void cleanupUI(void)
{
    if (uiFontLoaded) UnloadFont(uiFont);
    uiFontLoaded=false;
}

static void uiDrawText(const char *text, int x, int y, int fontSize, Color color)
{
    if (uiFontLoaded)
        DrawTextEx(uiFont,text,(Vector2){(float)x,(float)y},(float)fontSize,1.0f,color);
    else
        DrawText(text,x,y,fontSize,color);
}

static int uiMeasureText(const char *text, int fontSize)
{
    if (uiFontLoaded)
        return (int)MeasureTextEx(uiFont,text,(float)fontSize,1.0f).x;
    return MeasureText(text,fontSize);
}

static int drawHintAt(const char *key, const char *action, int x, int y, int fontSize)
{
    const int padX=8, padY=4, gap=9;
    int keyWidth=uiMeasureText(key,fontSize)+padX*2;
    int boxHeight=fontSize+padY*2;
    Rectangle outer={(float)x,(float)y,(float)keyWidth,(float)boxHeight};
    Rectangle inner={(float)(x+1),(float)(y+1),(float)(keyWidth-2),(float)(boxHeight-2)};
    DrawRectangleRounded(outer,0.28f,6,(Color){105,112,135,255});
    DrawRectangleRounded(inner,0.28f,6,(Color){39,42,55,255});
    uiDrawText(key,x+padX,y+padY,fontSize,(Color){235,238,248,255});
    uiDrawText(action,x+keyWidth+gap,y+padY,fontSize,(Color){185,189,202,255});
    return keyWidth+gap+uiMeasureText(action,fontSize);
}

static void drawHintRow(const char *const keys[], const char *const actions[],
                        int count, int y, int fontSize)
{
    const int itemGap=22;
    int totalWidth=0;
    for (int i=0;i<count;i++)
        totalWidth+=uiMeasureText(keys[i],fontSize)+16+9+uiMeasureText(actions[i],fontSize);
    if (count>1) totalWidth+=(count-1)*itemGap;
    int x=(GetScreenWidth()-totalWidth)/2;
    for (int i=0;i<count;i++)
        x+=drawHintAt(keys[i],actions[i],x,y,fontSize)+itemGap;
}

static int centeredLayoutOffsetY(void)
{
    return (GetScreenHeight()-UI_LAYOUT_HEIGHT)/2;
}

/* ---- Utility ---- */

void drawCenteredText(const char *t, int y, int fs, Color c)
{
    uiDrawText(t, (GetScreenWidth() - uiMeasureText(t, fs)) / 2, y, fs, c);
}

static bool textureAvailable(Texture2D texture)
{
    return texture.id != 0;
}

static void drawSprite(Texture2D texture, float x, float y, float size, Color tint)
{
    Rectangle source={0,0,(float)texture.width,(float)texture.height};
    Rectangle dest={x-size/2.0f,y-size/2.0f,size,size};
    DrawTexturePro(texture,source,dest,(Vector2){0,0},0,tint);
}

static void drawOpt(const char *t, int y, int fs, bool sel)
{
    const int buttonWidth=420;
    int x=(GetScreenWidth()-buttonWidth)/2;
    int buttonHeight=fs+12;
    Rectangle button={(float)x,(float)(y-5),(float)buttonWidth,(float)buttonHeight};
    DrawRectangleRounded(button,0.18f,8,sel?(Color){48,76,61,255}:(Color){40,42,54,235});
    if (sel) DrawRectangle(x,y-5,5,buttonHeight,COLOR_MENU_HIGHLIGHT);
    int textX=x+(buttonWidth-uiMeasureText(t,fs))/2;
    uiDrawText(t,textX,y,fs,sel?COLOR_MENU_HIGHLIGHT:LIGHTGRAY);
}

/* ---- Main Menu ---- */

void drawMenu(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    const int pitch=46, buttonHeight=40;
    int optionsHeight=7*buttonHeight+6*(pitch-buttonHeight);
    int optionsTop=(GetScreenHeight()-optionsHeight)/2+5;
    drawCenteredText("MAZE ESCAPE",80+oy,60,COLOR_TITLE);
    drawCenteredText("with Intelligent Enemies",150+oy,20,LIGHTGRAY);
    const char *opts[] = {"New Game","Continue","High Scores","Instructions","Settings","AI Demo","Quit"};
    for (int i=0;i<7;i++) drawOpt(opts[i], optionsTop+i*pitch, 28, i==g->menuSelection);
    const char *keys[]={"UP/DOWN","ENTER"};
    const char *actions[]={"Navigate","Select"};
    drawHintRow(keys,actions,2,GetScreenHeight()-38,16);
    drawNotifications(g);
}

/* ---- Difficulty ---- */

void drawDifficulty(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    int optionsHeight=3*80;
    int optionsTop=(GetScreenHeight()-optionsHeight)/2+5;
    drawCenteredText("SELECT DIFFICULTY",120+oy,40,COLOR_TITLE);
    const char *opts[] = {"Easy","Normal","Hard"};
    const char *desc[] = {
        "Small maze | Fewer enemies | Long timer | Large vision",
        "Medium maze | More enemies | Normal timer | Normal vision",
        "Large maze | Many enemies | Short timer | Small vision"
    };
    for (int i=0;i<3;i++) {
        drawOpt(opts[i], optionsTop+i*80, 32, i==g->menuSelection);
        if (i==g->menuSelection) drawCenteredText(desc[i], optionsTop+i*80+42, 16, GRAY);
    }
    const char *keys[]={"ESC"}; const char *actions[]={"Back"};
    drawHintRow(keys,actions,1,GetScreenHeight()-38,16);
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
                int kx=(int)(cx+CELL_SIZE/2.0f), ky=(int)(cy+CELL_SIZE/2.0f);
                if (textureAvailable(g->textures.key))
                    drawSprite(g->textures.key,(float)kx,(float)ky,CELL_SIZE*1.0f,WHITE);
                else {
                    /* Gold key fallback while no key.png has been supplied. */
                    DrawCircle(kx-5,ky,5,COLOR_KEY);
                    DrawCircle(kx-5,ky,2,COLOR_PATH);
                    DrawRectangle(kx-1,ky-2,11,4,COLOR_KEY);
                    DrawRectangle(kx+5,ky,3,4,COLOR_KEY);
                    DrawRectangle(kx+9,ky,3,3,COLOR_KEY);
                }
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

static Vector2 powerupPoint(int cx, int cy, float dx, float dy, float scale)
{
    return (Vector2){cx + dx*scale, cy + dy*scale};
}

static void drawPUs(Game *g)
{
    for (int i=0; i<g->powerupCount; i++) {
        if (g->powerups[i].collected) continue;
        int idx=g->powerups[i].gridY*g->maze->width+g->powerups[i].gridX;
        if (g->fogEnabled && !g->visible[idx]) continue;
        int cx=(int)g->powerups[i].x, cy=(int)g->powerups[i].y;
        Color c;
        switch(g->powerups[i].type){
            case POWERUP_SPEED:c=COLOR_POWERUP_SPEED;break;
            case POWERUP_INVISIBILITY:c=COLOR_POWERUP_INVIS;break;
            case POWERUP_FREEZE:c=COLOR_POWERUP_FREEZE;break;
            default:c=COLOR_POWERUP_HEALTH;break;
        }
        Texture2D sprite={0};
        switch(g->powerups[i].type) {
            case POWERUP_SPEED: sprite=g->textures.powerupSpeed; break;
            case POWERUP_INVISIBILITY: sprite=g->textures.powerupInvisibility; break;
            case POWERUP_FREEZE: sprite=g->textures.powerupFreeze; break;
            case POWERUP_HEALTH: sprite=g->textures.powerupHealth; break;
            default: break;
        }
        if (textureAvailable(sprite)) {
            float pulse=1.0f+0.06f*sinf((float)GetTime()*7.0f+i*0.6f);
            drawSprite(sprite,(float)cx,(float)cy,CELL_SIZE*0.9f*pulse,WHITE);
            continue;
        }
        float scale=1.4f+0.15f*sinf((float)GetTime()*7.0f+i*0.6f);
        /* Flat silhouettes pulse gently while staying clear of adjacent cells. */
        switch(g->powerups[i].type) {
            case POWERUP_SPEED:
                DrawTriangle(powerupPoint(cx,cy,0,-8,scale),powerupPoint(cx,cy,7,-1,scale),powerupPoint(cx,cy,2,-1,scale),c);
                DrawTriangle(powerupPoint(cx,cy,0,-8,scale),powerupPoint(cx,cy,2,-1,scale),powerupPoint(cx,cy,4,0,scale),c);
                DrawTriangle(powerupPoint(cx,cy,0,-8,scale),powerupPoint(cx,cy,4,0,scale),powerupPoint(cx,cy,-4,8,scale),c);
                DrawTriangle(powerupPoint(cx,cy,0,-8,scale),powerupPoint(cx,cy,-4,8,scale),powerupPoint(cx,cy,-2,1,scale),c);
                DrawTriangle(powerupPoint(cx,cy,0,-8,scale),powerupPoint(cx,cy,-2,1,scale),powerupPoint(cx,cy,-7,1,scale),c);
                break;
            case POWERUP_INVISIBILITY:
                DrawTriangle(powerupPoint(cx,cy,-7,0,scale),powerupPoint(cx,cy,0,-5,scale),powerupPoint(cx,cy,7,0,scale),c);
                DrawTriangle(powerupPoint(cx,cy,-7,0,scale),powerupPoint(cx,cy,0,5,scale),powerupPoint(cx,cy,7,0,scale),c);
                DrawCircleV((Vector2){(float)cx,(float)cy},2*scale,COLOR_PATH);
                break;
            case POWERUP_FREEZE:
                DrawLineEx(powerupPoint(cx,cy,-6,0,scale),powerupPoint(cx,cy,6,0,scale),2.2f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-3,-5,scale),powerupPoint(cx,cy,3,5,scale),2.2f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-3,5,scale),powerupPoint(cx,cy,3,-5,scale),2.2f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,6,0,scale),powerupPoint(cx,cy,4,-2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,6,0,scale),powerupPoint(cx,cy,4,2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-6,0,scale),powerupPoint(cx,cy,-4,-2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-6,0,scale),powerupPoint(cx,cy,-4,2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,3,5,scale),powerupPoint(cx,cy,1,2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,3,5,scale),powerupPoint(cx,cy,4,2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-3,-5,scale),powerupPoint(cx,cy,-1,-2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-3,-5,scale),powerupPoint(cx,cy,-4,-2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-3,5,scale),powerupPoint(cx,cy,-1,2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,-3,5,scale),powerupPoint(cx,cy,-4,2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,3,-5,scale),powerupPoint(cx,cy,1,-2,scale),1.7f*scale,c);
                DrawLineEx(powerupPoint(cx,cy,3,-5,scale),powerupPoint(cx,cy,4,-2,scale),1.7f*scale,c);
                break;
            case POWERUP_HEALTH:
                DrawRectangle((int)(cx-2*scale),(int)(cy-6*scale),(int)(4*scale),(int)(12*scale),c);
                DrawRectangle((int)(cx-6*scale),(int)(cy-2*scale),(int)(12*scale),(int)(4*scale),c);
                break;
            default:
                break;
        }
    }
}

static void drawEnemies(Game *g)
{
    for (int i=0; i<g->enemyCount; i++) {
        Enemy *e=&g->enemies[i];
        int idx=e->gridY*g->maze->width+e->gridX;
        if (g->fogEnabled && !g->visible[idx]) continue;
        Color c=enemyCol(e->type);
        Texture2D sprite=e->type==ENEMY_PATROL?g->textures.enemyPatrol:
                         e->type==ENEMY_HUNTER?g->textures.enemyBfs:g->textures.enemyAstar;
        if (textureAvailable(sprite)) drawSprite(sprite,e->x,e->y,CELL_SIZE*1.05f,WHITE);
        else {
            DrawCircle((int)e->x,(int)e->y,(int)(ENEMY_SIZE/2),c);
            Color inner=c; inner.a=120;
            DrawCircle((int)e->x,(int)e->y,(int)(ENEMY_SIZE/4),inner);
        }
        if (e->alertTimer > 0 || e->state == AI_CHASE) {
            int ay=(int)e->y-CELL_SIZE/2;
            DrawCircle((int)e->x,ay,9,(Color){35,35,45,235});
            uiDrawText("!",(int)e->x-3,ay-9,18,(Color){255,85,85,255});
        }
        if (g->debugMode) {
            const char *sn=stateName(e->state);
            uiDrawText(sn,(int)e->x-uiMeasureText(sn,10)/2,(int)e->y-CELL_SIZE,10,c);
        }
    }
}

static void drawPlayerEnt(Game *g)
{
    Player *p=&g->player;
    DrawEllipse((int)p->x,(int)(p->y+PLAYER_SIZE*0.32f),
                PLAYER_SIZE*0.34f,PLAYER_SIZE*0.15f,(Color){0,0,0,90});
    Color pc=COLOR_PLAYER;
    if (p->invincibilityTimer>0 && sinf(p->invincibilityTimer*20)>0) pc.a=120;
    if (p->invisibilityTimer>0) pc.a=80;
    if (p->isMoving) {
        const int segments=16;
        float spriteSize=CELL_SIZE*1.05f;
        float length=p->isSprinting?38.0f:28.0f;
        Vector2 head={p->x,p->y};
        Vector2 tail={head.x-p->dirX*length,head.y-p->dirY*length};
        for (int i=0;i<segments;i++) {
            float t0=(float)i/segments, t1=(float)(i+1)/segments;
            Vector2 a={tail.x+p->dirX*(length*t0),tail.y+p->dirY*(length*t0)};
            Vector2 b={tail.x+p->dirX*(length*t1),tail.y+p->dirY*(length*t1)};
            Color trail=YELLOW;
            trail.a=(unsigned char)((12+125*t1)*pc.a/255.0f);
            DrawLineEx(a,b,spriteSize*0.55f,trail);
        }
    }
    if (textureAvailable(g->textures.player)) {
        /* White RGB tint preserves the source PNG colours; alpha still blinks/fades. */
        Color spriteTint=WHITE;
        spriteTint.a=pc.a;
        drawSprite(g->textures.player,p->x,p->y,CELL_SIZE*1.05f,spriteTint);
    }
    else {
        DrawCircle((int)p->x,(int)p->y,(int)(PLAYER_SIZE/2),pc);
        DrawCircleLines((int)p->x,(int)p->y,(int)(PLAYER_SIZE*0.28f),BLACK);
        DrawCircle((int)(p->x+p->dirX*5),(int)(p->y+p->dirY*5),2,BLACK);
    }
    if (p->isSprinting) DrawCircleLines((int)p->x,(int)p->y,(int)(PLAYER_SIZE/2+3),YELLOW);
}

static void drawBonusAndDecoy(Game *g)
{
    if (g->bonusAvailable) {
        int idx=g->bonusPosition.y*g->maze->width+g->bonusPosition.x;
        if (!g->fogEnabled || g->visible[idx]) {
            float pulse=1.0f+0.12f*sinf(g->worldTime*5.0f);
            Vector2 c={(float)(g->bonusPosition.x*CELL_SIZE+CELL_SIZE/2),
                       (float)(g->bonusPosition.y*CELL_SIZE+CELL_SIZE/2)};
            DrawCircleV(c,11*pulse,(Color){255,184,108,65});
            DrawPoly(c,4,8*pulse,45,COLOR_KEY);
        }
    }
    if (g->decoyVisibleTimer>0) {
        Vector2 c={(float)(g->decoyPosition.x*CELL_SIZE+CELL_SIZE/2),
                   (float)(g->decoyPosition.y*CELL_SIZE+CELL_SIZE/2)};
        float phase=fmodf(g->worldTime,0.8f)/0.8f;
        DrawCircleLines((int)c.x,(int)c.y,8+(int)(phase*12),COLOR_POWERUP_INVIS);
        DrawCircleV(c,4,COLOR_POWERUP_INVIS);
    }
    if (g->pickupEffectTimer>0) {
        float progress=1.0f-g->pickupEffectTimer/0.45f;
        Color ring=g->pickupEffectColor; ring.a=(unsigned char)(200*(1-progress));
        DrawCircleLines((int)g->pickupEffectPosition.x,(int)g->pickupEffectPosition.y,
                        5+progress*25,ring);
    }
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
    drawMazeGrid(g); drawBonusAndDecoy(g); drawPUs(g); drawEnemies(g); drawPlayerEnt(g); drawDebugOverlay(g);
    EndMode2D();
    drawHUD(g); drawNotifications(g);
    if (g->debugMode) drawDebugInfo(g);
    if (g->screenFlashTimer>0) {
        Color flash=g->screenFlashColor;
        float fade=g->screenFlashTimer/0.28f;
        flash.a=(unsigned char)(flash.a*(fade>1?1:fade));
        DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),flash);
    }
}

/* ---- HUD ---- */

void drawHUD(Game *g)
{
    int ox=(GetScreenWidth()-UI_LAYOUT_WIDTH)/2;
    DrawRectangle(0,0,GetScreenWidth(),45,COLOR_HUD_BG);
    uiDrawText(TextFormat("LEVEL: %d",g->currentLevel),15+ox,12,20,WHITE);

    /* Health bar */
    float hp=(float)g->player.health/g->player.maxHealth;
    DrawRectangle(150+ox,10,150,20,DARKGRAY);
    Color hc=hp>0.5f?COLOR_PLAYER:hp>0.25f?YELLOW:COLOR_ENEMY_BFS;
    DrawRectangle(150+ox,10,(int)(150*hp),20,hc);
    uiDrawText(TextFormat("HP:%d",g->player.health),155+ox,12,16,WHITE);

    /* Stamina */
    float st=g->player.stamina/g->player.maxStamina;
    DrawRectangle(310+ox,10,100,20,DARKGRAY);
    DrawRectangle(310+ox,10,(int)(100*st),20,(Color){139,233,253,255});

    uiDrawText(TextFormat("KEYS:%d/%d",g->player.keysCollected,g->keysRequired),430+ox,12,20,COLOR_KEY);
    uiDrawText(TextFormat("SCORE:%d",g->totalScore+g->score),600+ox,12,20,WHITE);

    int min=(int)g->timer/60, sec=(int)g->timer%60;
    uiDrawText(TextFormat("TIME:%02d:%02d",min,sec),800+ox,12,20,g->timer<30?COLOR_ENEMY_BFS:WHITE);

    drawHintAt("Q",TextFormat("Decoy: %d",g->decoysAvailable),960+ox,8,14);
    int px=1080+ox;
    if (g->player.speedBoostTimer>0)   { uiDrawText(TextFormat("SPD %.0f",g->player.speedBoostTimer),  px,12,16,COLOR_POWERUP_SPEED); px+=70; }
    if (g->player.invisibilityTimer>0) { uiDrawText(TextFormat("INV %.0f",g->player.invisibilityTimer),px,12,16,COLOR_POWERUP_INVIS); px+=70; }
    if (g->player.freezeTimer>0)         uiDrawText(TextFormat("FRZ %.0f",g->player.freezeTimer),      px,12,16,COLOR_POWERUP_FREEZE);
}

void drawNotifications(Game *g)
{
    const int rowHeight=36, rowGap=6, leftPadding=24;
    int totalHeight=g->notificationCount>0?
        g->notificationCount*rowHeight+(g->notificationCount-1)*rowGap:0;
    int y=(GetScreenHeight()-totalHeight)/2;
    for (int i=0; i<g->notificationCount; i++) {
        float a=g->notifications[i].timer>=NOTIFICATION_FADE_DURATION?1.0f:
                g->notifications[i].timer/NOTIFICATION_FADE_DURATION;
        if (a<0) a=0;
        int w=uiMeasureText(g->notifications[i].text,22)+42;
        int x=leftPadding;
        Color bg=(Color){18,20,28,(unsigned char)(210*a)};
        Color c=g->notifications[i].color; c.a=(unsigned char)(255*a);
        DrawRectangleRounded((Rectangle){(float)x,(float)y,(float)w,36},0.35f,8,bg);
        DrawRectangle(x,y,4,36,c);
        uiDrawText(g->notifications[i].text,x+18,y+7,22,c);
        y+=rowHeight+rowGap;
    }
}

void drawDebugInfo(Game *g)
{
    int pw=280,ph=30+g->enemyCount*36,px=GetScreenWidth()-pw-10,py=GetScreenHeight()-ph-10;
    DrawRectangle(px,py,pw,ph,(Color){20,20,30,220});
    DrawRectangleLines(px,py,pw,ph,COLOR_TITLE);
    int tx=px+10,ty=py+8;
    uiDrawText("DEBUG (F1)",tx,ty,14,COLOR_TITLE); ty+=20;
    for (int i=0; i<g->enemyCount; i++) {
        Enemy *e=&g->enemies[i]; Color c=enemyCol(e->type);
        uiDrawText(TextFormat("E%d [%s] %s",i+1,enemyName(e->type),stateName(e->state)),tx,ty,12,c); ty+=16;
        uiDrawText(TextFormat("  Pos(%d,%d) Explored:%d Path:%d",e->gridX,e->gridY,e->nodesExplored,e->pathLength-e->pathIndex),tx,ty,11,GRAY); ty+=18;
    }
}

/* ---- Pause ---- */

void drawPaused(Game *g)
{
    drawPlaying(g);
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),(Color){0,0,0,180});
    int oy=centeredLayoutOffsetY();
    const int buttonHeight=40, rowGap=10;
    int optionsHeight=5*buttonHeight+4*rowGap;
    int optionsTop=(GetScreenHeight()-optionsHeight)/2+5;
    drawCenteredText("PAUSED",150+oy,50,COLOR_TITLE);
    const char *opts[]={"Resume","Restart Level","Settings","Save Game","Main Menu"};
    for (int i=0;i<5;i++) drawOpt(opts[i],optionsTop+i*(buttonHeight+rowGap)+5,28,i==g->menuSelection);
    const char *keys[]={"UP/DOWN","ENTER","ESC"};
    const char *actions[]={"Navigate","Select","Resume"};
    drawHintRow(keys,actions,3,GetScreenHeight()-38,15);
}

/* ---- Game Over ---- */

void drawGameOver(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    drawCenteredText("GAME OVER",150+oy,60,COLOR_ENEMY_BFS);
    drawCenteredText(TextFormat("Level Reached: %d",g->currentLevel),280+oy,24,LIGHTGRAY);
    drawCenteredText(TextFormat("Final Score: %d",g->totalScore),320+oy,28,COLOR_KEY);
    const char *keys[]={"ENTER","ESC"}; const char *actions[]={"Continue","Continue"};
    drawHintRow(keys,actions,2,GetScreenHeight()-76,16);
}

/* ---- Level Complete ---- */

void drawLevelComplete(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    drawCenteredText("LEVEL COMPLETE!",150+oy,50,COLOR_PLAYER);
    drawCenteredText(TextFormat("Level %d cleared",g->currentLevel),220+oy,24,LIGHTGRAY);
    drawCenteredText(TextFormat("Level Score: %d",g->score),270+oy,28,COLOR_KEY);
    drawCenteredText(TextFormat("Total Score: %d",g->totalScore),310+oy,24,WHITE);
    const char *keys[]={"ENTER","ESC"};
    const char *actions[]={"Next Level","Menu"};
    drawHintRow(keys,actions,2,GetScreenHeight()-44,16);
}

/* ---- High Scores ---- */

void drawHighScores(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    drawCenteredText("HIGH SCORES",80+oy,40,COLOR_TITLE);
    if (g->highScoreCount==0) { drawCenteredText("No scores yet!",250+oy,20,GRAY); }
    else {
        int tableWidth=560;
        int tableX=(GetScreenWidth()-tableWidth)/2;
        int y=160+oy;
        uiDrawText("RANK",tableX,y,18,GRAY);
        uiDrawText("NAME",tableX+100,y,18,GRAY);
        uiDrawText("SCORE",tableX+440,y,18,GRAY);
        y+=30;
        for (int i=0;i<g->highScoreCount;i++) {
            Color rc=i==0?COLOR_KEY:i==1?LIGHTGRAY:i==2?(Color){205,127,50,255}:GRAY;
            uiDrawText(TextFormat("%d.",i+1),tableX+10,y,20,rc);
            uiDrawText(g->highScores[i].name,tableX+100,y,20,rc);
            uiDrawText(TextFormat("%d",g->highScores[i].score),tableX+440,y,20,rc);
            y+=35;
        }
    }
    const char *keys[]={"ESC"}; const char *actions[]={"Back"};
    drawHintRow(keys,actions,1,GetScreenHeight()-38,16);
}

/* ---- Instructions ---- */

void drawInstructions(Game *g)
{
    (void)g;
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    int fs=18;
    int contentWidth=uiMeasureText("WASD/ARROWS",fs)+25+
                     uiMeasureText("Move; open turns apply instantly",fs);
    drawCenteredText("INSTRUCTIONS",60+oy,40,COLOR_TITLE);
    int y=130+oy,x=(GetScreenWidth()-contentWidth)/2; Color c=LIGHTGRAY;
    uiDrawText("OBJECTIVE:",x,y,fs+2,COLOR_KEY); y+=30;
    uiDrawText("Find the key(s), reach the exit, escape!",x,y,fs,c); y+=40;
    uiDrawText("CONTROLS:",x,y,fs+2,COLOR_KEY); y+=28;
    drawHintAt("WASD/ARROWS","Move; open turns apply instantly",x,y,fs); y+=28;
    drawHintAt("Q","Throw decoy (2 per level)",x,y,fs); y+=28;
    drawHintAt("CACHE","Find a dead end for +500",x,y,fs); y+=28;
    drawHintAt("ESC","Pause",x,y,fs); y+=28;
    drawHintAt("F1","Debug mode",x,y,fs); y+=28;
    drawHintAt("F2","Toggle fog",x,y,fs); y+=32;
    uiDrawText("ENEMIES:",x,y,fs+2,COLOR_ENEMY_BFS); y+=28;
    uiDrawText("Blue   = Patrol Guard",x,y,fs,COLOR_ENEMY_PATROL); y+=24;
    uiDrawText("Red    = BFS Hunter (shortest path)",x,y,fs,COLOR_ENEMY_BFS); y+=24;
    uiDrawText("Purple = A* Hunter (smart pathfinding)",x,y,fs,COLOR_ENEMY_ASTAR); y+=36;
    uiDrawText("POWER-UPS:",x,y,fs+2,COLOR_POWERUP_SPEED); y+=28;
    uiDrawText("Orange = Speed Boost",x,y,fs,COLOR_POWERUP_SPEED); y+=24;
    uiDrawText("Cyan   = Invisibility",x,y,fs,COLOR_POWERUP_INVIS); y+=24;
    uiDrawText("Blue   = Freeze Enemies",x,y,fs,COLOR_POWERUP_FREEZE); y+=24;
    uiDrawText("Pink   = Health Restore",x,y,fs,COLOR_POWERUP_HEALTH);
    const char *keys[]={"ESC"}; const char *actions[]={"Back"};
    drawHintRow(keys,actions,1,GetScreenHeight()-38,16);
}

/* ---- Settings ---- */

void drawSettings(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    int contentHeight=294;
    int top=(GetScreenHeight()-contentHeight)/2;
    const int panelWidth=680;
    int panelX=(GetScreenWidth()-panelWidth)/2;
    drawCenteredText("SETTINGS",120+oy,40,COLOR_TITLE);
    const char *lbl[]={"Sound Volume","Music Volume","Show FPS","Fog of War"};
    float vals[]={g->audio.soundVolume,g->audio.musicVolume,g->showFPS?1:0,g->fogEnabled?1:0};
    for (int i=0;i<4;i++){
        bool sel=i==g->menuSelection; Color c=sel?COLOR_MENU_HIGHLIGHT:LIGHTGRAY;
        int y=top+90+i*60;
        if (sel) DrawRectangle(panelX,y-5,panelWidth,40,(Color){80,250,123,20});
        uiDrawText(lbl[i],panelX+20,y,24,c);
        if (i<2) {
            DrawRectangle(panelX+300,y+4,300,20,DARKGRAY);
            DrawRectangle(panelX+300,y+4,(int)(300*vals[i]),20,c);
            uiDrawText(TextFormat("%.0f%%",vals[i]*100),panelX+610,y,20,c);
            if (sel) uiDrawText("<  >",panelX+260,y,20,COLOR_MENU_HIGHLIGHT);
        } else {
            uiDrawText(vals[i]>0.5f?"ON":"OFF",panelX+400,y,24,vals[i]>0.5f?COLOR_PLAYER:GRAY);
        }
    }
    const char *keys[]={"LEFT/RIGHT","ENTER","ESC"};
    const char *actions[]={"Adjust","Toggle","Back"};
    drawHintRow(keys,actions,3,GetScreenHeight()-38,15);
}

/* ---- AI Demo ---- */

void drawAIDemo(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    int ox=(GetScreenWidth()-800)/2-30;
    drawCenteredText("AI COMPARISON MODE",20+oy,30,COLOR_TITLE);
    const char *keys[]={"ENTER","1-5","0","ESC"};
    const char *actions[]={"Generate","Select","All","Back"};
    drawHintRow(keys,actions,4,54+oy,13);

    if (!g->aiResultsComputed || !g->demoMaze) {
        const char *keys[]={"ENTER"}; const char *actions[]={"Generate maze and compare algorithms"};
        drawHintRow(keys,actions,1,GetScreenHeight()/2,18);
        return;
    }

    Maze *m=g->demoMaze;
    int cs=6, mapX=30+ox, mapY=90+oy;
    for (int y=0;y<m->height;y++)
        for (int x=0;x<m->width;x++)
            DrawRectangle(mapX+x*cs,mapY+y*cs,cs,cs,m->grid[y][x]==CELL_WALL?COLOR_WALL:COLOR_PATH);

    /* Draw selected algorithm path(s) */
    Color algoCols[]={ORANGE,YELLOW,MAGENTA,COLOR_ENEMY_BFS,COLOR_ENEMY_ASTAR};
    for (int a=0;a<5;a++) {
        if (g->aiSelectedAlgo>=0 && g->aiSelectedAlgo!=a) continue;
        if (!g->aiResults[a].found || !g->aiResults[a].path) continue;
        Color pc=algoCols[a]; pc.a=g->aiSelectedAlgo>=0?200:100;
        for (int j=0;j<g->aiResults[a].pathCount;j++) {
            int px=mapX+g->aiResults[a].path[j].x*cs;
            int py=mapY+g->aiResults[a].path[j].y*cs;
            DrawRectangle(px,py,cs,cs,pc);
        }
    }

    /* Start/end markers */
    DrawRectangle(mapX+g->demoStart.x*cs,mapY+g->demoStart.y*cs,cs,cs,COLOR_PLAYER);
    DrawRectangle(mapX+g->demoEnd.x*cs,mapY+g->demoEnd.y*cs,cs,cs,COLOR_EXIT_UNLOCKED);

    /* Results table */
    int tx=280+ox,ty=100+oy;
    uiDrawText("Algorithm",tx,ty,20,COLOR_TITLE);
    uiDrawText("Path Length",tx+160,ty,20,COLOR_TITLE);
    uiDrawText("Nodes Explored",tx+340,ty,20,COLOR_TITLE);
    ty+=35;
    DrawLine(tx,ty-5,tx+520,ty-5,GRAY);
    for (int i=0;i<5;i++) {
        Color rc=algoCols[i];
        bool sel=g->aiSelectedAlgo==i;
        if (sel) DrawRectangle(tx-5,ty-3,530,26,(Color){80,250,123,30});
        uiDrawText(TextFormat("%d. %s",i+1,g->aiResults[i].name),tx,ty,18,rc);
        if (g->aiResults[i].found)
            uiDrawText(TextFormat("%d",g->aiResults[i].pathLength),tx+200,ty,18,rc);
        else uiDrawText("N/A",tx+200,ty,18,GRAY);
        uiDrawText(TextFormat("%d",g->aiResults[i].nodesExplored),tx+380,ty,18,rc);
        ty+=30;
    }
    ty+=20;
    uiDrawText("BFS and A* find the shortest path.",tx,ty,16,GRAY); ty+=22;
    uiDrawText("A* typically explores fewer nodes than BFS.",tx,ty,16,GRAY); ty+=22;
    uiDrawText("DFS may find a longer path.",tx,ty,16,GRAY); ty+=22;
    uiDrawText("Random walk is unpredictable.",tx,ty,16,GRAY);
}

/* ---- Name Entry ---- */

void drawNameEntry(Game *g)
{
    ClearBackground(COLOR_MENU_BG);
    int oy=centeredLayoutOffsetY();
    drawCenteredText("ENTER YOUR NAME",150+oy,40,COLOR_TITLE);
    const char *difficulty = g->difficulty==DIFF_EASY ? "Easy" :
        g->difficulty==DIFF_HARD ? "Hard" : "Normal";
    drawCenteredText(TextFormat("Difficulty: %s",difficulty),220+oy,24,LIGHTGRAY);

    int bw=420,bh=50,bx=(GetScreenWidth()-bw)/2,by=(GetScreenHeight()-bh)/2;
    Rectangle field={(float)bx,(float)by,(float)bw,(float)bh};
    Rectangle fieldFill={(float)(bx+2),(float)(by+2),(float)(bw-4),(float)(bh-4)};
    DrawRectangleRounded(field,0.18f,8,COLOR_MENU_HIGHLIGHT);
    DrawRectangleRounded(fieldFill,0.17f,8,(Color){40,42,54,255});
    int nameWidth=uiMeasureText(g->playerName,28);
    int nameX=bx+15;
    uiDrawText(g->playerName,nameX,by+12,28,WHITE);

    /* Blinking cursor */
    if ((int)(GetTime()*2)%2==0) {
        DrawRectangle(nameX+nameWidth+2,by+10,2,30,COLOR_MENU_HIGHLIGHT);
    }

    const char *keys[]={"ENTER","ESC"}; const char *actions[]={"Start Game","Back"};
    drawHintRow(keys,actions,2,GetScreenHeight()-38,16);
}
