# Makefile for Maze Escape with Intelligent Enemies
# Override paths as needed, for example:
#   make CC=gcc RAYLIB=/opt/raylib

ifeq ($(OS),Windows_NT)
ifeq ($(origin CC),default)
CC = C:/w64devkit_temp/w64devkit/bin/gcc
endif
RAYLIB ?= C:/raylib/raylib
W64BIN ?= C:/w64devkit_temp/w64devkit/bin
TARGET ?= MazeEscape.exe
RUN_PREFIX ?=
else
ifeq ($(origin CC),default)
CC = gcc
endif
RAYLIB ?= /usr/local
W64BIN ?=
TARGET ?= MazeEscape
RUN_PREFIX ?= ./
endif

RAYLIB_INCLUDE ?= $(RAYLIB)/include
RAYLIB_LIB ?= $(RAYLIB)/lib
BUILD_DIR ?= build
SAVE_DIR ?= saves
TEST_TARGET ?= $(BUILD_DIR)/maze_tests
PERSISTENCE_TEST_TARGET ?= $(BUILD_DIR)/persistence_tests
SANITIZER_TEST_TARGET ?= $(BUILD_DIR)/maze_tests_sanitize
SANITIZER_PERSISTENCE_TARGET ?= $(BUILD_DIR)/persistence_tests_sanitize
SANITIZER_FLAGS ?= -fsanitize=address,undefined -fno-omit-frame-pointer -g

ifeq ($(OS),Windows_NT)
TEST_TARGET := $(TEST_TARGET).exe
PERSISTENCE_TEST_TARGET := $(PERSISTENCE_TEST_TARGET).exe
SANITIZER_TEST_TARGET := $(SANITIZER_TEST_TARGET).exe
SANITIZER_PERSISTENCE_TARGET := $(SANITIZER_PERSISTENCE_TARGET).exe
MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR) & if not exist $(SAVE_DIR) mkdir $(SAVE_DIR)
CLEAN = if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR) & if exist $(TARGET) del /Q $(TARGET)
else
MKDIR = mkdir -p $(BUILD_DIR) $(SAVE_DIR)
CLEAN = rm -rf $(BUILD_DIR) $(TARGET)
endif

CFLAGS ?= -Wall -Wextra -std=c99 -O2
CPPFLAGS ?= -Iinclude -I$(RAYLIB_INCLUDE)
LDFLAGS ?= -L$(RAYLIB_LIB)
LDLIBS ?= -lraylib

ifeq ($(OS),Windows_NT)
LDLIBS += -lopengl32 -lgdi32 -lwinmm
endif
ifneq ($(strip $(W64BIN)),)
CFLAGS += -B$(W64BIN)
LDFLAGS += -B$(W64BIN)
endif

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)
TEST_PROD_SRC = $(filter-out src/main.c,$(SRC))

.PHONY: all clean run test test-sanitize

all: build $(TARGET)

build:
	$(MKDIR)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)
	@echo === Build successful: $(TARGET) ===

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): tests/test_maze_pathfinding.c src/maze.c src/pathfinding.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(PERSISTENCE_TEST_TARGET): tests/test_persistence.c $(TEST_PROD_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_persistence.c $(TEST_PROD_SRC) -o $@ $(LDFLAGS) $(LDLIBS)

test: build $(TEST_TARGET) $(PERSISTENCE_TEST_TARGET)
	$(RUN_PREFIX)$(TEST_TARGET)
	$(RUN_PREFIX)$(PERSISTENCE_TEST_TARGET)

test-sanitize: build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZER_FLAGS) tests/test_maze_pathfinding.c src/maze.c src/pathfinding.c -o $(SANITIZER_TEST_TARGET)
	$(RUN_PREFIX)$(SANITIZER_TEST_TARGET)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SANITIZER_FLAGS) tests/test_persistence.c $(TEST_PROD_SRC) -o $(SANITIZER_PERSISTENCE_TARGET) $(LDFLAGS) $(LDLIBS)
	$(RUN_PREFIX)$(SANITIZER_PERSISTENCE_TARGET)

run: all
	$(RUN_PREFIX)$(TARGET)

clean:
	$(CLEAN)
	@echo Cleaned.
