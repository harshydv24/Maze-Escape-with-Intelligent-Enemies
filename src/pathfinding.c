/*
 * pathfinding.c — Manual implementations of BFS, A*, DFS, Greedy, Random
 *
 * Demonstrates:
 *   - Queue (circular array) for BFS
 *   - Min-heap priority queue for A*
 *   - Stack (explicit array) for DFS
 *   - Graph traversal, heuristics, dynamic memory
 */

#include "pathfinding.h"
#include "maze.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static const int DX[4] = { 1, -1, 0, 0 };
static const int DY[4] = { 0, 0, 1, -1 };

/* ---- Queue ---- */

void queueInit(Queue *q, int cap)
{
    q->data = (GridNode *)malloc(sizeof(GridNode) * cap);
    q->front = q->rear = q->size = 0;
    q->capacity = cap;
}
void queuePush(Queue *q, GridNode n)
{
    if (q->size >= q->capacity) {
        int nc = q->capacity * 2;
        GridNode *nd = (GridNode *)malloc(sizeof(GridNode) * nc);
        if (!nd) return;
        for (int i = 0; i < q->size; i++)
            nd[i] = q->data[(q->front + i) % q->capacity];
        free(q->data);
        q->data = nd; q->front = 0; q->rear = q->size; q->capacity = nc;
    }
    q->data[q->rear] = n;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}
GridNode queuePop(Queue *q)
{
    GridNode n = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return n;
}
bool queueEmpty(const Queue *q) { return q->size == 0; }
void queueFree(Queue *q) { free(q->data); q->data = NULL; q->size = q->capacity = 0; }

/* ---- Min-Heap ---- */

void heapInit(MinHeap *h, int cap)
{
    h->data = (AStarNode *)malloc(sizeof(AStarNode) * cap);
    h->size = 0; h->capacity = cap;
}
static void hSwap(AStarNode *a, AStarNode *b) { AStarNode t = *a; *a = *b; *b = t; }
static void hUp(MinHeap *h, int i)
{
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[i].f < h->data[p].f) { hSwap(&h->data[i], &h->data[p]); i = p; }
        else break;
    }
}
static void hDown(MinHeap *h, int i)
{
    for (;;) {
        int s = i, l = 2*i+1, r = 2*i+2;
        if (l < h->size && h->data[l].f < h->data[s].f) s = l;
        if (r < h->size && h->data[r].f < h->data[s].f) s = r;
        if (s != i) { hSwap(&h->data[i], &h->data[s]); i = s; } else break;
    }
}
void heapPush(MinHeap *h, AStarNode n)
{
    if (h->size >= h->capacity) {
        int nc = h->capacity * 2;
        AStarNode *nd = (AStarNode *)realloc(h->data, sizeof(AStarNode) * nc);
        if (!nd) return;
        h->data = nd; h->capacity = nc;
    }
    h->data[h->size] = n;
    hUp(h, h->size);
    h->size++;
}
AStarNode heapPop(MinHeap *h)
{
    AStarNode top = h->data[0];
    h->size--;
    if (h->size > 0) { h->data[0] = h->data[h->size]; hDown(h, 0); }
    return top;
}
bool heapEmpty(const MinHeap *h) { return h->size == 0; }
void heapFree(MinHeap *h) { free(h->data); h->data = NULL; h->size = h->capacity = 0; }

/* ---- Path reconstruction ---- */

static PathResult reconstruct(int *px, int *py, int w,
                               GridNode s, GridNode e, int explored)
{
    PathResult r = { NULL, 0, explored, true };
    int len = 0, cx = e.x, cy = e.y;
    while (!(cx == s.x && cy == s.y)) {
        len++;
        int tx = px[cy * w + cx], ty = py[cy * w + cx];
        cx = tx; cy = ty;
        if (len > w * w) { r.found = false; return r; }
    }
    len++;
    r.nodes = (GridNode *)malloc(sizeof(GridNode) * len);
    r.length = len;
    if (!r.nodes) { r.found = false; return r; }
    cx = e.x; cy = e.y;
    for (int i = len - 1; i >= 0; i--) {
        r.nodes[i] = (GridNode){ cx, cy };
        int tx = px[cy * w + cx], ty = py[cy * w + cx];
        cx = tx; cy = ty;
    }
    return r;
}

