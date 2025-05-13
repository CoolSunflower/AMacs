CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Wall
LIBS=-IC:/msys64/mingw64/include/SDL2 -Dmain=SDL_main -lmingw32 -mwindows -lSDL2main -lSDL2 -lm -mconsole

te: src/main.c
	$(CC) -o amacs src/main.c src/la.c $(CFLAGS) $(LIBS) -msse3