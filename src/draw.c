#include "draw.h"
#include "pages.h"

static view_t current_view;

void fill_rows(WINDOW *win, int *line, int rows, attr_t attr) {
    wattron(win, attr);
    wmove(win, *line, 0);
    for (int i = 0; i < rows * PAGE_COLS; i++) {
        waddch(win, ' ');
    }
    wattroff(win, attr);
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    *line += rows;
}

void print_keybinding(WINDOW *win, int *line, const char *desc, const char *key) {
    int desc_length = strlen(desc);
    int key_length = strlen(key);
    int dots = PAGE_COLS - desc_length - key_length - 2 * (PAGE_SIDE_PADDING);

    mvwprintw(win, *line, PAGE_SIDE_PADDING, desc);

    for (int i = 0; i < dots;i++) {
        waddch(win, '.');
    }

    wprintw(win, key);
    *line += 1;
}

void print_bold_title(WINDOW *win, int *line, const char *title, attr_t attrs) {
    // Add empty line above
    *line += 1;
    wattron(win, A_BOLD | attrs);
    mvwprintw(win, *line, PAGE_SIDE_PADDING, title);
    wattroff(win, A_BOLD | attrs);
    *line += 1;
}

void print_toprow(WINDOW *win, int *line, const char *id, const char *title) {
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    mvwprintw(win, *line, 0, id);
    mvwprintw(win, *line, strlen(id) + 1, title);
    *line += 1;
}

void print_center(WINDOW *win, int *line, const char *str, int side_padding, attr_t attrs) {
    int str_length = strlen(str);
    int center_start = (PAGE_COLS - str_length) / 2;

    wattron(win, attrs);
    wmove(win, *line, center_start - side_padding);

    for (int i = 0; i < str_length + 2 * side_padding; i++) {
        waddch(win, ' ');
    }

    mvwprintw(win, *line, center_start, str);
    *line += 1;
}

void draw_error(WINDOW *win) {
    const char *error_str = error_get_string();

    if (!error_str) {
        return;
    }

    mvwprintw(win, PAGE_LINES - 1, 0, error_str);
}

static void draw_help(WINDOW *win) {
    int line = 0;
    print_toprow(win, &line, "0", "Keybindings");
    fill_rows(win, &line, 4, COLOR_PAIR(COLORSCHEME_YB));
    print_bold_title(win, &line, "Navigation", 0);
    print_keybinding(win, &line, "previous page", "h");
    print_keybinding(win, &line, "select next link on page", "j");
    print_keybinding(win, &line, "select previous link on page", "k");
    print_keybinding(win, &line, "next page", "l");
    print_keybinding(win, &line, "go to selected link", "enter");
    print_bold_title(win, &line, "General", 0);
    print_keybinding(win, &line, "display (this) help page", "?");
    print_keybinding(win, &line, "quit", "q");
    line = PAGE_LINES - 2;
    print_center(win, &line, "Page 1/1", PAGE_SIDE_PADDING_LG, COLOR_PAIR(COLORSCHEME_BY));
    fill_rows(win, &line, 1, COLOR_PAIR(COLORSCHEME_YB));
}

static void draw_empty_page(WINDOW *win) {
    mvwprintw(win, 0, 0, "Empty page!");
}

void draw_main(WINDOW *win, page_t *page) {
    if (error_is_set()) {
        draw_error(win);
    }

    if (!page) {
        draw_empty_page(win);
        return;
    }

    // TODO: Render page
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