/* ================================================================
 * BFS — Breadth-First Search
 * Guarantees shortest path in unweighted grid.
 * ================================================================ */

PathResult findPathBFS(Maze *maze, GridNode start, GridNode end)
{
    PathResult no = { NULL, 0, 0, false };
    int w = maze->width, h = maze->height, sz = w * h;
    bool *vis = (bool *)calloc(sz, sizeof(bool));
    int  *px  = (int *)malloc(sizeof(int) * sz);
    int  *py  = (int *)malloc(sizeof(int) * sz);
    if (!vis || !px || !py) { free(vis); free(px); free(py); return no; }
    memset(px, -1, sizeof(int) * sz);
    memset(py, -1, sizeof(int) * sz);

    Queue q; queueInit(&q, 256);
    vis[start.y * w + start.x] = true;
    px[start.y * w + start.x] = start.x;
    py[start.y * w + start.x] = start.y;
    queuePush(&q, start);
    int explored = 0;
    PathResult result = no;

    while (!queueEmpty(&q)) {
        GridNode c = queuePop(&q);
        explored++;
        if (c.x == end.x && c.y == end.y) {
            result = reconstruct(px, py, w, start, end, explored);
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nx = c.x + DX[d], ny = c.y + DY[d];
            if (nx >= 0 && nx < w && ny >= 0 && ny < h &&
                !vis[ny * w + nx] && isCellWalkable(maze, nx, ny))
            {
                vis[ny * w + nx] = true;
                px[ny * w + nx] = c.x;
                py[ny * w + nx] = c.y;
                queuePush(&q, (GridNode){ nx, ny });
            }
        }
    }
    result.nodesExplored = explored;
    queueFree(&q); free(vis); free(px); free(py);
    return result;
}

/* ================================================================
 * A* — uses Manhattan heuristic, guarantees optimal path
 *   f(n) = g(n) + h(n)
 * ================================================================ */

static int manhattan(int x1, int y1, int x2, int y2)
{
    int dx = x1-x2; if (dx < 0) dx = -dx;
    int dy = y1-y2; if (dy < 0) dy = -dy;
    return dx + dy;
}

PathResult findPathAStar(Maze *maze, GridNode start, GridNode end)
{
    PathResult no = { NULL, 0, 0, false };
    int w = maze->width, h = maze->height, sz = w * h;
    bool *cl = (bool *)calloc(sz, sizeof(bool));
    int  *gs = (int *)malloc(sizeof(int) * sz);
    int  *px = (int *)malloc(sizeof(int) * sz);
    int  *py = (int *)malloc(sizeof(int) * sz);
    if (!cl || !gs || !px || !py) { free(cl); free(gs); free(px); free(py); return no; }
    for (int i = 0; i < sz; i++) gs[i] = INT_MAX;
    memset(px, -1, sizeof(int) * sz);
    memset(py, -1, sizeof(int) * sz);

    MinHeap op; heapInit(&op, 256);
    gs[start.y*w+start.x] = 0;
    px[start.y*w+start.x] = start.x;
    py[start.y*w+start.x] = start.y;
    int hv = manhattan(start.x, start.y, end.x, end.y);
    heapPush(&op, (AStarNode){ start.x, start.y, hv, 0, hv });
    int explored = 0;
    PathResult result = no;

    while (!heapEmpty(&op)) {
        AStarNode cur = heapPop(&op);
        if (cl[cur.y*w+cur.x]) continue;
        cl[cur.y*w+cur.x] = true;
        explored++;
        if (cur.x == end.x && cur.y == end.y) {
            result = reconstruct(px, py, w, start, end, explored);
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nx = cur.x+DX[d], ny = cur.y+DY[d];
            if (nx>=0 && nx<w && ny>=0 && ny<h &&
                !cl[ny*w+nx] && isCellWalkable(maze,nx,ny))
            {
                int tg = cur.g + 1;
                if (tg < gs[ny*w+nx]) {
                    gs[ny*w+nx] = tg;
                    px[ny*w+nx] = cur.x;
                    py[ny*w+nx] = cur.y;
                    int hh = manhattan(nx, ny, end.x, end.y);
                    heapPush(&op, (AStarNode){ nx, ny, tg+hh, tg, hh });
                }
            }
        }
    }
    result.nodesExplored = explored;
    heapFree(&op); free(cl); free(gs); free(px); free(py);
    return result;
}

