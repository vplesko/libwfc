.DEFAULT_GOAL = all

CC = clang

BIN_DIR = bin

HDRS = $(wildcard *.h) ../wfc.h
SRCS = $(wildcard *.c)

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -I../
LINK_FLAGS = -lm `sdl2-config --cflags --libs` -lSDL2_image

all: build build_asan

build: $(BIN_DIR)/main

$(BIN_DIR)/main: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -O2 $(SRCS) -o $@ $(LINK_FLAGS)

build_asan: $(BIN_DIR)/main_asan

# MSan requires that libs are compiled with it and -fsanitize-ignorelist doesn't seem able to filter .so files
$(BIN_DIR)/main_asan: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -O1 -fsanitize=address,undefined -fno-omit-frame-pointer $(SRCS) -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
