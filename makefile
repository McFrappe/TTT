CC=gcc
CFLAGS=-Wall -pedantic -g
CFLAGS_LIB=-c
LIB_PATH=lib
OBJ_FILES=src/ui.c src/api.c src/parser.c
BUILD_DIR=build
BUILD_PATH=$(BUILD_DIR)/ttt.out

main: src/main.c $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_PATH)

%.o: %.o
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $^ -o $@

run: main
	$(BUILD_PATH)

clean:
	rm -f src/*.o
	rm -rf ./$(BUILD_DIR)
