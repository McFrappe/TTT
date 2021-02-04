#include "colors.h"

void colors_initialize() {
    start_color();
    use_default_colors();
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_BLUE, 0, 0, 850);
    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_pair(COLORSCHEME_DEFAULT,  COLOR_WHITE,    COLOR_BLACK);
    init_pair(COLORSCHEME_BW,       COLOR_BLACK,    COLOR_WHITE);
    init_pair(COLORSCHEME_WBL,      COLOR_WHITE,    COLOR_BLUE);
    init_pair(COLORSCHEME_YBL,      COLOR_YELLOW,   COLOR_BLUE);
    init_pair(COLORSCHEME_BLY,      COLOR_BLUE,     COLOR_YELLOW);
    init_pair(COLORSCHEME_YX,       COLOR_YELLOW,   COLOR_BLACK);
}
