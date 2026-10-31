/*
 * maze.c — Procedural maze generation using Recursive Backtracking (DFS)
 *
 * Demonstrates:
 *   - Dynamic memory allocation (malloc, calloc, free)
 *   - Pointers to pointers (2D dynamic array)
 *   - Recursion (DFS maze carving)
 *   - BFS for connectivity validation
 */

#include "maze.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ================================================================
 * Maze creation / destruction
 * ================================================================ */

Maze *createMaze(int width, int height)
{
    if (width  % 2 == 0) width++;
    if (height % 2 == 0) height++;

    Maze *maze = (Maze *)calloc(1, sizeof(Maze));
    if (!maze) { fprintf(stderr, "Error: Failed to allocate Maze.\n"); return NULL; }

    maze->width  = width;
    maze->height = height;

    maze->grid = (CellType **)malloc(sizeof(CellType *) * height);
    if (!maze->grid) {
        fprintf(stderr, "Error: Failed to allocate maze grid.\n");
        free(maze); return NULL;
    }
    for (int y = 0; y < height; y++) {
        maze->grid[y] = (CellType *)calloc(width, sizeof(CellType));
        if (!maze->grid[y]) {
            for (int j = 0; j < y; j++) free(maze->grid[j]);
            free(maze->grid); free(maze); return NULL;
        }
    }

    /* All cells start as walls */
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            maze->grid[y][x] = CELL_WALL;

    maze->keys     = NULL;
    maze->keyCount = 0;
    return maze;
}

void destroyMaze(Maze *maze)
{
    if (!maze) return;
    if (maze->grid) {
        for (int y = 0; y < maze->height; y++) free(maze->grid[y]);
        free(maze->grid);
    }
    free(maze->keys);
    free(maze);
}

/* ================================================================
 * Recursive Backtracking (DFS) maze carving
 *
 * Logical cells sit at odd coordinates (1,3,5,…).
 * Walls occupy even coordinates and the outer border.
 * ================================================================ */

static void carvePath(Maze *maze, int cx, int cy)
{
    maze->grid[cy][cx] = CELL_PATH;

    int dx[] = { 2, 0, -2, 0 };
    int dy[] = { 0, 2, 0, -2 };

    /* Fisher-Yates shuffle */
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int t;
        t = dx[i]; dx[i] = dx[j]; dx[j] = t;
        t = dy[i]; dy[i] = dy[j]; dy[j] = t;
    }

    for (int i = 0; i < 4; i++) {
        int nx = cx + dx[i], ny = cy + dy[i];
        if (nx > 0 && nx < maze->width - 1 &&
            ny > 0 && ny < maze->height - 1 &&
            maze->grid[ny][nx] == CELL_WALL)
        {
            maze->grid[cy + dy[i] / 2][cx + dx[i] / 2] = CELL_PATH;
            carvePath(maze, nx, ny);
        }
    }
}

void generateMaze(Maze *maze)
{
    for (int y = 0; y < maze->height; y++)
        for (int x = 0; x < maze->width; x++)
            maze->grid[y][x] = CELL_WALL;
    carvePath(maze, 1, 1);
}

/* ================================================================
 * Validation — BFS connectivity check
 * ================================================================ */

bool validateMaze(Maze *maze)
{
    int w = maze->width, h = maze->height, size = w * h;
    bool *vis = (bool *)calloc(size, sizeof(bool));
    if (!vis) return false;

    typedef struct { int x, y; } Pos;
    Pos *q = (Pos *)malloc(sizeof(Pos) * size);
    if (!q) { free(vis); return false; }

    int front = 0, rear = 0;
    q[rear++] = (Pos){ maze->start.x, maze->start.y };
    vis[maze->start.y * w + maze->start.x] = true;

    const int dx[] = { 1, -1, 0, 0 };
    const int dy[] = { 0, 0, 1, -1 };

    while (front < rear) {
        Pos c = q[front++];
        for (int d = 0; d < 4; d++) {
            int nx = c.x + dx[d], ny = c.y + dy[d];
            if (nx >= 0 && nx < w && ny >= 0 && ny < h &&
                !vis[ny * w + nx] && maze->grid[ny][nx] != CELL_WALL)
            {
                vis[ny * w + nx] = true;
                q[rear++] = (Pos){ nx, ny };
            }
        }
    }

    bool ok = vis[maze->exitPos.y * w + maze->exitPos.x];
    for (int i = 0; i < maze->keyCount && ok; i++)
        ok = vis[maze->keys[i].y * w + maze->keys[i].x];

    free(vis); free(q);
    return ok;
}

/* ================================================================
 * Utilities
 * ================================================================ */

bool isCellWalkable(Maze *maze, int x, int y)
{
    if (x < 0 || x >= maze->width || y < 0 || y >= maze->height) return false;
    return maze->grid[y][x] != CELL_WALL;
}

GridNode randomPathCell(Maze *maze)
{
    GridNode n;
    int att = 0;
    do {
        n.x = 1 + rand() % (maze->width - 2);
        n.y = 1 + rand() % (maze->height - 2);
        att++;
    } while (maze->grid[n.y][n.x] != CELL_PATH && att < 10000);
    return n;
}

static int manDist(GridNode a, GridNode b)
{
    int dx = a.x - b.x; if (dx < 0) dx = -dx;
    int dy = a.y - b.y; if (dy < 0) dy = -dy;
    return dx + dy;
}

void placeMazeObjects(Maze *maze, int keyCount)
{
    maze->start = (GridNode){ 1, 1 };
    maze->grid[1][1] = CELL_START;

    maze->exitPos = (GridNode){ maze->width - 2, maze->height - 2 };
    maze->grid[maze->height - 2][maze->width - 2] = CELL_EXIT;

    free(maze->keys);
    maze->keyCount = keyCount;
    maze->keys = (GridNode *)malloc(sizeof(GridNode) * keyCount);
    if (!maze->keys) { maze->keyCount = 0; return; }

    for (int i = 0; i < keyCount; i++) {
        GridNode pos; int att = 0;
        do { pos = randomPathCell(maze); att++; }
        while ((manDist(pos, maze->start) < 8 ||
                maze->grid[pos.y][pos.x] != CELL_PATH) && att < 10000);
        maze->keys[i] = pos;
        maze->grid[pos.y][pos.x] = CELL_KEY;
    }
}
