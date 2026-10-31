#include "game.h"
#include "save.h"
#include "score.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                           \
        }                                                                       \
    } while (0)

typedef struct {
    int magic, version, level, difficulty;
    int score, totalScore; float timer;
    int keysRequired, keysCollected;
    float px, py; int pgx, pgy, pHealth; float pStamina;
    int mw, mh, eCount, puCount;
} TestSaveHeader;

static TestSaveHeader validHeader(void)
{
    TestSaveHeader header = {0};
    header.magic = SAVE_MAGIC;
    header.version = SAVE_VERSION;
    header.level = 1;
    header.difficulty = DIFF_NORMAL;
    header.timer = 60.0f;
    header.px = 48.0f;
    header.py = 48.0f;
    header.pgx = 1;
    header.pgy = 1;
    header.pHealth = PLAYER_MAX_HEALTH;
    header.pStamina = PLAYER_MAX_STAMINA;
    header.mw = 11;
    header.mh = 11;
    return header;
}

static int writeSaveHeader(const TestSaveHeader *header)
{
    FILE *file = fopen(SAVE_FILE_PATH, "wb");
    CHECK(file != NULL);
    CHECK(fwrite(header, sizeof(*header), 1, file) == 1);
    CHECK(fclose(file) == 0);
    return 0;
}

static int testMalformedSaves(void)
{
    Game game = {0};
    TestSaveHeader header = validHeader();

    header.magic = 0;
    CHECK(writeSaveHeader(&header) == 0);
    CHECK(!loadGame(&game));
    CHECK(game.maze == NULL);

    header = validHeader();
    header.mw = 53;
    CHECK(writeSaveHeader(&header) == 0);
    CHECK(!loadGame(&game));
    CHECK(game.maze == NULL);

    header = validHeader();
    header.eCount = MAX_ENEMIES + 1;
    CHECK(writeSaveHeader(&header) == 0);
    CHECK(!loadGame(&game));
    CHECK(game.maze == NULL);

    header = validHeader();
    header.puCount = -1;
    CHECK(writeSaveHeader(&header) == 0);
    CHECK(!loadGame(&game));
    CHECK(game.maze == NULL);

    header = validHeader();
    CHECK(writeSaveHeader(&header) == 0);
    CHECK(!loadGame(&game));
    CHECK(game.maze == NULL);

    remove(SAVE_FILE_PATH);
    return 0;
}

static int testSaveRoundTrip(void)
{
    Game source = {0};
    Game loaded = {0};
    int cellCount = 11 * 11;

    source.currentLevel = 1;
    source.difficulty = DIFF_NORMAL;
    source.timer = 75.0f;
    source.score = 123;
    source.totalScore = 456;
    source.keysRequired = 0;
    source.maze = createMaze(11, 11);
    CHECK(source.maze != NULL);
    srand(7);
    generateMaze(source.maze);
    CHECK(placeMazeObjects(source.maze, 0));
    source.player.gridX = source.maze->start.x;
    source.player.gridY = source.maze->start.y;
    source.player.x = 48.0f;
    source.player.y = 48.0f;
    source.player.health = PLAYER_MAX_HEALTH;
    source.player.stamina = PLAYER_MAX_STAMINA;
    source.explored = (bool *)calloc((size_t)cellCount, sizeof(bool));
    CHECK(source.explored != NULL);

    CHECK(saveGame(&source));
    CHECK(loadGame(&loaded));
    CHECK(loaded.maze != NULL);
    CHECK(loaded.maze->width == source.maze->width);
    CHECK(loaded.maze->height == source.maze->height);
    CHECK(loaded.score == source.score);
    CHECK(loaded.totalScore == source.totalScore);
    CHECK(loaded.player.gridX == source.player.gridX);
    CHECK(loaded.player.gridY == source.player.gridY);

    cleanupLevel(&source);
    cleanupLevel(&loaded);
    remove(SAVE_FILE_PATH);
    return 0;
}

static int writeScores(int count, const ScoreEntry *scores, int scoreCount)
{
    FILE *file = fopen(HIGHSCORE_FILE_PATH, "wb");
    CHECK(file != NULL);
    CHECK(fwrite(&count, sizeof(count), 1, file) == 1);
    if (scoreCount > 0)
        CHECK(fwrite(scores, sizeof(*scores), (size_t)scoreCount, file) ==
              (size_t)scoreCount);
    CHECK(fclose(file) == 0);
    return 0;
}

static int testMalformedLeaderboards(void)
{
    ScoreEntry scores[MAX_HIGH_SCORES] = {0};
    ScoreEntry valid[2] = { {"ALPHA", 900}, {"BETA", 500} };

    CHECK(writeScores(-1, NULL, 0) == 0);
    CHECK(loadHighScores(scores, MAX_HIGH_SCORES) == 0);

    CHECK(writeScores(MAX_HIGH_SCORES + 1, NULL, 0) == 0);
    CHECK(loadHighScores(scores, MAX_HIGH_SCORES) == 0);

    CHECK(writeScores(2, valid, 1) == 0);
    CHECK(loadHighScores(scores, MAX_HIGH_SCORES) == 0);

    CHECK(writeScores(2, valid, 2) == 0);
    CHECK(loadHighScores(scores, MAX_HIGH_SCORES) == 2);
    CHECK(strcmp(scores[0].name, "ALPHA") == 0);
    CHECK(scores[1].score == 500);

    remove(HIGHSCORE_FILE_PATH);
    return 0;
}

int main(void)
{
    if (testMalformedSaves() != 0 ||
        testSaveRoundTrip() != 0 ||
        testMalformedLeaderboards() != 0)
        return EXIT_FAILURE;

    puts("persistence tests passed");
    return EXIT_SUCCESS;
}
