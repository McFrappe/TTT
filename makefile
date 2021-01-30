CC=gcc
CFLAGS=-Wall -pedantic -g
CFLAGS_LIB=-c
LIBS:=-lcurl -lncurses
TEST_LIBS:=-lcunit $(LIBS)
OBJ_FILES:=src/ui.o src/api.o src/parser.o src/pages.o
MAIN_FILES:=src/main.c $(OBJ_FILES)
TEST_FILES:=test/unittests.c $(OBJ_FILES)
DIST_DIR=build
TTT_OUT_PATH=$(DIST_DIR)/ttt.out
TEST_OUT_PATH=$(DIST_DIR)/tests.out
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --suppressions=static/valgrind.supp

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $(LIBS) $^ -o $@

main: src/main.c $(OBJ_FILES)
	$(CC) $(CFLAGS) $(LIBS) $(MAIN_FILES) -o $(TTT_OUT_PATH)

unittests: prebuild test/unittests.c $(OBJ_FILES)
	$(CC) $(CFLAGS) $(TEST_LIBS) $(TEST_FILES) -o $(TEST_OUT_PATH)

run: prebuild main
	./$(TTT_OUT_PATH)

memrun: main
	valgrind $(VALGRIND_FLAGS) ./$(TTT_OUT_PATH)

test: unittests
	./$(TEST_OUT_PATH)

memtest: unittests
	valgrind $(VALGRIND_FLAGS) ./$(TEST_OUT_PATH)

prebuild:
	mkdir -p $(DIST_DIR)

clean:
	rm -f src/*.o
	rm -rf ./$(DIST_DIR)
