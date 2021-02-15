#include "colors.h"

void colors_initialize() {
    start_color();
    /* use_default_colors(); */
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_BLUE, 0, 0, 850);
    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_color(COLOR_CYAN, 0, 1000, 1000);
    init_color(COLOR_RED, 1000, 0, 0);
    init_pair(COLORSCHEME_DEFAULT,  COLOR_WHITE,    COLOR_BLACK);
    init_pair(COLORSCHEME_BW,       COLOR_BLACK,    COLOR_WHITE);
    init_pair(COLORSCHEME_WBL,      COLOR_WHITE,    COLOR_BLUE);
    init_pair(COLORSCHEME_YBL,      COLOR_YELLOW,   COLOR_BLUE);
    init_pair(COLORSCHEME_BLY,      COLOR_BLUE,     COLOR_YELLOW);
    init_pair(COLORSCHEME_YX,       COLOR_YELLOW,   COLOR_BLACK);
    init_pair(COLORSCHEME_CX,       COLOR_CYAN,     COLOR_BLACK);
    init_pair(COLORSCHEME_WR,       COLOR_WHITE,    COLOR_RED);
    init_pair(COLORSCHEME_RX,       COLOR_RED,      COLOR_BLACK);
}

attr_t colors_get_color_pair_from_style(page_token_style_t style) {
    if (
        (style.fg == PAGE_TOKEN_ATTR_YELLOW && style.bg == PAGE_TOKEN_ATTR_BG_YELLOW) ||
        (style.fg == PAGE_TOKEN_ATTR_BLUE && style.bg == PAGE_TOKEN_ATTR_BG_YELLOW)
    ) {
        return COLOR_PAIR(COLORSCHEME_BLY);
    } else if (
        (style.fg == PAGE_TOKEN_ATTR_BLUE || style.fg == PAGE_TOKEN_ATTR_YELLOW) &&
        (style.bg == PAGE_TOKEN_ATTR_BG_BLUE)
    ) {
        return COLOR_PAIR(COLORSCHEME_YBL);
    } else if (style.fg == PAGE_TOKEN_ATTR_CYAN && style.bg == PAGE_TOKEN_ATTR_BG_BLACK) {
        return COLOR_PAIR(COLORSCHEME_CX);
    } else if (style.fg == PAGE_TOKEN_ATTR_YELLOW && style.bg == PAGE_TOKEN_ATTR_BG_BLACK) {
        return COLOR_PAIR(COLORSCHEME_YX);
    } else if (style.fg == PAGE_TOKEN_ATTR_WHITE && style.bg == PAGE_TOKEN_ATTR_BG_BLUE) {
        return COLOR_PAIR(COLORSCHEME_WBL);
    } else if (style.fg == PAGE_TOKEN_ATTR_RED && style.bg == PAGE_TOKEN_ATTR_BG_RED) {
        return COLOR_PAIR(COLORSCHEME_WR);
    } else {
        return COLOR_PAIR(COLORSCHEME_DEFAULT);
    }
}
