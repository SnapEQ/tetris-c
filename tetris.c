#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "primlib.h"
#include "tetris.h"

#define SCREEN_W gfx_screenWidth()
#define SCREEN_H gfx_screenHeight()
#define CONTAINER_WIDTH 15
#define CONTAINER_HEIGHT 18
#define PIECE_SIZE 4
#define FRAME_DELAY_MS 16
#define NUMBER_OF_PIECES 7
#define FALL_COUNTER_MAXIMUM 30
#define BOARD_TOP_MARGIN 40
#define BOARD_BOTTOM_MARGIN 20
#define BOARD_SIDE_MARGIN 40
#define PANEL_GAP 24
#define PANEL_MIN_WIDTH 140
#define MIN_CELL_SIZE 8

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
int cellSize = MIN_CELL_SIZE;
int incomingPieceX = 0;
int incomingPieceY = 0;
int scoreX = 0;
int scoreY = 0;

static int minInt(int a, int b)
{
    return (a < b) ? a : b;
}

static int maxInt(int a, int b)
{
    return (a > b) ? a : b;
}

static void updateLayout()
{
    int availableHeight = SCREEN_H - BOARD_TOP_MARGIN - BOARD_BOTTOM_MARGIN;
    int availableWidth = SCREEN_W - 2 * BOARD_SIDE_MARGIN - 2 * PANEL_MIN_WIDTH - 2 * PANEL_GAP;

    cellSize = minInt(availableHeight / CONTAINER_HEIGHT, availableWidth / CONTAINER_WIDTH);
    cellSize = maxInt(cellSize, MIN_CELL_SIZE);

    int boardWidth = CONTAINER_WIDTH * cellSize;
    int boardHeight = CONTAINER_HEIGHT * cellSize;
    int boardTop = maxInt(BOARD_TOP_MARGIN, (SCREEN_H - boardHeight) / 2);

    board.x1 = (SCREEN_W - boardWidth) / 2;
    board.x2 = board.x1 + boardWidth - 1;
    board.y1 = boardTop;
    board.y2 = board.y1 + boardHeight - 1;

    scoreX = BOARD_SIDE_MARGIN;
    scoreY = board.y1 + 20;
    incomingPieceX = board.x2 + PANEL_GAP;
    incomingPieceY = board.y1 + cellSize;
}

static bool pieceFits(int nextX, int nextY, int nextRot)
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

static void getPiecePivot(int type, int rotation, int *pivotX, int *pivotY)
{
    for (int py = 0; py < PIECE_SIZE; py++)
    {
        for (int px = 0; px < PIECE_SIZE; px++)
        {
            if (pieces[type][rotation][py][px] == 2)
            {
                *pivotX = px;
                *pivotY = py;
                return;
            }
        }
    }

    *pivotX = 0;
    *pivotY = 0;
}

static void clearGameState()
{
    for (int y = 0; y < CONTAINER_HEIGHT; y++)
    {
        for (int x = 0; x < CONTAINER_WIDTH; x++)
        {
            board.board[y][x] = 0;
        }
    }
}

static void clearRow(int r)
{
    for (int x = 0; x < CONTAINER_WIDTH; x++)
    {
        board.board[r][x] = 0;
    }
}

static void moveRows(int r)
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

static void eraseFullRows(int *rowsCleared)
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

static void drawPreviewCell(int x, int y, enum color c)
{
    int x2 = x + cellSize - 1;
    int y2 = y + cellSize - 1;

    gfx_filledRect(x, y, x2, y2, c);
}

static void drawIncomingPiece()
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
            int screenX = incomingPieceX + px * cellSize;
            int screenY = incomingPieceY + py * cellSize;

            drawPreviewCell(screenX, screenY, color);
        }
    }
}

static void drawCell(int col, int row, enum color c)
{
    int x1 = board.x1 + col * cellSize;
    int y1 = board.y1 + row * cellSize;
    int x2 = x1 + cellSize - 1;
    int y2 = y1 + cellSize - 1;

    gfx_filledRect(x1, y1, x2, y2, c);
}

static void drawBoardTiles()
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

static void pickNewPiece()
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

static void drawCurrentPiece()
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

static void lockCurrentPiece()
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

static void lockAndRespawnPiece()
{
    lockCurrentPiece();
    pickNewPiece();
    if (!pieceFits(currentPiece.x, currentPiece.y, currentPiece.rotation))
    {
        gameOver = true;
    }
}

static void hardDropCurrentPiece()
{
    while (pieceFits(currentPiece.x, currentPiece.y + 1, currentPiece.rotation))
    {
        currentPiece.y++;
    }
    lockAndRespawnPiece();
}

static void drawBoard()
{
    gfx_rect(board.x1, board.y1, board.x2, board.y2, WHITE);
    gfx_line(board.x1, board.y1, board.x2, board.y1, BLACK);
}

static void drawScreen()
{
    gfx_filledRect(0, 0, SCREEN_W - 1, SCREEN_H - 1, BLACK);
    drawBoard();
    gfx_textout(incomingPieceX, board.y1, "Next", WHITE);
}

static void drawEndgameScreen()
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

static void handleInput(bool *running)
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
    if (key == SDLK_SPACE)
    {
        int nextRotation = (currentPiece.rotation + 1) % 4;
        int currentPivotX;
        int currentPivotY;
        int nextPivotX;
        int nextPivotY;
        int nextX;
        int nextY;

        getPiecePivot(currentPiece.type, currentPiece.rotation, &currentPivotX, &currentPivotY);
        getPiecePivot(currentPiece.type, nextRotation, &nextPivotX, &nextPivotY);

        nextX = currentPiece.x + currentPivotX - nextPivotX;
        nextY = currentPiece.y + currentPivotY - nextPivotY;

        if (pieceFits(nextX, nextY, nextRotation))
        {
            currentPiece.x = nextX;
            currentPiece.y = nextY;
            currentPiece.rotation = nextRotation;
        }
    }
    if (key == SDLK_DOWN)
    {
        hardDropCurrentPiece();
    }
}

static void updateScore(int rows)
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

    case 4:
        score += 1200;
        break;

    }

    }

static void displayScore()
{
    char str[20];
    snprintf(str, sizeof(str), "Score: %d", score);
    gfx_textout(scoreX, scoreY, str, WHITE);
}

void initGame()
{
    updateLayout();

    clearGameState();

    incomingPiece.type = rand() % NUMBER_OF_PIECES;
    incomingPiece.rotation = 0;
    pickNewPiece();
    gameOver = false;
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
        SDL_Delay(FRAME_DELAY_MS);
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
    SDL_Delay(FRAME_DELAY_MS);
}
