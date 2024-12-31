#!/bin/sh

set -xe

CC="${CXX:-cc}"
PKGS="sdl2 glew freetype2"
CFLAGS="-Wall -Wextra -std=c11 -pedantic -ggdb"
LIBS=-lm
SRC="src/main.c src/la.c"

if [ `uname` = "Darwin" ]; then
    CFLAGS+=" -framework OpenGL"
fi

$CC $CFLAGS `pkg-config --cflags $PKGS` -o ./build/amacs $SRC $LIBS `pkg-config --libs $PKGS`