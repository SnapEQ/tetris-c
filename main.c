#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "piece.c"
#include "primlib.h"

#define SCREEN_W gfx_screenWidth()
#define SCREEN_H gfx_screenHeight()
#define CONTAINER_WIDTH 12
#define CONTAINER_HEIGHT 14
#define PIECE_SIZE 4
#define FRAME_DELAY_MS 16
#define CELL_SIZE 36
#define NUMBER_OF_PIECES 7
#define FALL_COUNTER_MAXIMUM 30
#define INCOMINGPIECE_X SCREEN_W / 2 + CONTAINER_WIDTH *CELL_SIZE / 2 + 2 * CELL_SIZE
#define INCOMINGPIECE_Y SCREEN_H - 4 * CELL_SIZE
#define SCORE_X SCREEN_W / 10
#define SCORE_Y SCREEN_H / 10

typedef struct
{
    int type;
    int x;
    int y;
    int rotation;
} Piece;

typedef struct
{
    int board[CONTAINER_HEIGHT][CONTAINER_WIDTH];
    int x1;
    int y1;
    int x2;
    int y2;
} Board;

Board board;
Piece currentPiece;
Piece incomingPiece;
int score = 0;
bool gameOver = false;

bool pieceFits(int nextX, int nextY, int nextRot)
{
    for (int py = 0; py < PIECE_SIZE; py++)
    {
        for (int px = 0; px < PIECE_SIZE; px++)
        {
            if (pieces[currentPiece.type][nextRot][py][px] == 0)
            {
                continue;
            }

            int boardX = nextX + px;
            int boardY = nextY + py;

            if (boardX < 0 || boardX >= CONTAINER_WIDTH ||
                boardY < 0 || boardY >= CONTAINER_HEIGHT)
            {
                return false;
            }

            if (board.board[boardY][boardX] != 0)
            {
                return false;
            }
        }
    }
    return true;
}

void clearGameState(void)
{
    for (int y = 0; y < CONTAINER_HEIGHT; y++)
    {
        for (int x = 0; x < CONTAINER_WIDTH; x++)
        {
            board.board[y][x] = 0;
        }
    }
}

void clearRow(int r)
{
    for (int x = 0; x < CONTAINER_WIDTH; x++)
    {
        board.board[r][x] = 0;
    }
}

void moveRows(int r)
{
    for (int y = r; y > 0; y--)
    {
        for (int x = 0; x < CONTAINER_WIDTH; x++)
        {
            int prev = board.board[y][x];
            board.board[y][x] = board.board[y - 1][x];
            board.board[y - 1][x] = prev;
        }
    }
}

void eraseFullRows(int *rowsCleared)
{
    for (int y = 0; y < CONTAINER_HEIGHT; y++)
    {
        bool isFullRow = true;
        for (int x = 0; x < CONTAINER_WIDTH; x++)
        {
            if (board.board[y][x] != 3)
            {
                isFullRow = false;
            }
        }
        if (isFullRow)
        {
            (*rowsCleared)++;
            clearRow(y);
            moveRows(y);
        }
    }
}

void drawPreviewCell(int x, int y, enum color c)
{
    int x2 = x + CELL_SIZE - 1;
    int y2 = y + CELL_SIZE - 1;

    gfx_filledRect(x, y, x2, y2, c);
}

void drawIncomingPiece(void)
{
    for (int py = 0; py < PIECE_SIZE; py++)
    {
        for (int px = 0; px < PIECE_SIZE; px++)
        {
            char value = pieces[incomingPiece.type][incomingPiece.rotation][py][px];

            if (value == 0)
            {
                continue;
            }

            enum color color = (value == 2) ? YELLOW : GREEN;
            int screenX = INCOMINGPIECE_X + px * CELL_SIZE;
            int screenY = INCOMINGPIECE_Y + py * CELL_SIZE;

            drawPreviewCell(screenX, screenY, color);
        }
    }
}

void drawCell(int col, int row, enum color c)
{
    int x1 = board.x1 + col * CELL_SIZE;
    int y1 = board.y1 + row * CELL_SIZE;
    int x2 = x1 + CELL_SIZE - 1;
    int y2 = y1 + CELL_SIZE - 1;

    gfx_filledRect(x1, y1, x2, y2, c);
}

void drawBoardTiles(void)
{
    for (int y = 0; y < CONTAINER_HEIGHT; y++)
    {
        for (int x = 0; x < CONTAINER_WIDTH; x++)
        {
            if (board.board[y][x] == 3)
            {
                drawCell(x, y, RED);
            }
        }
    }
}

void spawnPiece(void)
{
    currentPiece.type = rand() % NUMBER_OF_PIECES;
    currentPiece.rotation = 0;
    currentPiece.x = CONTAINER_WIDTH / 2 - 2;
    currentPiece.y = 0;
}

// TODO refactor for one function

void drawCurrentPiece(void)
{
    for (int py = 0; py < PIECE_SIZE; py++)
    {
        for (int px = 0; px < PIECE_SIZE; px++)
        {
            char value = pieces[currentPiece.type][currentPiece.rotation][py][px];

            if (value == 0)
            {
                continue;
            }

            enum color color = (value == 2) ? YELLOW : GREEN;
            drawCell(currentPiece.x + px, currentPiece.y + py, color);
        }
    }
}

void pickNewPiece(void)
{
    currentPiece = incomingPiece;
    currentPiece.x = CONTAINER_WIDTH / 2 - 2;
    currentPiece.y = 0;
    currentPiece.rotation = 0;

    incomingPiece.type = rand() % NUMBER_OF_PIECES;
    incomingPiece.rotation = 0;
    incomingPiece.x = 0;
    incomingPiece.y = 0;
}

