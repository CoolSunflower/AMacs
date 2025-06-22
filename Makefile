CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Wno-implicit-function-declaration -Wno-error

# Detect operating system properly
UNAME_S := $(shell uname -s 2>/dev/null || echo "Windows")

# Set platform-specific flags
ifeq ($(OS),Windows_NT)
    # Windows with MinGW
    INCLUDES=-I./dependencies/SDL2/include
    LDFLAGS=-L./dependencies/SDL2/lib
    LIBS=-Dmain=SDL_main -lmingw32 -lSDL2main -lSDL2 -lm
    PLATFORM_FLAGS=-mwindows -mconsole
else ifeq ($(UNAME_S),Linux)
    # Linux - use system SDL2
    INCLUDES=$(shell pkg-config --cflags sdl2 2>/dev/null || echo "-I./dependencies/SDL2/include")
    LDFLAGS=$(shell pkg-config --libs sdl2 2>/dev/null || echo "-L./dependencies/SDL2/lib -lSDL2")
    LIBS=-lm
    PLATFORM_FLAGS=
else ifeq ($(UNAME_S),Darwin)
    # macOS - use system SDL2 or Homebrew
    INCLUDES=$(shell pkg-config --cflags sdl2 2>/dev/null || echo "-I/opt/homebrew/include/SDL2 -I/usr/local/include/SDL2")
    LDFLAGS=$(shell pkg-config --libs sdl2 2>/dev/null || echo "-L/opt/homebrew/lib -L/usr/local/lib -lSDL2")
    LIBS=-lm
    PLATFORM_FLAGS=-framework Cocoa
endif

te: src/main.c src/la.c src/editor.c
	$(CC) $(CFLAGS) $(INCLUDES) -o amacs src/main.c src/la.c src/editor.c $(LDFLAGS) $(LIBS) $(PLATFORM_FLAGS) -msse3

clean:
	rm -f amacs amacs.exe

.PHONY: clean te