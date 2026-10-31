/*
 * pathfinding.h — BFS, A*, DFS, Greedy, Random pathfinding + data structures
 */

#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "config.h"

/* ---- Path result ---- */
typedef struct {
    GridNode *nodes;        /* caller must free */
    int       length;
    int       nodesExplored;
    bool      found;
} PathResult;

/* ---- Queue (circular array) for BFS ---- */
typedef struct {
    GridNode *data;
    int front, rear, size, capacity;
} Queue;

void      queueInit(Queue *q, int capacity);
void      queuePush(Queue *q, GridNode node);
GridNode  queuePop(Queue *q);
bool      queueEmpty(const Queue *q);
void      queueFree(Queue *q);

/* ---- Min-Heap for A* ---- */
typedef struct {
    int x, y, f, g, h;
} AStarNode;

typedef struct {
    AStarNode *data;
    int size, capacity;
} MinHeap;

void       heapInit(MinHeap *h, int capacity);
void       heapPush(MinHeap *h, AStarNode node);
AStarNode  heapPop(MinHeap *h);
bool       heapEmpty(const MinHeap *h);
void       heapFree(MinHeap *h);

/* ---- Pathfinding algorithms ---- */
struct Maze;
PathResult findPathBFS(struct Maze *maze, GridNode start, GridNode end);
PathResult findPathAStar(struct Maze *maze, GridNode start, GridNode end);
PathResult findPathDFS(struct Maze *maze, GridNode start, GridNode end);
PathResult findPathGreedy(struct Maze *maze, GridNode start, GridNode end);
PathResult findPathRandom(struct Maze *maze, GridNode start, GridNode end, int maxSteps);
void       freePath(PathResult *result);

/* ---- AI Comparison ---- */
typedef struct {
    char      name[32];
    int       pathLength;
    int       nodesExplored;
    bool      found;
    GridNode *path;
    int       pathCount;
} AIComparisonResult;

void runAIComparison(struct Maze *maze, GridNode start, GridNode end,
                     AIComparisonResult results[5]);
void freeAIComparisonResults(AIComparisonResult results[5]);

#endif /* PATHFINDING_H */
