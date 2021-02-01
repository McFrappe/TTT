#include "ui.h"

enum views {
    VIEW_MAIN,
    VIEW_HELP
};

// TODO: Add window buffer cache to prevent rerendering of pages when switching between VIEW_MAIN and VIEW_HELP
// TODO: Add window where we will echo and take input
static WINDOW *content_win;
static enum views current_view = VIEW_MAIN;
static page_collection_t *current_collection;

static void draw_error() {
    const char *error_str = error_get_string();

    if (!error_str) {
        return;
    }

    mvwprintw(content_win, PAGE_LINES - 1, 1, error_str);
}

static void draw_main(page_t *page) {
    if (error_is_set()) {
        draw_error();
    }

    if (!current_collection) {
        return;
    }

    mvwprintw(content_win, 1, 1, "MAIN WINDOW");
    mvwprintw(content_win, 1, PAGE_COLS - 4, "%d", current_collection->pages[0]->id);
    mvwprintw(content_win, 2, 1, "%s", current_collection->pages[0]->title);
}

static void draw_help() {
    mvwprintw(content_win, 1, 1, "HELP WINDOW");
}

static void draw(enum views view) {
    wclear(content_win);

    switch (view) {
        case VIEW_MAIN:
            draw_main(NULL);
            break;
        case VIEW_HELP:
            draw_help();
            break;
        default:
            break;
    }

    current_view = view;
    move(0, 0);
    wborder(content_win, 0, 0, 0, 0, 0, 0, 0, 0);
    wrefresh(content_win);
}

static void toggle_view() {
    if (current_view == VIEW_MAIN) {
        draw(VIEW_HELP);
    } else {
        draw(VIEW_MAIN);
    }
}

static void set_page(uint16_t page) {
    error_reset();

    if (current_collection) {
        page_collection_destroy(current_collection);
    }

    current_collection = api_get_page(page);
    draw(VIEW_MAIN);
}

static void resize_win() {
    endwin();
    clear();
    refresh();
    getmaxyx(stdscr, LINES, COLS);
    mvwin(content_win, (LINES / 2) - (PAGE_LINES / 2), (COLS / 2) - (PAGE_COLS / 2));
    refresh();
    draw(current_view);
}

static void resize_handler(int sig) {
    resize_win();
}

void ui_initialize() {
    api_initialize();
    initscr();
    content_win = newwin(
        PAGE_LINES,
        PAGE_COLS,
        (LINES / 2) - (PAGE_LINES / 2),
        (COLS / 2) - (PAGE_COLS / 2)
    );
    signal(SIGWINCH, resize_handler);
    noecho();
    nodelay(stdscr, TRUE);
    refresh();
    set_page(TTT_PAGE_HOME);
}

void ui_event_loop() {
    int key;

    while (true)  {
        // https://stackoverflow.com/questions/3808626/ncurses-refresh/3808913#3808913
        key = wgetch(content_win);

        switch (key) {
            case '?':
                toggle_view();
                break;
            case 'q':
                return;
        }
    }
}

void ui_destroy() {
    LINES = 0;
    COLS = 0;
    current_view = VIEW_MAIN;
    page_collection_destroy(current_collection);
    delwin(content_win);
    endwin();
}
