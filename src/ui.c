#include "ui.h"

// TODO: Add window buffer cache to prevent rerendering of pages when switching between VIEW_MAIN and VIEW_HELP
// TODO: Add window where we will echo and take input
static WINDOW *content_win;
static int previous_page_index = -1;
static int current_page_index = -1;
static int current_page_id = TTT_PAGE_HOME;
static page_t *current_page = NULL;
static page_collection_t *collection;

static void set_page_index(int index) {
    if (!collection || index == -1 || collection->size <= index) {
        return;
    }

    previous_page_index = current_page_index;
    current_page_index = index;
    current_page = collection->pages[index];
    draw(content_win, VIEW_MAIN, current_page);
}

static void set_page(uint16_t id) {
    error_reset();

    page_t *page = api_get_page(id);

    if (!page) {
        if (error_is_set()) {
            draw_error(error_get_string());
        }

        return;
    }

    page_collection_resize(collection, collection->size + 1);
    collection->pages[collection->size - 1] = page;
    set_page_index(collection->size - 1);
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

static void undo_follow_highlighted_link() {
    if (previous_page_index != -1) {
        set_page_index(previous_page_index);
    }
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
    collection = page_collection_create(0);
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
        case 'p':
            previous_page();
            break;

        case 'j':
            draw_next_link(content_win);
            break;

        case 'k':
            draw_previous_link(content_win);
            break;

        case 'l':
        case 'n':
            next_page();
            break;

        case '?':
            draw_toggle_help(content_win, current_page);
            break;

        case '\n':
            follow_highlighted_link();
            break;

        case 'u':
        case 'b':
            undo_follow_highlighted_link();
            break;

        case 'q':
            return;
        }
    }
}

void ui_destroy() {
    page_collection_destroy(collection);
    delwin(content_win);
    endwin();
}
