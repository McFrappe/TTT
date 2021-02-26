![Tests](https://github.com/McFrappe/TTT/workflows/Tests/badge.svg)
![Formatted .c and .h files](https://github.com/McFrappe/TTT/workflows/Format/badge.svg)

# TTT
TTT, Terminal Text TV, is a terminal version of the Swedish teletext system
("Text-TV") written in C, relying on the [Text TV API](https://texttv.nu/blogg/texttv-api).

## Dependencies
* [libcurl](https://curl.se/docs/install.html)
* [ncurses](https://invisible-island.net/ncurses)

## Internal libraries
* [jsmn](https://github.com/zserge/jsmn) (included in the source)

## Installation
Install [libcurl](https://curl.se/docs/install.html) and [ncurses](https://invisible-island.net/ncurses)
if you do not already have them installed. These should be available in the repositories of your distribution - if not, refer to the links above.

### Official packages
Not yet available.

### Building from source
```
$ git clone https://github.com/McFrappe/TTT.git
$ cd TTT
$ sudo make install
```

The default installation path prefix is `/usr/local`, i.e. the program is
installed to `/usr/local/bin`. This can be changed by specifying another prefix:

```
$ sudo PREFIX="<your/prefix>" make install
```

## Usage
The program is launched with `ttt` (provided that the binary is included in your `$PATH`).

### Keybindings
All keybindings are listed in the help page of the program. This page can be
opened and closed using `?`.

### Arguments
* `-h` - display help message
* `-r` - restore terminal colors on quit (using the `reset` syscall)
* `-d` - do not overwrite terminal colors (uses your terminal colors instead)
* `-t` - use transparent background for pages (instead of black)

## Development usage
```
$ make run        # run the program
$ make memrun     # run the program with valgrind

$ make test       # run tests
$ make memtest    # run tests with valgrind

$ make clean      # removes all compiled files
```
