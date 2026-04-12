#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>

extern int pieces[7][4][4][4];

void initGame();
void gameLoop(bool *running, int *fallCounter);

#endif
