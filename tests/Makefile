.DEFAULT_GOAL = test

CC = clang

BIN_DIR = bin
TEST_RECORD = tested.txt

HDRS = $(wildcard *.h) ../wfc.h
SRCS = $(wildcard *.c)

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -fno-omit-frame-pointer -O1 -I../
LINK_FLAGS = -lm

test: $(BIN_DIR)/$(TEST_RECORD)

$(BIN_DIR)/$(TEST_RECORD): $(BIN_DIR)/main $(BIN_DIR)/main_asan $(BIN_DIR)/main_msan
	$(BIN_DIR)/main
	$(BIN_DIR)/main_asan
	$(BIN_DIR)/main_msan
	valgrind -q --leak-check=yes $(BIN_DIR)/main
	@touch $@

$(BIN_DIR)/main: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(SRCS) -o $@ $(LINK_FLAGS)

$(BIN_DIR)/main_asan: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -fsanitize=address,undefined $(SRCS) -o $@ $(LINK_FLAGS)

$(BIN_DIR)/main_msan: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -fsanitize=memory -fsanitize-memory-track-origins -fPIE -pie $(SRCS) -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
