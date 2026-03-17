#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "piece.c"
#include "primlib.h"

#define SCREEN_W gfx_screenWidth()
#define SCREEN_H gfx_screenHeight()
#define CONTAINER_WIDTH 15
#define CONTAINER_HEIGHT 20
#define PIECE_SIZE 4
#define FRAME_DELAY_MS 16
#define CELL_SIZE 26
#define NUMBER_OF_PIECES 7


typedef struct{
    int type;
    int x;
    int y;
    int rotation;
} Piece;

typedef struct{
    char board[CONTAINER_HEIGHT][CONTAINER_WIDTH];
    int x1;
    int y1;
    int x2;
    int y2;
} Board;

Board board;
Piece currentPiece;
Piece incomingPiece;

void pickNewPiece(void){
    currentPiece = incomingPiece;
    incomingPiece.type = rand() % NUMBER_OF_PIECES;
    incomingPiece.rotation = 0;
}

void drawBoard(void){
    gfx_rect(board.x1, board.y1, board.x2, board.y2, WHITE);
    gfx_line(board.x1, board.y1, board.x2, board.y1, BLACK);
}

void drawScreen(void){
   gfx_filledRect(0, 0, SCREEN_W- 1, SCREEN_H - 1, BLACK);
    drawBoard();
}

void populateGameState(){
    for (int i = 0; i < CONTAINER_HEIGHT; i++){
        for (int j = 0; j < CONTAINER_WIDTH; j++){
            board.board[i][j] = 0;
        }
    }
}

void initGame(void){
    board.x1 = SCREEN_W/2 - CONTAINER_WIDTH*CELL_SIZE/2;
    board.x2 = SCREEN_W/2 + CONTAINER_WIDTH*CELL_SIZE/2;
    board.y1 = SCREEN_H - CONTAINER_HEIGHT*CELL_SIZE;
    board.y2 = SCREEN_H - 1;
    populateGameState();
}

void printGameState(void){
    for (int i = 0; i < CONTAINER_HEIGHT; i++){
        for (int j = 0; j < CONTAINER_WIDTH; j++){
            printf("%d", atoi(&board.board[i][j]));
        }
        printf("\n");
    }
    system("clear");
}

int main(int argc, char *argv[]){
    if (gfx_init()){
        exit(3);
    }

    bool running = 1;
    srand(time(NULL));


    initGame();

    do{
        char key = gfx_pollkey();
        if (key == 'q'){
            running = !running;
        }
        printGameState();
        drawScreen();
        gfx_updateScreen();
        SDL_Delay(16);
    }while(running);

    return 0;
}