void lockCurrentPiece(void)
{
    for (int py = 0; py < PIECE_SIZE; py++)
    {
        for (int px = 0; px < PIECE_SIZE; px++)
        {
            if (pieces[currentPiece.type][currentPiece.rotation][py][px] != 0)
            {
                int boardX = currentPiece.x + px;
                int boardY = currentPiece.y + py;
                board.board[boardY][boardX] = 3;
            }
        }
    }
}

void lockAndRespawnPiece(void)
{
    lockCurrentPiece();
    pickNewPiece();
    if (!pieceFits(currentPiece.x, currentPiece.y, currentPiece.rotation))
    {
        gameOver = true;
    }
}

void hardDropCurrentPiece(void)
{
    while (pieceFits(currentPiece.x, currentPiece.y + 1, currentPiece.rotation))
    {
        currentPiece.y++;
    }
    lockAndRespawnPiece();
}

void drawBoard(void)
{
    gfx_rect(board.x1, board.y1, board.x2, board.y2, WHITE);
    gfx_line(board.x1, board.y1, board.x2, board.y1, BLACK);
}

void drawScreen(void)
{
    gfx_filledRect(0, 0, SCREEN_W - 1, SCREEN_H - 1, BLACK);
    drawBoard();
}

void drawEndgameScreen(void)
{
    int centerX = SCREEN_W / 2;
    int centerY = SCREEN_H / 2;

    gfx_filledRect(centerX - 220, centerY - 90, centerX + 220, centerY + 90, BLUE);
    gfx_rect(centerX - 220, centerY - 90, centerX + 220, centerY + 90, WHITE);
    gfx_textout(centerX - 45, centerY - 60, "GAME OVER", WHITE);

    char scoreText[40];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
    gfx_textout(centerX - 45, centerY - 20, scoreText, WHITE);
    gfx_textout(centerX - 165, centerY + 20, "Press Enter to restart", WHITE);
    gfx_textout(centerX - 130, centerY + 50, "Press Esc to quit", WHITE);
}

void populateGameState(void)
{
    for (int y = 0; y < CONTAINER_HEIGHT; y++)
    {
        for (int x = 0; x < CONTAINER_WIDTH; x++)
        {
            board.board[y][x] = 0;
        }
    }
}

void initGame(void)
{
    board.x1 = SCREEN_W / 2 - CONTAINER_WIDTH * CELL_SIZE / 2;
    board.x2 = SCREEN_W / 2 + CONTAINER_WIDTH * CELL_SIZE / 2;
    board.y1 = SCREEN_H - CONTAINER_HEIGHT * CELL_SIZE;
    board.y2 = SCREEN_H - 1;

    populateGameState();

    incomingPiece.type = rand() % NUMBER_OF_PIECES;
    incomingPiece.rotation = 0;
    pickNewPiece();
    gameOver = false;
}

void handleInput(bool *running)
{
    int key = gfx_pollkey();

    if (gameOver)
    {
        if (key == SDLK_ESCAPE)
        {
            *running = false;
        }
        if (key == SDLK_RETURN)
        {
            score = 0;
            clearGameState();
            initGame();
        }
        return;
    }

    if (key == SDLK_ESCAPE)
    {
        *running = false;
    }
    if (key == SDLK_LEFT && pieceFits(currentPiece.x - 1, currentPiece.y, currentPiece.rotation))
    {
        currentPiece.x--;
    }
    if (key == SDLK_RIGHT && pieceFits(currentPiece.x + 1, currentPiece.y, currentPiece.rotation))
    {
        currentPiece.x++;
    }
    if (key == SDLK_SPACE && pieceFits(currentPiece.x, currentPiece.y, (currentPiece.rotation + 1) % 4))
    {
        currentPiece.rotation = (currentPiece.rotation + 1) % 4;
    }
    if (key == SDLK_DOWN)
    {
        hardDropCurrentPiece();
    }
}

void updateScore(int rows)
{
    switch (rows)
    {
    case 1:
        score += 100;
        break;

    case 2:
        score += 300;
        break;

    case 3:
        score += 800;
        break;
    }
}

void displayScore(void)
{
    char str[20];
    sprintf(str, "%d", score);
    gfx_textout(SCORE_X, SCORE_Y, str, WHITE);
}

void gameLoop(bool *running, int *fallCounter)
{

    handleInput(running);

    if (gameOver)
    {
        drawScreen();
        drawBoardTiles();
        displayScore();
        drawEndgameScreen();
        gfx_updateScreen();
        SDL_Delay(16);
        return;
    }

    (*fallCounter)++;
    if (*fallCounter >= FALL_COUNTER_MAXIMUM)
    {
        if (pieceFits(currentPiece.x, currentPiece.y + 1, currentPiece.rotation))
        {
            currentPiece.y++;
        }
        else
        {
            lockAndRespawnPiece();
        }
        *fallCounter = 0;
    }

    int rowsCleared = 0;

    eraseFullRows(&rowsCleared);
    updateScore(rowsCleared);
    drawScreen();
    drawIncomingPiece();
    drawBoardTiles();
    drawCurrentPiece();
    displayScore();
    gfx_updateScreen();
    SDL_Delay(16);
}

int main(int argc, char *argv[])
{
    if (gfx_init())
    {
        exit(3);
    }

    bool running = true;
    int fallCounter = 0;

    srand(time(NULL));

    initGame();

    do
    {
        gameLoop(&running, &fallCounter);
    } while (running);

    return 0;
}
