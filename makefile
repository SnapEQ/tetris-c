
# Add -fsanitize=undefined

CC = gcc
CFLAGS = -g -Wall -pedantic $(shell pkg-config --cflags sdl2 SDL2_gfx)
LDLIBS = $(shell pkg-config --libs sdl2 SDL2_gfx) -lm

all: main

main: main.o tetris.o piece.o primlib.o
	$(CC) $^ -o $@ $(LDLIBS)

main.o: main.c primlib.h tetris.h
	$(CC) $(CFLAGS) -c main.c

tetris.o: tetris.c tetris.h primlib.h
	$(CC) $(CFLAGS) -c tetris.c

piece.o: piece.c
	$(CC) $(CFLAGS) -c piece.c

primlib.o: primlib.c primlib.h
	$(CC) $(CFLAGS) -c primlib.c

clean:
	rm -f main *.o
