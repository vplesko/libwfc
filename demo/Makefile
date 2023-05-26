.DEFAULT_GOAL = build

CC = clang

BIN_DIR = bin

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -I../
LINK_FLAGS = `sdl2-config --cflags --libs` -lSDL2_image

build:
	mkdir -p $(BIN_DIR)
	$(CC) $(BUILD_FLAGS) -O2 main.c -o $(BIN_DIR)/main $(LINK_FLAGS)

# MSan requires that libs are compiled with it and -fsanitize-ignorelist doesn't seem able to filter .so files
build_asan:
	mkdir -p $(BIN_DIR)
	$(CC) $(BUILD_FLAGS) -O1 -fsanitize=address,undefined -fno-omit-frame-pointer main.c -o $(BIN_DIR)/main $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
