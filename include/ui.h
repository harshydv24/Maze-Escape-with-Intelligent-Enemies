#ifndef UI_H
#define UI_H

#include "config.h"

struct Game;

void drawMenu(struct Game *g);
void drawDifficulty(struct Game *g);
void drawPlaying(struct Game *g);
void drawPaused(struct Game *g);
void drawGameOver(struct Game *g);
void drawLevelComplete(struct Game *g);
void drawHighScores(struct Game *g);
void drawInstructions(struct Game *g);
void drawSettings(struct Game *g);
void drawAIDemo(struct Game *g);
void drawNameEntry(struct Game *g);
void drawHUD(struct Game *g);
void drawNotifications(struct Game *g);
void drawDebugInfo(struct Game *g);
void drawCenteredText(const char *text, int y, int fontSize, Color color);

#endif /* UI_H */
