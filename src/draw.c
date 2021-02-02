#include "draw.h"
#include "pages.h"

static view_t current_view;

void draw_error(WINDOW *win) {
    const char *error_str = error_get_string();

    if (!error_str) {
        return;
    }

    mvwprintw(win, PAGE_LINES - 1, 0, error_str);
}

static void draw_help(WINDOW *win) {
    wattron(win, A_STANDOUT);
    mvwprintw(win, 0, 0, " HELP WINDOW ");
    wattroff(win, A_STANDOUT);
    mvwprintw(win, 2, 0, "q - Quit the program");
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

    wattron(win, A_STANDOUT);
    mvwprintw(win, 0, 0, " MAIN WINDOW ");
    wattroff(win, A_STANDOUT);
    mvwprintw(win, 0, PAGE_COLS - 3, "%d", page->id);
    wattron(win, COLOR_PAIR(COLORSCHEME_HEADER));
    mvwprintw(win, 1, 0, "%s", page->title);
    wattroff(win, COLOR_PAIR(COLORSCHEME_HEADER));
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    mvwprintw(win, 2, 0, "\u00e5\u00e4\u00f6");
    wattroff(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
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
