.DEFAULT_GOAL = all

CC = clang

BIN_DIR = bin

HDRS = $(wildcard *.h) ../wfc.h

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -fno-omit-frame-pointer -O3 -mavx2 -I../ -I../external/lib
LINK_FLAGS = -lm

all: $(BIN_DIR)/demo_sdl $(BIN_DIR)/demo_stbi

$(BIN_DIR)/demo_sdl: demo_sdl.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $< -o $@ $(LINK_FLAGS) `sdl2-config --cflags --libs` -lSDL2_image

 $(BIN_DIR)/demo_stbi: demo_stbi.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $< -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
