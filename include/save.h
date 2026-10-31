#ifndef SAVE_H
#define SAVE_H

#include "config.h"

struct Game;
bool saveGame(const struct Game *game);
bool loadGame(struct Game *game);
bool saveFileExists(void);

#endif /* SAVE_H */
