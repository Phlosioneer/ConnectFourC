
#ifndef GRID_H
#define GRID_H

#pragma once

#include "common.h"

#define BOARD_HEIGHT 6
#define BOARD_WIDTH 7

#define BOARD_PLAYER1 'O'
#define BOARD_PLAYER2 'X'
#define BOARD_EMPTY ' '

typedef struct Board Board;

Board* newBoard(void);
void freeBoard(Board* board);

void drawBoard(Board* board);



#endif // !GRID_H

