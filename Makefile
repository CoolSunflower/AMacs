CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Wall -Wno-implicit-function-declaration -Wno-error=incompatible-pointer-types -Wno-error

# Detect OS and set appropriate libs
ifeq ($(OS),Windows_NT)
	# Windows with MinGW
	LIBS=-I./dependencies/SDL2/include -L./dependencies/SDL2/lib -Dmain=SDL_main -lmingw32 -mwindows -lSDL2main -lSDL2 -lm -mconsole
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		# Linux
		LIBS=-I./dependencies/SDL2/include -L./dependencies/SDL2/lib -lSDL2main -lSDL2 -lm
	endif
	ifeq ($(UNAME_S),Darwin)
		# macOS
		LIBS=-I./dependencies/SDL2/include -L./dependencies/SDL2/lib -lSDL2main -lSDL2 -lm
	endif
endif

te: src/main.c
	$(CC) -o amacs src/main.c src/la.c src/editor.c $(CFLAGS) $(LIBS) -msse3