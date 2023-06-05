.DEFAULT_GOAL = test

CC = clang

BIN_DIR = bin
TEST_RECORD = tested.txt

HDRS = $(wildcard *.h) ../wfc.h

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -fno-omit-frame-pointer -O3 -mavx2 -I../ -I../external/lib
LINK_FLAGS = -lm

test: $(BIN_DIR)/$(TEST_RECORD)

$(BIN_DIR)/$(TEST_RECORD): $(BIN_DIR)/main
	$(BIN_DIR)/main
	@touch $@

$(BIN_DIR)/main: main.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $< -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
