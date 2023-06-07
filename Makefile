.DEFAULT_GOAL = test

CC = clang

BIN_DIR = bin

HDRS = $(wildcard *.h external/lib/*.h)
TEST_HDRS = $(wildcard test/*.h)
UI_HDRS = $(wildcard ui/*.h)
BENCHMARK_HDRS = $(wildcard benchmark/*.h)

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -I./ -Iexternal/lib -g -fno-omit-frame-pointer
BUILD_FLAGS_DBG = -O1
BUILD_FLAGS_REL = -O3 -mavx2
LINK_FLAGS = -lm

test: $(BIN_DIR)/test/done.txt

$(BIN_DIR)/test/done.txt: $(BIN_DIR)/test/main $(BIN_DIR)/test/main_asan $(BIN_DIR)/test/main_msan
	$(BIN_DIR)/test/main
	$(BIN_DIR)/test/main_asan
	$(BIN_DIR)/test/main_msan
	valgrind -q --leak-check=yes $(BIN_DIR)/test/main
	@touch $@

$(BIN_DIR)/test/main: test/main.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/test/main_asan: test/main.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion -fsanitize=address,undefined $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/test/main_msan: test/main.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion -fsanitize=memory -fsanitize-memory-track-origins -fPIE -pie $< -o $@ $(LINK_FLAGS)

benchmark: $(BIN_DIR)/benchmark/done.txt

$(BIN_DIR)/benchmark/done.txt: $(BIN_DIR)/benchmark/main
	$(BIN_DIR)/benchmark/main
	@touch $@

$(BIN_DIR)/benchmark/main: benchmark/main.c $(HDRS) $(BENCHMARK_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -o $@ $(LINK_FLAGS)

ui: cli gui

cli: $(BIN_DIR)/ui/cli

$(BIN_DIR)/ui/cli: ui/main_cli.c $(HDRS) $(UI_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -o $@ $(LINK_FLAGS)

gui: $(BIN_DIR)/ui/gui

$(BIN_DIR)/ui/gui: ui/main_gui.c $(HDRS) $(UI_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -o $@ $(LINK_FLAGS) `sdl2-config --cflags --libs` -lSDL2_image

clean:
	rm -rf $(BIN_DIR)
