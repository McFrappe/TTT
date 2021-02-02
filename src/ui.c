#include "ui.h"

// TODO: Add window buffer cache to prevent rerendering of pages when switching between VIEW_MAIN and VIEW_HELP
// TODO: Add window where we will echo and take input
static WINDOW *content_win;
static page_t *current_page = NULL;
static page_collection_t *current_collection;

static void set_page(uint16_t page) {
    error_reset();

    if (current_collection) {
        page_collection_destroy(current_collection);
    }

    //current_collection = api_get_page(page);
    current_collection = page_collection_create(1);
    current_collection->pages[0] = page_create_empty();
    current_collection->pages[0]->id = 100;
    current_collection->pages[0]->title = strdup("Test page title");
    // TODO: set the current page accordingly
    current_page = current_collection->pages[0];
    draw(content_win, VIEW_MAIN, current_page);
}

static void resize_win() {
    endwin();
    clear();
    refresh();
    getmaxyx(stdscr, LINES, COLS);
    mvwin(content_win, (LINES / 2) - (PAGE_LINES / 2), (COLS / 2) - (PAGE_COLS / 2));
    refresh();
    draw_refresh_current(content_win, current_page);
}

static void resize_handler(int sig) {
    resize_win();
}

void ui_initialize() {
    setlocale(LC_ALL, "");
    initscr();
    api_initialize();
    colors_initialize();
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
                draw_toggle_help(content_win, current_page);
                break;
            case 'q':
                return;
        }
    }
}

void ui_destroy() {
    LINES = 0;
    COLS = 0;
    page_collection_destroy(current_collection);
    delwin(content_win);
    endwin();
}
