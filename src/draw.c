#include "draw.h"

static view_t current_view;

/// @brief Prints a T in a 3x3 box
void print_logo_letter(WINDOW *win, int line, int *col) {
    wattron(win, COLOR_PAIR(COLORSCHEME_BW) | A_UNDERLINE);
    wmove(win, line, *col);

    for (int i = 0; i < 6; i++) {
        waddch(win, ' ');
    }

    wmove(win, line + 1, (*col) + 2);

    for (int i = 0; i < 2; i++) {
        waddch(win, ' ');
    }

    wmove(win, line + 2, (*col) + PAGE_SIDE_PADDING_LG);
    waddch(win, ' ');
    wattroff(win, COLOR_PAIR(COLORSCHEME_BW) | A_UNDERLINE);
    *col += 3 + PAGE_SIDE_PADDING;
}

void fill_rows(WINDOW *win, int *line, int rows, attr_t attr) {
    wattron(win, attr);
    wmove(win, *line, PAGE_SIDE_PADDING);

    for (int i = 0; i < rows; i++) {
        wmove(win, (*line + i), 1);

        for (int j = 0; j < PAGE_COLS - PAGE_SIDE_PADDING; j++) {
            waddch(win, ' ');
        }
    }

    wattroff(win, attr);
    *line += rows;
}

void print_logo(WINDOW *win, int *line) {
    int col = PAGE_SIDE_PADDING_LG;
    fill_rows(win, line, 4, COLOR_PAIR(COLORSCHEME_YBL));
    *line -= 3;
    print_logo_letter(win, *line, &col);
    print_logo_letter(win, *line, &col);
    print_logo_letter(win, *line, &col);
    wattron(win, COLOR_PAIR(COLORSCHEME_WBL));
    mvwprintw(win, (*line) + 2, col + 2, "version: %s", VERSION);
    wattroff(win, COLOR_PAIR(COLORSCHEME_WBL));
    *line += 3;
}

void print_keybinding(WINDOW *win, int *line, const char *desc, const char *key) {
    int desc_length = strlen(desc);
    int key_length = strlen(key);
    int dots = PAGE_COLS - desc_length - key_length - 2 * (PAGE_SIDE_PADDING);
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    mvwprintw(win, *line, PAGE_SIDE_PADDING_LG, desc);

    for (int i = 0; i < dots; i++) {
        waddch(win, '.');
    }

    wattron(win, COLOR_PAIR(COLORSCHEME_YX));
    wprintw(win, key);
    wattroff(win, COLOR_PAIR(COLORSCHEME_YX));
    *line += 1;
}

void print_bold_title(WINDOW *win, int *line, const char *title) {
    // Add empty line above
    *line += 1;
    wattron(win, A_BOLD | COLOR_PAIR(COLORSCHEME_YX));
    mvwprintw(win, *line, PAGE_SIDE_PADDING_LG, title);
    wattroff(win, A_BOLD | COLOR_PAIR(COLORSCHEME_YX));
    *line += 1;
}

void print_toprow(WINDOW *win, int *line, const char *id, const char *title) {
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    mvwprintw(win, *line, PAGE_SIDE_PADDING, id);
    mvwprintw(win, *line, strlen(id) + 1 + PAGE_SIDE_PADDING, title);
    *line += 1;
}

void print_center(WINDOW *win, int *line, const char *str, int side_padding, attr_t attrs) {
    int str_length = strlen(str);
    int center_start = (PAGE_COLS - str_length) / 2 + PAGE_SIDE_PADDING;
    wattron(win, attrs);
    wmove(win, *line, center_start - side_padding);

    for (int i = 0; i < str_length + 2 * side_padding; i++) {
        waddch(win, ' ');
    }

    mvwprintw(win, *line, center_start, str);
    wattroff(win, attrs);
    *line += 1;
}

void print_center_fill(WINDOW *win, int *line, const char *str, attr_t attrs) {
    fill_rows(win, line, 1, attrs);
    *line -= 1;
    print_center(win, line, str, 0, attrs);
}

void draw_error() {
    const char *error_str = error_get_string();

    if (!error_str) {
        return;
    }

    mvaddstr(LINES - 1, 1, error_str);
}

static void draw_help(WINDOW *win) {
    int line = 0;
    print_toprow(win, &line, "0", "Keybindings");
    print_logo(win, &line);
    print_bold_title(win, &line, "Navigation");
    print_keybinding(win, &line, "previous page", "h");
    print_keybinding(win, &line, "select next link on page", "j");
    print_keybinding(win, &line, "select previous link on page", "k");
    print_keybinding(win, &line, "next page", "l");
    print_keybinding(win, &line, "show index page", "i");
    print_keybinding(win, &line, "go to selected link", "enter");
    print_bold_title(win, &line, "Command mode");
    print_keybinding(win, &line, "enter command mode", ":");
    print_keybinding(win, &line, "go to page", ":<page-number>");
    print_bold_title(win, &line, "General");
    print_keybinding(win, &line, "display (this) help page", "?");
    print_keybinding(win, &line, "quit", "q");
    line = PAGE_LINES - 2;
    print_center(win, &line, "Page 1/1", PAGE_SIDE_PADDING_LG, COLOR_PAIR(COLORSCHEME_BLY));
    print_center_fill(win, &line, "? to close", COLOR_PAIR(COLORSCHEME_YBL));
}

static void draw_empty_page(WINDOW *win) {
    mvwprintw(win, 0, 0, "Empty page!");
}

void draw_main(WINDOW *win, page_t *page) {
    if (error_is_set()) {
        draw_error();
    }

    if (!page || !page->tokens) {
        draw_empty_page(win);
        return;
    }

    page_token_t *cursor = page->tokens;
    wmove(win, 0, 0);

    while (cursor) {
        attr_t style = colors_get_color_pair_from_style(cursor->style);

        if (cursor->style.extra != -1) {
            style |= A_BOLD;
        }

        if (cursor->type == PAGE_TOKEN_LINK) {
            style |= A_UNDERLINE;
        }

        wattron(win, style);

        if (cursor->text) {
            wprintw(win, cursor->text);
        }

        wattroff(win, style);

        cursor = cursor->next;
    }
}

void draw(WINDOW *win, view_t view, page_t *page) {
    wclear(win);
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));

    // Fill with empty characters to show the background color
    for (int i = 0; i < PAGE_LINES * PAGE_COLS; i++) {
        waddch(win, ' ');
    }

    switch (view) {
        case VIEW_MAIN:
            draw_main(win, page);
            break;

        case VIEW_HELP:
            draw_help(win);
            break;

        default:
            break;
    }

    current_view = view;
    wattroff(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    refresh();
    wrefresh(win);
}

void draw_toggle_help(WINDOW *win, page_t *page) {
    if (current_view == VIEW_HELP) {
        draw(win, VIEW_MAIN, page);
    } else {
        draw(win, VIEW_HELP, page);
    }
}

void draw_refresh_current(WINDOW *win, page_t *page) {
    draw(win, current_view, page);
}
