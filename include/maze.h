/*
 * maze.h — Maze data structure, procedural generation, and validation
 */

#ifndef MAZE_H
#define MAZE_H

#include "config.h"

typedef struct Maze {
    CellType **grid;    /* 2D dynamically allocated grid (grid[y][x]) */
    int width;          /* always odd */
    int height;
    GridNode start;
    GridNode exitPos;
    GridNode *keys;     /* dynamically allocated key positions */
    int keyCount;
} Maze;

Maze     *createMaze(int width, int height);
void      destroyMaze(Maze *maze);
void      generateMaze(Maze *maze);
bool      placeMazeObjects(Maze *maze, int keyCount);
bool      validateMaze(Maze *maze);
GridNode  randomPathCell(Maze *maze);
bool      isCellWalkable(Maze *maze, int x, int y);

#endif /* MAZE_H */
