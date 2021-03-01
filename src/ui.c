#include "ui.h"

#define ESCAPE              27
#define DELETE              127
#define BACKSPACE           8
#define PAGE_ID_MAX_LENGTH  3

// TODO: Add window buffer cache to prevent rerendering of pages when switching between VIEW_MAIN and VIEW_HELP
// TODO: Add window where we will echo and take input
static WINDOW *content_win;
static WINDOW *command_win;
static int current_page_index = -1;
static int previous_page_index = -1;
static int previous_page_link_index = -1;
static int current_page_id = TTT_PAGE_HOME;
static page_t *current_page = NULL;
static page_collection_t *collection;

static void set_page_index(int index) {
    if (!collection || index == -1 || collection->size <= index) {
        return;
    }

    previous_page_index = current_page_index;
    previous_page_link_index = draw_get_highlighted_link_index();
    current_page_index = index;
    current_page = collection->pages[index];
    current_page_id = current_page->id;
    draw(content_win, VIEW_MAIN, current_page);
}

static void set_page(uint16_t id) {
    error_reset();

    // Check if the page has been cached
    for (int i = 0; i < collection->size; i++) {
        if (collection->pages[i]->id == id) {
            // TODO: Refetch page if it has an update (and some time has passed, e.g. 5 min)
            set_page_index(i);
            return;
        }
    }

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
    if (draw_get_current_view() != VIEW_MAIN || current_page_id == TTT_PAGE_HOME) {
        return;
    }

    current_page_id -= 1;
    set_page(current_page_id);
}

static void next_page() {
    if (draw_get_current_view() != VIEW_MAIN || current_page_id == TTT_PAGE_CONTENTS) {
        return;
    }

    current_page_id += 1;
    set_page(current_page_id);
}

static void undo_follow_highlighted_link() {
    if (draw_get_current_view() != VIEW_MAIN || previous_page_index == -1) {
        return;
    }

    // Save the previous link index so that we can set it after rendering the page
    int link_index = previous_page_link_index;
    set_page_index(previous_page_index);

    if (link_index != -1) {
        draw_set_highlighted_link_index(content_win, link_index);
    }
}

static void follow_highlighted_link() {
    if (draw_get_current_view() != VIEW_MAIN) {
        return;
    }

    uint16_t href = draw_get_highlighted_link_href();

    if (href >= TTT_PAGE_HOME) {
        set_page(href);
    } else {
        draw_error("ERROR: No or invalid link selected");
    }
}

static void reset_command_mode_input(char buf[256], int *buf_length, bool *command_mode) {
    buf[*buf_length] = '\0';
    *buf_length = 0;
    *command_mode = false;
    draw_command_message(command_win, NULL);
}

static void remove_command_mode_key(char buf[256], int *buf_length) {
    if (*buf_length == 0) {
        return;
    }

    *buf_length -= 1;
    buf[*buf_length] = '\0';
    draw_command_key_remove(command_win, *buf_length);
}

static void execute_command_mode_input(char buf[256], int length) {
    // Clear input window
    draw_command_message(command_win, NULL);

    if (length == 0 || length > PAGE_ID_MAX_LENGTH) {
        return;
    }

    int id = atoi(buf);

    if (!id || id < TTT_PAGE_HOME) {
        draw_command_message(command_win, "Invalid page id");
        return;
    }

    set_page(id);
}

static void create_win() {
    content_win = newwin(
                      PAGE_LINES,
                      PAGE_COLS,
                      (LINES - PAGE_LINES) / 2,
                      (COLS - PAGE_COLS) / 2
                  );
}

static void create_command_win() {
    // Create window below the page window with a height of 1
    command_win = newwin(
                      1,
                      PAGE_COLS,
                      (LINES - PAGE_LINES) / 2 + PAGE_LINES,
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
        create_command_win();
    } else {
        // This fixes a bug where the window "collapses" on resize
        // and flows out to the side.
        wresize(content_win, PAGE_LINES, PAGE_COLS);
        wresize(command_win, 1, PAGE_COLS);
    }

    // TODO: Move window to edges if the screen does not fit the window
    mvwin(content_win, (LINES - PAGE_LINES) / 2, (COLS - PAGE_COLS) / 2);
    mvwin(command_win, (LINES - PAGE_LINES) / 2 + PAGE_LINES, (COLS - PAGE_COLS) / 2);
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
    create_command_win();
    signal(SIGWINCH, resize_handler);
    refresh();
    set_page(current_page_id);
}

void ui_event_loop() {
    int key;
    int buf_length = 0;
    char buf[256];
    bool command_mode = false;

    while (true) {
        // https://stackoverflow.com/questions/3808626/ncurses-refresh/3808913#3808913
        key = wgetch(content_win);

        if (command_mode) {
            switch (key) {
            case ESCAPE:
                reset_command_mode_input(buf, &buf_length, &command_mode);
                break;

            case DELETE:
            case BACKSPACE:
                remove_command_mode_key(buf, &buf_length);
                break;

            case '\n':
                reset_command_mode_input(buf, &buf_length, &command_mode);
                execute_command_mode_input(buf, buf_length - 1);
                break;

            // Colon is not a valid command character
            case ':':
                break;

            default:
                draw_command_key(command_win, key, buf_length);
                buf[buf_length] = key;
                buf_length++;
                break;
            }
        } else {
            switch (key) {
            case ':':
                draw_command_start(command_win);
                command_mode = true;
                break;

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

            case 'i':
                set_page(TTT_PAGE_CONTENTS);
                break;

            case 's':
                set_page(TTT_PAGE_HOME);
                break;

            case 'q':
                return;
            }
        }
    }
}

void ui_destroy() {
    page_collection_destroy(collection);
    delwin(content_win);
    endwin();
}
