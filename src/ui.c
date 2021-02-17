#include "ui.h"

// TODO: Add window buffer cache to prevent rerendering of pages when switching between VIEW_MAIN and VIEW_HELP
// TODO: Add window where we will echo and take input
static WINDOW *content_win;
static int current_page_index = -1;
static int current_page_id = TTT_PAGE_HOME;
static page_t *current_page = NULL;
static page_collection_t *current_collection;

static void set_page(uint16_t page) {
    error_reset();

    if (current_collection) {
        page_collection_destroy(current_collection);
    }

    current_collection = api_get_page(page);
    current_page_index = 0;
    current_page = current_collection->pages[0];
    draw(content_win, VIEW_MAIN, current_page);
}

static void previous_page() {
    if (current_page_id == TTT_PAGE_HOME) {
        return;
    }

    current_page_id -= 1;
    set_page(current_page_id);
}

static void next_page() {
    if (current_page_id == TTT_PAGE_CONTENTS) {
        return;
    }

    current_page_id += 1;
    set_page(current_page_id);
}

static void follow_highlighted_link() {
    uint16_t href = draw_get_highlighted_link_page_id();

    if (href >= TTT_PAGE_HOME) {
        set_page(href);
    } else {
        draw_error("ERROR: No or invalid link selected");
    }
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

void ui_initialize(bool overwrite_colors, bool transparent_background) {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    api_initialize();
    colors_initialize(overwrite_colors, transparent_background);
    create_win();
    signal(SIGWINCH, resize_handler);
    refresh();
    set_page(current_page_id);
}

void ui_event_loop() {
    int key;

    while (true) {
        // https://stackoverflow.com/questions/3808626/ncurses-refresh/3808913#3808913
        key = wgetch(content_win);

        switch (key) {
        case 'h':
            previous_page();
            break;

        case 'j':
            draw_next_link(content_win);
            break;

        case 'k':
            draw_previous_link(content_win);
            break;

        case 'l':
            next_page();
            break;

        case '?':
            draw_toggle_help(content_win, current_page);
            break;

        case '\n':
            follow_highlighted_link();
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
