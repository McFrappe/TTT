CC=gcc
CFLAGS=-Wall -pedantic -g
CFLAGS_LIB=-c

LIBS=-lcurl -lncursesw
TEST_LIBS=-lcunit

BASE_OBJ_FILES:=src/parser.o src/pages.o src/errors.c
OBJ_FILES:=src/ui.o src/api.o src/draw.c src/colors.c $(BASE_OBJ_FILES)
MAIN_FILES:=src/main.c $(OBJ_FILES)
TEST_FILES:=test/unittests.c $(BASE_OBJ_FILES)

DIST_DIR=bin
TTT_OUT_PATH=$(DIST_DIR)/ttt
TEST_OUT_PATH=$(DIST_DIR)/ttt_tests

VALGRIND_FLAGS=--leak-check=full \
	       --show-leak-kinds=all \
	       --suppressions=static/valgrind.supp \
	       --error-exitcode=1 \
	       --track-origins=yes

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $(LIBS) $^ -o $@

main: prebuild $(MAIN_FILES)
	$(CC) $(CFLAGS) $(MAIN_FILES) -o $(TTT_OUT_PATH) $(LIBS)

unittests: prebuild $(TEST_FILES)
	$(CC) $(CFLAGS) $(TEST_FILES) -o $(TEST_OUT_PATH) $(TEST_LIBS)

run: main
	./$(TTT_OUT_PATH)

runr: main
	./$(TTT_OUT_PATH) -r

memrun: main
	valgrind $(VALGRIND_FLAGS) ./$(TTT_OUT_PATH)

memrunr: main
	valgrind $(VALGRIND_FLAGS) ./$(TTT_OUT_PATH) -r

test: unittests
	./$(TEST_OUT_PATH)

memtest: unittests
	valgrind $(VALGRIND_FLAGS) ./$(TEST_OUT_PATH)

prebuild:
	mkdir -p $(DIST_DIR)

clean:
	rm -f src/*.o
	rm -rf ./$(DIST_DIR)
