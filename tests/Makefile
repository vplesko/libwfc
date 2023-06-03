# @TODO sanitizer tests
# @TODO print the rand seed if not ok, so easier if flaky

.DEFAULT_GOAL = test

CC = clang

BIN_DIR = bin
TEST_RECORD = tested.txt

HDRS = $(wildcard *.h) ../wfc.h
SRCS = $(wildcard *.c)

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -g -I../
LINK_FLAGS = -lm

test: $(BIN_DIR)/$(TEST_RECORD)

$(BIN_DIR)/$(TEST_RECORD): $(BIN_DIR)/main
	$(BIN_DIR)/main
	@touch $@

$(BIN_DIR)/main: $(HDRS) $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) -O2 $(SRCS) -o $@ $(LINK_FLAGS)

clean:
	rm -rf $(BIN_DIR)
