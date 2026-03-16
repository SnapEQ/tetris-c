#include <stdlib.h>
#include <stdbool.h>
#include "piece.c"
#include "primlib.h"

#define SCREEN_W gfx_screenWidth()
#define SCREEN_H gfx_screenHeight()
#define CONTAINER_WIDTH 30
#define CONTAINER_HEIGHT 40
#define FRAME_DELAY_MS 16
#define CELL_SIZE 10
#define TOP_OFFSET 150

typedef struct{
    int type;
    int x;
    int y;
    int rotation;
} Piece;

typedef struct{
    int board[CONTAINER_HEIGHT][CONTAINER_WIDTH];
    int x1;
    int y1;
    int x2;
    int y2;
} Board;

Board board;


void drawBoard(void){
    gfx_rect(board.x1, board.y1, board.x2, board.y2, WHITE);
    gfx_line(board.x1, board.y1, board.x2, board.y1, BLACK);
}

void drawScreen(void){
   gfx_filledRect(0, 0, SCREEN_W- 1, SCREEN_H - 1, BLACK);
    drawBoard();
}

void initGame(void){
    board.x1 = SCREEN_W/2 - CONTAINER_WIDTH*CELL_SIZE/2;
    board.x2 = SCREEN_W/2 + CONTAINER_WIDTH*CELL_SIZE/2;
    board.y1 = SCREEN_H/2 - CONTAINER_HEIGHT*CELL_SIZE/2 - TOP_OFFSET;
    board.y2 = SCREEN_H - 1;

}

int main(int argc, char *argv[]){
    if (gfx_init()){
        exit(3);
    }

    bool running = 1;
    int gameState[CONTAINER_HEIGHT][CONTAINER_WIDTH];

    initGame();

    do{
        char key = gfx_pollkey();
        if (key == 'q'){
            running = !running;
        }
        drawScreen();
        gfx_updateScreen();
        SDL_Delay(FRAME_DELAY_MS);
    }while(running);

    return 0;
}
