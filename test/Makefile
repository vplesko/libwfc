.DEFAULT_GOAL = test

CC = clang

BIN_DIR = bin
TEST_RECORD = tested.txt

HDRS = $(wildcard *.h) ../wfc.h

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Wconversion -Werror -g -fno-omit-frame-pointer -O1 -I../
LINK_FLAGS = -lm

test: $(BIN_DIR)/$(TEST_RECORD)

$(BIN_DIR)/$(TEST_RECORD): $(BIN_DIR)/main $(BIN_DIR)/main_asan $(BIN_DIR)/main_msan
	$(BIN_DIR)/main
	$(BIN_DIR)/main_asan
	$(BIN_DIR)/main_msan
	valgrind -q --leak-check=yes $(BIN_DIR)/main
	@touch $@

$(BIN_DIR)/main: main.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/main_asan: main.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -fsanitize=address,undefined $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/main_msan: main.c $(HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -fsanitize=memory -fsanitize-memory-track-origins -fPIE -pie $< -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
