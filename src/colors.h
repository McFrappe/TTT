#pragma once
#include <curses.h>

// Color pair 0 is reserved for the system:
// https://linux.die.net/man/3/color_pair
typedef enum colorschemes {
    COLORSCHEME_SYSTEM,
    COLORSCHEME_DEFAULT,    // WHITE-BLACK
    COLORSCHEME_BW,         // BLACK-WHITE
    COLORSCHEME_BY,         // BLUE-YELLOW
    COLORSCHEME_YB,         // YELLOW-BLUE
    COLORSCHEME_YX,         // YELLOW-DEFAULT
    // TODO: Add colorscheme for each colorscheme in response (docs/parser/html.md)
} colorschemes_t;

void colors_initialize();
