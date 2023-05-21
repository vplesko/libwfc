CC = clang

BIN_DIR = bin

build:
	mkdir -p $(BIN_DIR)
	$(CC) -O2 -std=c99 -Wall -Wextra -pedantic -Werror `sdl2-config --cflags --libs` main.c -o $(BIN_DIR)/main

clean:
	rm -rf $(BIN_DIR)
