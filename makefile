CC=gcc
CFLAGS=-Wall -pedantic -g
CFLAGS_LIB=-c
CURL_LIB=-lcurl
NCURSES_LIB=-lncurses
OBJ_FILES=src/ui.c src/api.c src/parser.c
BUILD_DIR=build
BUILD_PATH=$(BUILD_DIR)/ttt.out
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --suppressions=static/valgrind.supp

main: src/main.c $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ $(CURL_LIB) $(NCURSES_LIB) -o $(BUILD_PATH)

%.o: %.o
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $^ -o $@

run: main
	$(BUILD_PATH)

memtest: main
	valgrind $(VALGRIND_FLAGS) ./build/ttt.out

clean:
	rm -f src/*.o
	rm -rf ./$(BUILD_DIR)