/* ================================================================
 * DFS — Depth-First Search (explicit stack)
 * ================================================================ */

PathResult findPathDFS(Maze *maze, GridNode start, GridNode end)
{
    PathResult no = { NULL, 0, 0, false };
    int w = maze->width, h = maze->height, sz = w * h;
    bool *vis = (bool *)calloc(sz, sizeof(bool));
    int  *px  = (int *)malloc(sizeof(int) * sz);
    int  *py  = (int *)malloc(sizeof(int) * sz);
    GridNode *stk = (GridNode *)malloc(sizeof(GridNode) * sz);
    if (!vis||!px||!py||!stk) { free(vis);free(px);free(py);free(stk); return no; }
    memset(px,-1,sizeof(int)*sz);
    memset(py,-1,sizeof(int)*sz);

    int top = 0;
    stk[top++] = start;
    vis[start.y*w+start.x] = true;
    px[start.y*w+start.x] = start.x;
    py[start.y*w+start.x] = start.y;
    int explored = 0;
    PathResult result = no;

    while (top > 0) {
        GridNode c = stk[--top];
        explored++;
        if (c.x == end.x && c.y == end.y) {
            result = reconstruct(px,py,w,start,end,explored);
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nx = c.x+DX[d], ny = c.y+DY[d];
            if (nx>=0&&nx<w&&ny>=0&&ny<h&&!vis[ny*w+nx]&&isCellWalkable(maze,nx,ny)) {
                vis[ny*w+nx] = true;
                px[ny*w+nx] = c.x;
                py[ny*w+nx] = c.y;
                stk[top++] = (GridNode){ nx, ny };
            }
        }
    }
    result.nodesExplored = explored;
    free(stk); free(vis); free(px); free(py);
    return result;
}

/* ================================================================
 * Greedy Best-First Search (f = h only)
 * ================================================================ */

PathResult findPathGreedy(Maze *maze, GridNode start, GridNode end)
{
    PathResult no = { NULL, 0, 0, false };
    int w = maze->width, h = maze->height, sz = w * h;
    bool *cl = (bool *)calloc(sz, sizeof(bool));
    int  *px = (int *)malloc(sizeof(int) * sz);
    int  *py = (int *)malloc(sizeof(int) * sz);
    if (!cl||!px||!py) { free(cl);free(px);free(py); return no; }
    memset(px,-1,sizeof(int)*sz);
    memset(py,-1,sizeof(int)*sz);

    MinHeap op; heapInit(&op, 256);
    px[start.y*w+start.x] = start.x;
    py[start.y*w+start.x] = start.y;
    int hs = manhattan(start.x,start.y,end.x,end.y);
    heapPush(&op, (AStarNode){ start.x, start.y, hs, 0, hs });
    int explored = 0;
    PathResult result = no;

    while (!heapEmpty(&op)) {
        AStarNode cur = heapPop(&op);
        if (cl[cur.y*w+cur.x]) continue;
        cl[cur.y*w+cur.x] = true;
        explored++;
        if (cur.x==end.x && cur.y==end.y) {
            result = reconstruct(px,py,w,start,end,explored);
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nx=cur.x+DX[d], ny=cur.y+DY[d];
            if (nx>=0&&nx<w&&ny>=0&&ny<h&&!cl[ny*w+nx]&&isCellWalkable(maze,nx,ny)) {
                if (px[ny*w+nx]==-1) { px[ny*w+nx]=cur.x; py[ny*w+nx]=cur.y; }
                int hh = manhattan(nx,ny,end.x,end.y);
                heapPush(&op, (AStarNode){ nx, ny, hh, 0, hh });
            }
        }
    }
    result.nodesExplored = explored;
    heapFree(&op); free(cl); free(px); free(py);
    return result;
}

