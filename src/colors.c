#include "colors.h"

void colors_initialize() {
    start_color();
    use_default_colors();
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_BLUE, 0, 0, 850);
    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_pair(COLORSCHEME_DEFAULT,  COLOR_WHITE,    COLOR_BLACK);
    init_pair(COLORSCHEME_YB,       COLOR_YELLOW,   COLOR_BLUE);
    init_pair(COLORSCHEME_BY,       COLOR_BLUE,     COLOR_YELLOW);
}
