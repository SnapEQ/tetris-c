#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "primlib.h"
#include "tetris.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

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
