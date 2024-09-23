CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Wno-unused-parameter
LIBS=$(shell pkg-config --cflags --libs sdl2) -lm -mconsole

te: main.c
	$(CC) -o te main.c ./src/la/la.c $(CFLAGS) $(LIBS) -msse3
