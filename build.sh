#!/usr/bin/env bash

DEBUG=1
SOURCE_FILES="src/joytext.c"
SETTINGS="-std=c89 -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wno-comment"
GUI_LIBS="`sdl2-config --cflags --libs` -framework OpenGL"
BUILD_DIRECTORY="build"

if [ ! -d "$BUILD_DIRECTORY" ]; then
    mkdir $BUILD_DIRECTORY;
fi

if [ $DEBUG -eq 0 ]; then
    echo "Optimized build";
    TARGET="-O2 -o build/joytext.exe"
elif [ $DEBUG -eq 1 ]; then
    echo "Debug build";
    TARGET="-g3 -O0 -o build/joytext.out"
fi

echo $TARGET
echo $SETTINGS
echo $GUI_LIBS
echo $SOURCE_FILES

gcc $TARGET $SETTINGS $GUI_LIBS $SOURCE_FILES
