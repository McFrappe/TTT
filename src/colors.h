#pragma once
#include <curses.h>

// Color pair 0 is reserved for the system:
// https://linux.die.net/man/3/color_pair
typedef enum colorschemes {
    COLORSCHEME_SYSTEM,
    COLORSCHEME_DEFAULT,
    COLORSCHEME_HEADER,
    // TODO: Add colorscheme for each colorscheme in response (docs/parser/html.md)
} colorschemes_t;

void colors_initialize();
