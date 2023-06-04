.DEFAULT_GOAL = all

CC = clang

BIN_DIR = bin

HDRS = $(wildcard *.h) ../wfc.h
SRCS = $(wildcard *.c)

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -I../
LINK_FLAGS = -lm `sdl2-config --cflags --libs` -lSDL2_image

all: build

build: $(BIN_DIR)/main

$(BIN_DIR)/main: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -O2 $(SRCS) -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
