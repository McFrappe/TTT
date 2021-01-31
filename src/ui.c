#include "ui.h"

#define WIN_COLS 42
#define WIN_ROWS 24

enum views {
    MAIN,
    HELP
};

// TODO: Add window where we will echo and take input
static WINDOW *CONTENT_WIN;
static int _ROWS, _COLS;
static enum views CURRENT_VIEW = MAIN;

static void draw_main_view(page_t *page) {
    mvwprintw(CONTENT_WIN, WIN_ROWS / 2, WIN_COLS / 2, "MAIN WINDOW");
}

static void draw_help_view() {
    mvwprintw(CONTENT_WIN, WIN_ROWS / 2, WIN_COLS / 2, "HELP WINDOW");
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
    getmaxyx(stdscr, _ROWS, _COLS);
    CONTENT_WIN = newwin(
        WIN_ROWS,
        WIN_COLS,
        (_ROWS / 2) - (WIN_ROWS / 2),
        (_COLS / 2) - (WIN_COLS / 2)
    );
    draw(CURRENT_VIEW);
}

static void resize_handler(int sig) {
    resize_win();
}

void ui_initialize() {
    initscr();
    signal(SIGWINCH, resize_handler);
    noecho();
    nodelay(stdscr, TRUE);
    refresh();
    resize_win();
}

void ui_event_loop() {
    int key;
    
    while (true)  {
        key = getch();
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
    _ROWS = 0;
    _COLS = 0;
    CURRENT_VIEW = MAIN;
    endwin();
}