/* ================================================================
 * Random Walk
 * ================================================================ */

PathResult findPathRandom(Maze *maze, GridNode start, GridNode end, int maxSteps)
{
    PathResult r = { NULL, 0, 0, false };
    int w = maze->width, h = maze->height;
    GridNode *path = (GridNode *)malloc(sizeof(GridNode)*(maxSteps+1));
    if (!path) return r;

    GridNode cur = start;
    path[0] = cur;
    int len = 1;

    while (len <= maxSteps) {
        if (cur.x==end.x && cur.y==end.y) {
            r.nodes = path; r.length = len;
            r.nodesExplored = len; r.found = true;
            return r;
        }
        GridNode nb[4]; int nc = 0;
        for (int d = 0; d < 4; d++) {
            int nx=cur.x+DX[d], ny=cur.y+DY[d];
            if (nx>=0&&nx<w&&ny>=0&&ny<h&&isCellWalkable(maze,nx,ny))
                nb[nc++] = (GridNode){ nx, ny };
        }
        if (nc == 0) break;
        cur = nb[rand()%nc];
        path[len++] = cur;
    }
    free(path);
    return r;
}

/* ---- Free helper ---- */
void freePath(PathResult *r)
{
    if (r && r->nodes) { free(r->nodes); r->nodes = NULL; }
    if (r) { r->length = 0; r->found = false; }
}

/* ================================================================
 * AI Comparison — run all five on the same maze
 * ================================================================ */

void runAIComparison(Maze *maze, GridNode start, GridNode end,
                     AIComparisonResult res[5])
{
    const char *names[] = {"Random","Greedy","DFS","BFS","A*"};
    for (int i = 0; i < 5; i++) {
        strncpy(res[i].name, names[i], 31);
        res[i].name[31] = '\0';
    }
    PathResult pr;

    pr = findPathRandom(maze, start, end, 5000);
    res[0].pathLength=pr.found?pr.length:-1; res[0].nodesExplored=pr.nodesExplored;
    res[0].found=pr.found; res[0].path=pr.nodes; res[0].pathCount=pr.length;

    pr = findPathGreedy(maze, start, end);
    res[1].pathLength=pr.found?pr.length:-1; res[1].nodesExplored=pr.nodesExplored;
    res[1].found=pr.found; res[1].path=pr.nodes; res[1].pathCount=pr.length;

    pr = findPathDFS(maze, start, end);
    res[2].pathLength=pr.found?pr.length:-1; res[2].nodesExplored=pr.nodesExplored;
    res[2].found=pr.found; res[2].path=pr.nodes; res[2].pathCount=pr.length;

    pr = findPathBFS(maze, start, end);
    res[3].pathLength=pr.found?pr.length:-1; res[3].nodesExplored=pr.nodesExplored;
    res[3].found=pr.found; res[3].path=pr.nodes; res[3].pathCount=pr.length;

    pr = findPathAStar(maze, start, end);
    res[4].pathLength=pr.found?pr.length:-1; res[4].nodesExplored=pr.nodesExplored;
    res[4].found=pr.found; res[4].path=pr.nodes; res[4].pathCount=pr.length;
}

void freeAIComparisonResults(AIComparisonResult res[5])
{
    for (int i = 0; i < 5; i++) { free(res[i].path); res[i].path = NULL; }
}
