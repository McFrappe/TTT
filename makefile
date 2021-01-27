CC=gcc
CFLAGS=-Wall -pedantic -g
CFLAGS_LIB=-c
CURL_LIB=-lcurl
LIB_PATH=lib
OBJ_FILES=src/ui.c src/api.c src/parser.c
BUILD_DIR=build
BUILD_PATH=$(BUILD_DIR)/ttt.out

main: src/main.c $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ $(CURL_LIB) -o $(BUILD_PATH)

%.o: %.o
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $^ -o $@

run: main
	$(BUILD_PATH)

memtest: main
	valgrind --leak-check=full --show-leak-kinds=all ./build/ttt.out
	
clean:
	rm -f src/*.o
	rm -rf ./$(BUILD_DIR)
