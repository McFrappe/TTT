#include "ui.h"

// TODO: Add window buffer cache to prevent rerendering of pages when switching between VIEW_MAIN and VIEW_HELP
// TODO: Add window where we will echo and take input
static WINDOW *content_win;
static int current_page_index = -1;
static page_t *current_page = NULL;
static page_collection_t *current_collection;
static char page_cache[MAX_PAGE_COLLECTION_SIZE][PAGE_LINES * PAGE_COLS];

static void save_view_buffer() {
    assert(current_page_index >= 0);
    assert(current_page_index < MAX_PAGE_COLLECTION_SIZE);
    mvwinstr(content_win, PAGE_LINES, PAGE_COLS, page_cache[current_page_index]);
}

static void restore_page_buffer(int index) {
    assert(index >= 0);
    assert(index < MAX_PAGE_COLLECTION_SIZE);

    if (page_cache[index][0] == '\0') {
        return;
    }

    mvwaddstr(content_win, PAGE_LINES, PAGE_COLS, page_cache[index]);
    page_cache[index][0] = '\0';
}

static void set_page(uint16_t page) {
    error_reset();

    if (current_collection) {
        page_collection_destroy(current_collection);
    }

    current_collection = api_get_page(page);
    // TODO: set the current page and current_page_index accordingly
    current_page_index = 0;
    current_page = current_collection->pages[0];
    draw(content_win, VIEW_MAIN, current_page);
}

static void create_win() {
    content_win = newwin(
                      PAGE_LINES,
                      PAGE_COLS,
                      (LINES - PAGE_LINES) / 2,
                      (COLS - PAGE_COLS) / 2
                  );
}

static void resize_win() {
    endwin();
    clear();
    refresh();
    getmaxyx(stdscr, LINES, COLS);

    // If the window size is larger than the size of the terminal
    // at launch, the window will no be created correctly.
    // If this is the case, the window will be NULL and we simply
    // try to create it again.
    if (!content_win) {
        create_win();
    } else {
        // This fixes a bug where the window "collapses" on resize
        // and flows out to the side.
        wresize(content_win, PAGE_LINES, PAGE_COLS);
    }

    // TODO: Move window to edges if the screen does not fit the window
    mvwin(content_win, (LINES - PAGE_LINES) / 2, (COLS - PAGE_COLS) / 2);
    draw_refresh_current(content_win, current_page);
}

static void resize_handler(int sig) {
    resize_win();
}

void ui_initialize() {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    nodelay(stdscr, TRUE);
    // Hide cursor
    curs_set(0);
    api_initialize();
    colors_initialize();
    create_win();
    signal(SIGWINCH, resize_handler);

    // By checking the first char of each page buffer cache we can detect empty buffers
    for (int i = 0; i < MAX_PAGE_COLLECTION_SIZE; i++) {
        page_cache[i][0] = '\0';
    }

    refresh();
    set_page(TTT_PAGE_HOME);
}

void ui_event_loop() {
    int key;

    while (true) {
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
    page_collection_destroy(current_collection);
    delwin(content_win);
    endwin();
}
