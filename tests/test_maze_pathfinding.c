#include "maze.h"
#include "pathfinding.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                           \
        }                                                                       \
    } while (0)

static bool sameNode(GridNode a, GridNode b)
{
    return a.x == b.x && a.y == b.y;
}

static bool validPath(const Maze *maze, PathResult result,
                      GridNode start, GridNode end)
{
    if (!result.found || !result.nodes || result.length < 1 ||
        !sameNode(result.nodes[0], start) ||
        !sameNode(result.nodes[result.length - 1], end))
        return false;

    for (int i = 0; i < result.length; i++) {
        if (!isCellWalkable((Maze *)maze, result.nodes[i].x, result.nodes[i].y))
            return false;
        if (i > 0) {
            int dx = abs(result.nodes[i].x - result.nodes[i - 1].x);
            int dy = abs(result.nodes[i].y - result.nodes[i - 1].y);
            if (dx + dy != 1) return false;
        }
    }
    return true;
}

static int testGeneratedMaze(void)
{
    const int sizes[] = {11, 21, 31};

    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++) {
        Maze *maze = createMaze(sizes[s], sizes[s]);
        CHECK(maze != NULL);
        srand((unsigned int)(100 + sizes[s]));
        generateMaze(maze);
        CHECK(placeMazeObjects(maze, 1));
        CHECK(validateMaze(maze));
        CHECK(maze->grid[maze->start.y][maze->start.x] == CELL_START);
        CHECK(maze->grid[maze->exitPos.y][maze->exitPos.x] == CELL_EXIT);
        CHECK(maze->keyCount == 1);
        CHECK(isCellWalkable(maze, maze->keys[0].x, maze->keys[0].y));

        PathResult bfs = findPathBFS(maze, maze->start, maze->exitPos);
        PathResult astar = findPathAStar(maze, maze->start, maze->exitPos);
        PathResult dfs = findPathDFS(maze, maze->start, maze->exitPos);
        CHECK(validPath(maze, bfs, maze->start, maze->exitPos));
        CHECK(validPath(maze, astar, maze->start, maze->exitPos));
        CHECK(validPath(maze, dfs, maze->start, maze->exitPos));
        CHECK(bfs.length == astar.length);
        freePath(&bfs);
        freePath(&astar);
        freePath(&dfs);
        destroyMaze(maze);
    }
    return 0;
}

static int testKnownGrid(void)
{
    Maze *maze = createMaze(7, 7);
    CHECK(maze != NULL);
    for (int y = 0; y < maze->height; y++)
        for (int x = 0; x < maze->width; x++)
            maze->grid[y][x] = CELL_PATH;

    GridNode start = {1, 1};
    GridNode end = {5, 5};
    maze->grid[3][1] = CELL_WALL;
    maze->grid[3][2] = CELL_WALL;
    maze->grid[3][3] = CELL_WALL;
    maze->grid[3][4] = CELL_WALL;
    maze->grid[3][5] = CELL_WALL;

    PathResult bfs = findPathBFS(maze, start, end);
    PathResult astar = findPathAStar(maze, start, end);
    CHECK(validPath(maze, bfs, start, end));
    CHECK(validPath(maze, astar, start, end));
    CHECK(bfs.length == 11);
    CHECK(astar.length == bfs.length);
    freePath(&bfs);
    freePath(&astar);

    for (int y = 0; y < maze->height; y++)
        for (int x = 0; x < maze->width; x++)
            maze->grid[y][x] = CELL_WALL;
    maze->grid[start.y][start.x] = CELL_PATH;
    maze->grid[end.y][end.x] = CELL_PATH;
    PathResult unreachable = findPathBFS(maze, start, end);
    CHECK(!unreachable.found);
    CHECK(unreachable.nodes == NULL);
    freePath(&unreachable);

    destroyMaze(maze);
    return 0;
}

static int testQueueAndHeap(void)
{
    Queue queue;
    queueInit(&queue, 2);
    for (int i = 0; i < 10; i++) queuePush(&queue, (GridNode){i, i + 1});
    for (int i = 0; i < 10; i++) {
        GridNode node = queuePop(&queue);
        CHECK(node.x == i && node.y == i + 1);
    }
    CHECK(queueEmpty(&queue));
    queueFree(&queue);

    MinHeap heap;
    heapInit(&heap, 2);
    heapPush(&heap, (AStarNode){0, 0, 9, 0, 9});
    heapPush(&heap, (AStarNode){0, 1, 2, 1, 1});
    heapPush(&heap, (AStarNode){1, 0, 6, 2, 4});
    CHECK(heapPop(&heap).f == 2);
    CHECK(heapPop(&heap).f == 6);
    CHECK(heapPop(&heap).f == 9);
    CHECK(heapEmpty(&heap));
    heapFree(&heap);
    return 0;
}

int main(void)
{
    if (testGeneratedMaze() != 0 ||
        testKnownGrid() != 0 ||
        testQueueAndHeap() != 0)
        return EXIT_FAILURE;

    puts("maze/pathfinding tests passed");
    return EXIT_SUCCESS;
}
