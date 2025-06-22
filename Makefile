CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Wall -Wno-implicit-function-declaration
LIBS=-I./dependencies/SDL2/include -L./dependencies/SDL2/lib -Dmain=SDL_main -lmingw32 -mwindows -lSDL2main -lSDL2 -lm -mconsole

te: src/main.c
	$(CC) -o amacs src/main.c src/la.c src/editor.c $(CFLAGS) $(LIBS) -msse3