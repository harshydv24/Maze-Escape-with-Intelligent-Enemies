/*
 * score.c — Score calculation, QuickSort, high-score persistence
 *
 * Demonstrates: QuickSort (Lomuto partition), binary file I/O.
 */

#include "score.h"
#include <stdio.h>
#include <string.h>

int calculateLevelScore(int level, float timeRem, int health, int keys, int powerups)
{
    int s = SCORE_BASE_LEVEL * level;
    s += (int)timeRem * SCORE_TIME_BONUS;
    s += health * SCORE_HEALTH_BONUS;
    s += keys * SCORE_KEY_BONUS;
    s += powerups * SCORE_POWERUP_BONUS;
    return s > 0 ? s : 0;
}

/* ---- QuickSort (descending) ---- */

static int partition(ScoreEntry a[], int lo, int hi)
{
    int piv = a[hi].score;
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        if (a[j].score >= piv) {
            i++;
            ScoreEntry t = a[i]; a[i] = a[j]; a[j] = t;
        }
    }
    ScoreEntry t = a[i+1]; a[i+1] = a[hi]; a[hi] = t;
    return i + 1;
}

void quickSortScores(ScoreEntry a[], int lo, int hi)
{
    if (lo < hi) {
        int p = partition(a, lo, hi);
        quickSortScores(a, lo, p - 1);
        quickSortScores(a, p + 1, hi);
    }
}

/* ---- File I/O ---- */

int loadHighScores(ScoreEntry s[], int max)
{
    FILE *f = fopen(HIGHSCORE_FILE_PATH, "rb");
    if (!f) return 0;
    int n = 0;
    if (fread(&n, sizeof(int), 1, f) != 1) { fclose(f); return 0; }
    if (n > max) n = max;
    fread(s, sizeof(ScoreEntry), n, f);
    fclose(f);
    return n;
}

void saveHighScores(const ScoreEntry s[], int count)
{
    FILE *f = fopen(HIGHSCORE_FILE_PATH, "wb");
    if (!f) { fprintf(stderr, "Warning: cannot save high scores.\n"); return; }
    fwrite(&count, sizeof(int), 1, f);
    fwrite(s, sizeof(ScoreEntry), count, f);
    fclose(f);
}

void insertHighScore(ScoreEntry s[], int *count, int max, const char *name, int score)
{
    int idx = *count;
    if (idx >= max) {
        if (score <= s[max-1].score) return;
        idx = max - 1;
    } else (*count)++;

    strncpy(s[idx].name, name, MAX_NAME_LENGTH - 1);
    s[idx].name[MAX_NAME_LENGTH-1] = '\0';
    s[idx].score = score;
    quickSortScores(s, 0, *count - 1);
}
