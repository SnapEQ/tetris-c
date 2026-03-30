#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>

void initGame();
void gameLoop(bool *running, int *fallCounter);

#endif
