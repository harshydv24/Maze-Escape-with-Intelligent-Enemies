# Makefile for Maze Escape with Intelligent Enemies
# Compiler: w64devkit GCC (or any MinGW-w64 GCC)
# Library:  raylib (installed at C:/raylib/raylib)
#
# If w64devkit is installed, prepend its bin/ to your PATH:
#   set PATH=C:\w64devkit_temp\w64devkit\bin;%PATH%

CC       = C:/w64devkit_temp/w64devkit/bin/gcc
RAYLIB   = C:/raylib/raylib
W64BIN   = C:/w64devkit_temp/w64devkit/bin

CFLAGS   = -B$(W64BIN) -Wall -Wextra -std=c99 -O2 -Iinclude -I$(RAYLIB)/include
LDFLAGS  = -B$(W64BIN) -L$(RAYLIB)/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC      = $(wildcard src/*.c)
OBJ      = $(SRC:src/%.c=build/%.o)
TARGET   = MazeEscape.exe

.PHONY: all clean run

all: build $(TARGET)

build:
	@if not exist build mkdir build
	@if not exist saves mkdir saves

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo === Build successful: $(TARGET) ===

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	@if exist build rmdir /S /Q build
	@if exist $(TARGET) del /Q $(TARGET)
	@echo Cleaned.
