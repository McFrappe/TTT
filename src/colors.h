#pragma once
#include <curses.h>

#include "pages.h"

// Color pair 0 is reserved for the system:
// https://linux.die.net/man/3/color_pair
typedef enum colorschemes {
    COLORSCHEME_SYSTEM,
    COLORSCHEME_DEFAULT,    // WHITE-BLACK
    COLORSCHEME_BW,         // BLACK-WHITE
    COLORSCHEME_WBL,        // WHITE-BLUE
    COLORSCHEME_BLY,        // BLUE-YELLOW
    COLORSCHEME_YBL,        // YELLOW-BLUE
    COLORSCHEME_YX,         // YELLOW-DEFAULT
    COLORSCHEME_CX,         // CYAN-DEFAULT
    COLORSCHEME_WR,         // WHITE-RED
    COLORSCHEME_RX,         // RED-BLACK
} colorschemes_t;

void colors_initialize(bool overwrite_colors, bool transparent_background);
attr_t colors_get_color_pair_from_style(page_token_style_t style);
