CC					    = gcc
CFLAGS				  = -Wall -pedantic -g
CFLAGS_LIB		  = -c
CURL_LIB			  = -lcurl
LIB_PATH			  = lib
OBJ_FILES			  = src/ui.c src/api.c src/parser.c
BUILD_DIR			  = build
BUILD_PATH			= $(BUILD_DIR)/ttt.out
SHOW_ALL_KINDS	= --show-leak-kinds=all
SHOW_LEAK_CHECK	= --leak-check=full
NCURSES_LIB     =-lncurses

main: src/main.c $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ $(CURL_LIB) $(NCURSES_LIB) -o $(BUILD_PATH)

%.o: %.o
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $^ -o $@

run: main
	$(BUILD_PATH)

memtest: main
	valgrind $(SHOW_LEAK_CHECK) ./build/ttt.out
	
clean:
	rm -f src/*.o
	rm -rf ./$(BUILD_DIR)
