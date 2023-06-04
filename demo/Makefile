.DEFAULT_GOAL = all

CC = clang

BIN_DIR = bin

HDRS = $(wildcard *.h) ../wfc.h

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -fno-omit-frame-pointer -I../
LINK_FLAGS = -lm `sdl2-config --cflags --libs` -lSDL2_image

all: $(BIN_DIR)/showcase

# @TODO optimal build
$(BIN_DIR)/showcase: showcase.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -O2 $< -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
