/*
 * main.c — Entry point for Maze Escape with Intelligent Enemies
 *
 * Demonstrates: raylib window creation, game loop, timing, modular structure.
 */

#include "game.h"
#include "ui.h"
#include "save.h"
#include <stdlib.h>
#include <time.h>

int main(void)
{
    srand((unsigned int)time(NULL));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maze Escape with Intelligent Enemies");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game = {0};
    initGame(&game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        updateGame(&game, dt);

        BeginDrawing();
        ClearBackground(COLOR_BG);
        drawGameFrame(&game);
        EndDrawing();
    }

    cleanupGame(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
