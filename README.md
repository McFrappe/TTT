# TTT
TTT, Terminal Text TV, is a terminal version of Text TV written in C.

Uses the [Text TV API](https://texttv.nu/blogg/texttv-api).

## Dependencies
* [libcurl](https://curl.se/docs/install.html)
* [jsmn](https://github.com/zserge/jsmn)
* [ncurses](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html)

## Usage
```
make run        # run the program
make memrun     $ run the program with valgrind

make test       # run tests
make memtest    # run tests with valgrind

make clean      # removes all compiled files
```
