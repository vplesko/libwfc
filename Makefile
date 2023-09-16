.DEFAULT_GOAL = ui

CC = clang
CXX = clang++

BIN_DIR = bin

ifdef WIN
	STBI_OBJ = stb_image_impl.obj
	CLI_EXE = cli.exe
	GUI_EXE = gui.exe
else
	STBI_OBJ = stb_image_impl.o
	CLI_EXE = cli
	GUI_EXE = gui
endif

LIB_HDRS = $(wildcard external/lib/*.h)
HDRS = $(wildcard *.h shared/*.h) $(LIB_HDRS)
CLI_HDRS = $(wildcard cli/*.h)
GUI_HDRS = $(wildcard gui/*.h)
TEST_HDRS = $(wildcard test/*.h)
BENCHMARK_HDRS = $(wildcard benchmark/*.h)

BUILD_FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -I. -Ishared -Iexternal/lib -g -fno-omit-frame-pointer
BUILD_FLAGS_CXX = -std=c++11 -Wall -Wextra -pedantic -Werror -I. -Ishared -Iexternal/lib -g -fno-omit-frame-pointer
ifdef VC
	BUILD_FLAGS += -D_CRT_SECURE_NO_WARNINGS
endif
BUILD_FLAGS_DBG = -O2
BUILD_FLAGS_REL = -O3 -mavx2

LINK_FLAGS =
ifndef VC
	LINK_FLAGS += -lm
endif

$(BIN_DIR)/lib/$(STBI_OBJ): shared/stb_image_impl.c $(LIB_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -c -o $@

ui: cli gui

cli: $(BIN_DIR)/$(CLI_EXE)

$(BIN_DIR)/$(CLI_EXE): cli/main.c $(HDRS) $(CLI_HDRS) $(BIN_DIR)/lib/$(STBI_OBJ)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -o $@ $(LINK_FLAGS) $(BIN_DIR)/lib/$(STBI_OBJ)

gui: $(BIN_DIR)/$(GUI_EXE)

GUI_LINK_FLAGS = `sdl2-config --cflags --libs`
ifdef VC
	GUI_LINK_FLAGS += -Xlinker /subsystem:console -lshell32
endif

$(BIN_DIR)/$(GUI_EXE): gui/main.c $(HDRS) $(GUI_HDRS) $(BIN_DIR)/lib/$(STBI_OBJ)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -o $@ $(LINK_FLAGS) $(GUI_LINK_FLAGS) $(BIN_DIR)/lib/$(STBI_OBJ)

test: $(BIN_DIR)/test/done.txt

# MSan and Valgrind don't work on Windows, so skip them in that case.
TEST_MSAN_PATH =
TEST_VALGRIND_CMD =
ifndef WIN
	TEST_MSAN_PATH = $(BIN_DIR)/test/test_msan
	TEST_VALGRIND_CMD = valgrind -q --leak-check=yes $(BIN_DIR)/test/test
endif

$(BIN_DIR)/test/done.txt: $(BIN_DIR)/test/test $(BIN_DIR)/test/test_asan $(TEST_MSAN_PATH) $(BIN_DIR)/test/test_multi $(BIN_DIR)/test/test_cpp
	$(BIN_DIR)/test/test
	$(BIN_DIR)/test/test_asan
	$(TEST_MSAN_PATH)
	$(TEST_VALGRIND_CMD)
	$(BIN_DIR)/test/test_multi
	$(BIN_DIR)/test/test_cpp
	@touch $@

$(BIN_DIR)/test/test: test/test.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/test/test_asan: test/test.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion -fsanitize=address,undefined $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/test/test_msan: test/test.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion -fsanitize=memory -fsanitize-memory-track-origins -fPIE -pie $< -o $@ $(LINK_FLAGS)

$(BIN_DIR)/test/test_multi: test/test_multi1.c test/test_multi2.c $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_DBG) -Wconversion test/test_multi1.c test/test_multi2.c -o $@ $(LINK_FLAGS)

$(BIN_DIR)/test/test_cpp: test/test.cpp $(HDRS) $(TEST_HDRS)
	@mkdir -p $(@D)
	$(CXX) $(BUILD_FLAGS_CXX) $(BUILD_FLAGS_DBG) -Wconversion $< -o $@ $(LINK_FLAGS)

benchmark: $(BIN_DIR)/benchmark/done.txt

$(BIN_DIR)/benchmark/done.txt: $(BIN_DIR)/benchmark/main
	$(BIN_DIR)/benchmark/main
	@touch $@

$(BIN_DIR)/benchmark/main: benchmark/main.c $(HDRS) $(BENCHMARK_HDRS) $(BIN_DIR)/lib/$(STBI_OBJ)
	@mkdir -p $(@D)
	$(CC) $(BUILD_FLAGS) $(BUILD_FLAGS_REL) $< -o $@ $(LINK_FLAGS) $(BIN_DIR)/lib/$(STBI_OBJ)

clean:
	rm -rf $(BIN_DIR)
