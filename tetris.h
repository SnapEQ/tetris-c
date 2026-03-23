#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>

void initGame(void);
void gameLoop(bool *running, int *fallCounter);

#endif
