#ifndef SCORE_H
#define SCORE_H

#include "config.h"

typedef struct {
    char name[MAX_NAME_LENGTH];
    int  score;
} ScoreEntry;

int  calculateLevelScore(int level, float timeRem, int health, int keys, int powerups);
int  loadHighScores(ScoreEntry s[], int max);
void saveHighScores(const ScoreEntry s[], int count);
void insertHighScore(ScoreEntry s[], int *count, int max, const char *name, int score);
void quickSortScores(ScoreEntry s[], int lo, int hi);

#endif /* SCORE_H */
