#include "ui.h"
#include <curses.h>
#include <locale.h>

enum views {
    VIEW_MAIN,
    VIEW_HELP
};

// Color pair 0 is reserved for the system:
// https://linux.die.net/man/3/color_pair
enum colorschemes {
    COLORSCHEME_SYSTEM,
    COLORSCHEME_DEFAULT,
    COLORSCHEME_HEADER,
    // TODO: Add colorscheme for each colorscheme in response (docs/parser/html.md)
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

    mvwprintw(content_win, PAGE_LINES - 1, 0, error_str);
}

static void draw_main(page_t *page) {
    if (error_is_set()) {
        draw_error();
    }

    if (!current_collection) {
        return;
    }

    wattron(content_win, A_STANDOUT);
    mvwprintw(content_win, 0, 0, " MAIN WINDOW ");
    wattroff(content_win, A_STANDOUT);
    mvwprintw(content_win, 0, PAGE_COLS - 3, "%d", current_collection->pages[0]->id);
    wattron(content_win, COLOR_PAIR(COLORSCHEME_HEADER));
    mvwprintw(content_win, 1, 0, "%s", current_collection->pages[0]->title);
    wattroff(content_win, COLOR_PAIR(COLORSCHEME_HEADER));
}

static void draw_help() {
    wattron(content_win, A_STANDOUT);
    mvwprintw(content_win, 0, 0, " HELP WINDOW ");
    wattroff(content_win, A_STANDOUT);
    mvwprintw(content_win, 2, 0, "q - Quit the program");

}

static void draw(enum views view) {
    wclear(content_win);
    wattron(content_win, COLOR_PAIR(COLORSCHEME_DEFAULT));

    // Fill with empty characters to show the background color
    for (int i = 0; i < PAGE_LINES * PAGE_COLS; i++) {
        waddch(content_win, ' ');
    }

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
    wattroff(content_win, COLOR_PAIR(COLORSCHEME_DEFAULT));
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

    //current_collection = api_get_page(page);
    current_collection = page_collection_create(1);
    current_collection->pages[0] = page_create_empty();
    current_collection->pages[0]->id = 100;
    current_collection->pages[0]->title = strdup("Test page title");
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

static void set_colorschemes() {
    start_color();
    use_default_colors();
    // TODO: Save initial colors and restore on exit
    //       E.g. if using pywal and st, updating these will
    //       actually set the colors globally for the current instance
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_BLUE, 0, 0, 500);
    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_pair(COLORSCHEME_DEFAULT, COLOR_WHITE,   COLOR_BLACK);
    init_pair(COLORSCHEME_HEADER,  COLOR_YELLOW,  COLOR_BLUE);
}

void ui_initialize() {
    setlocale(LC_ALL, "");
    api_initialize();
    initscr();
    set_colorschemes();
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
