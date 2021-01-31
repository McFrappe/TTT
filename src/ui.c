#include "ui.h"

enum views {
    MAIN,
    HELP
};

// TODO: Add window buffer cache to prevent rerendering of pages when switching between MAIN and HELP
// TODO: Add window where we will echo and take input
static WINDOW *CONTENT_WIN;
static enum views CURRENT_VIEW = MAIN;

static void draw_main_view(page_t *page) {
    mvwprintw(CONTENT_WIN, 1, 1, "MAIN WINDOW");
}

static void draw_help_view() {
    mvwprintw(CONTENT_WIN, 1, 1, "HELP WINDOW");
}

static void draw(enum views view) {
    wclear(CONTENT_WIN);

    switch (view) {
        case MAIN:
            draw_main_view(NULL);
            break;
        case HELP:
            draw_help_view();
            break;
        default:
            break;
    }

    CURRENT_VIEW = view;
    move(0, 0);
    wborder(CONTENT_WIN, 0, 0, 0, 0, 0, 0, 0, 0);
    wrefresh(CONTENT_WIN);
}

static void resize_win() {
    endwin();
    clear();
    refresh();
    getmaxyx(stdscr, LINES, COLS);
    mvwin(CONTENT_WIN, (LINES / 2) - (PAGE_LINES / 2), (COLS / 2) - (PAGE_COLS / 2));
    refresh();
    draw(CURRENT_VIEW);
}

static void resize_handler(int sig) {
    resize_win();
}

void ui_initialize() {
    initscr();
    CONTENT_WIN = newwin(
        PAGE_LINES,
        PAGE_COLS,
        (LINES / 2) - (PAGE_LINES / 2),
        (COLS / 2) - (PAGE_COLS / 2)
    );
    signal(SIGWINCH, resize_handler);
    noecho();
    nodelay(stdscr, TRUE);
    refresh();
    draw(MAIN);
}

void ui_event_loop() {
    int key;

    while (true)  {
        // https://stackoverflow.com/questions/3808626/ncurses-refresh/3808913#3808913
        key = wgetch(CONTENT_WIN);

        switch (key) {
            case '?':
                draw(HELP);
                break;
            case 'x':
                draw(MAIN);
                break;
            case 'q':
                return;
        }
    }
}

void ui_destroy() {
    LINES = 0;
    COLS = 0;
    CURRENT_VIEW = MAIN;
    delwin(CONTENT_WIN);
    endwin();
}
