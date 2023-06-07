.DEFAULT_GOAL = all

CC = clang

BIN_DIR = bin

HDRS = $(wildcard *.h) ../wfc.h

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -fno-omit-frame-pointer -O3 -mavx2 -I../ -I../external/lib
LINK_FLAGS = -lm

all: $(BIN_DIR)/cli $(BIN_DIR)/gui

$(BIN_DIR)/cli: main_cli.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/gui: main_gui.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $< -o $@ $(LINK_FLAGS) `sdl2-config --cflags --libs` -lSDL2_image

clean:
	rm -rf $(BIN_DIR)
