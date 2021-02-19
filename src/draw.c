#include "draw.h"

#define MAX_PAGE_LINKS 32

// Stores the position and length of a clickable link
typedef struct {
    int x, y;
    page_token_t *token;
} link_t;

static view_t current_view;

static int current_link = -1;
static int current_link_count = 0;
static link_t rendered_links[MAX_PAGE_LINKS];

static bool is_valid_link_index(int link_index) {
    return link_index != -1 && current_link_count > 0 && link_index < current_link_count;
}

static void save_rendered_link(WINDOW *win, page_token_t *token) {
    if (current_link_count >= MAX_PAGE_LINKS - 1) {
        return;
    }

    getyx(win, rendered_links[current_link_count].y, rendered_links[current_link_count].x);
    rendered_links[current_link_count].token = token;
    current_link_count++;
}

static void terminate_rendered_link_list() {
    rendered_links[current_link].token = NULL;
}

static void draw_token(WINDOW *win, page_token_t *token, bool save_link) {
    attr_t style = colors_get_color_pair_from_style(token->style);

    if (token->style.extra != -1) {
        style |= A_BOLD;
    }

    if (token->type == PAGE_TOKEN_LINK) {
        style |= A_UNDERLINE;

        if (save_link) {
            save_rendered_link(win, token);
        }
    }

    wattron(win, style);

    if (token->text) {
        wprintw(win, token->text);
    }

    wattroff(win, style);
}

static void dehighlight_link(WINDOW *win) {
    assert(current_link >= 0);
    assert(current_link < MAX_PAGE_LINKS);
    link_t current = rendered_links[current_link];
    wmove(win, current.y, current.x);
    draw_token(win, current.token, false);
    wrefresh(win);
}

static void highlight_link(WINDOW *win) {
    assert(current_link >= 0);
    assert(current_link < MAX_PAGE_LINKS);
    link_t current = rendered_links[current_link];
    wmove(win, current.y, current.x);
    attr_t style = COLOR_PAIR(COLORSCHEME_BW);
    wattron(win, style | A_BOLD | A_UNDERLINE);
    waddnstr(win, current.token->text, current.token->length);
    wattroff(win, style | A_BOLD | A_UNDERLINE);
    wrefresh(win);
}

/// @brief Prints a T in a 3x3 box
static void print_logo_letter(WINDOW *win, int line, int *col) {
    wattron(win, COLOR_PAIR(COLORSCHEME_BW) | A_UNDERLINE);
    wmove(win, line, *col);

    for (int i = 0; i < 6; i++) {
        waddch(win, ' ');
    }

    wmove(win, line + 1, (*col) + 2);

    for (int i = 0; i < 2; i++) {
        waddch(win, ' ');
    }

    wmove(win, line + 2, (*col) + PAGE_SIDE_PADDING_LG);
    waddch(win, ' ');
    wattroff(win, COLOR_PAIR(COLORSCHEME_BW) | A_UNDERLINE);
    *col += 3 + PAGE_SIDE_PADDING;
}

static void fill_rows(WINDOW *win, int *line, int rows, attr_t attr) {
    wattron(win, attr);
    wmove(win, *line, PAGE_SIDE_PADDING);

    for (int i = 0; i < rows; i++) {
        wmove(win, (*line + i), 1);

        for (int j = 0; j < PAGE_COLS - PAGE_SIDE_PADDING; j++) {
            waddch(win, ' ');
        }
    }

    wattroff(win, attr);
    *line += rows;
}

static void print_logo(WINDOW *win, int *line) {
    int col = PAGE_SIDE_PADDING_LG;
    fill_rows(win, line, 4, COLOR_PAIR(COLORSCHEME_YBL));
    *line -= 3;
    print_logo_letter(win, *line, &col);
    print_logo_letter(win, *line, &col);
    print_logo_letter(win, *line, &col);
    wattron(win, COLOR_PAIR(COLORSCHEME_WBL));
    mvwprintw(win, (*line) + 2, col + 2, "version: %s", VERSION);
    wattroff(win, COLOR_PAIR(COLORSCHEME_WBL));
    *line += 3;
}

static void print_keybinding(WINDOW *win, int *line, const char *desc, const char *key) {
    int desc_length = strlen(desc);
    int key_length = strlen(key);
    int dots = PAGE_COLS - desc_length - key_length - 2 * (PAGE_SIDE_PADDING);
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    mvwprintw(win, *line, PAGE_SIDE_PADDING_LG, desc);

    for (int i = 0; i < dots; i++) {
        waddch(win, '.');
    }

    wattron(win, COLOR_PAIR(COLORSCHEME_YX));
    wprintw(win, key);
    wattroff(win, COLOR_PAIR(COLORSCHEME_YX));
    *line += 1;
}

static void print_bold_title(WINDOW *win, int *line, const char *title) {
    // Add empty line above
    *line += 1;
    wattron(win, A_BOLD | COLOR_PAIR(COLORSCHEME_YX));
    mvwprintw(win, *line, PAGE_SIDE_PADDING_LG, title);
    wattroff(win, A_BOLD | COLOR_PAIR(COLORSCHEME_YX));
    *line += 1;
}

static void print_toprow(WINDOW *win, int *line, const char *id, const char *title) {
    wattron(win, COLOR_PAIR(COLORSCHEME_DEFAULT));
    mvwprintw(win, *line, PAGE_SIDE_PADDING, id);
    mvwprintw(win, *line, strlen(id) + 1 + PAGE_SIDE_PADDING, title);
    *line += 1;
}

static void print_center(WINDOW *win, int *line, const char *str, int side_padding, attr_t attrs) {
    int str_length = strlen(str);
    int center_start = (PAGE_COLS - str_length) / 2 + PAGE_SIDE_PADDING;
    wattron(win, attrs);
    wmove(win, *line, center_start - side_padding);

    for (int i = 0; i < str_length + 2 * side_padding; i++) {
        waddch(win, ' ');
    }

    mvwprintw(win, *line, center_start, str);
    wattroff(win, attrs);
    *line += 1;
}

static void print_center_fill(WINDOW *win, int *line, const char *str, attr_t attrs) {
    fill_rows(win, line, 1, attrs);
    *line -= 1;
    print_center(win, line, str, 0, attrs);
}

uint16_t draw_get_highlighted_link_href() {
    if (!is_valid_link_index(current_link)) {
        // Everything below TTT_PAGE_HOME is considered invalid
        return 0;
    }

    return rendered_links[current_link].token->href;
}

int draw_get_highlighted_link_index() {
    return current_link;
}

void draw_set_highlighted_link_index(WINDOW *win, int new_index) {
    if (!is_valid_link_index(new_index)) {
        return;
    }

    if (current_link == -1) {
        current_link = new_index;
        highlight_link(win);
    } else {
        // Could make a new function for this, but it is easier to use the existing one
        current_link = new_index - 1;
        draw_next_link(win);
    }
}

void draw_next_link(WINDOW *win) {
    if (current_link != -1 && current_link < current_link_count) {
        dehighlight_link(win);
    }

    if (current_link >= current_link_count - 1) {
        current_link = -1;
        return;
    }

    current_link++;
    highlight_link(win);
}

void draw_previous_link(WINDOW *win) {
    if (current_link != -1) {
        dehighlight_link(win);
    }

    if (current_link == 0) {
        current_link = -1;
        return;
    } else if (current_link == -1) {
        current_link = current_link_count - 1;
    } else {
        current_link--;
    }

    highlight_link(win);
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


void draw_error(const char *str) {
    if (!str) {
        return;
    }

    mvaddstr(LINES - 1, 1, str);
    refresh();
}

void draw_help(WINDOW *win) {
    int line = 0;
    print_toprow(win, &line, "0", "Keybindings");
    print_logo(win, &line);
    print_bold_title(win, &line, "Navigation");
    print_keybinding(win, &line, "previous page", "h/p");
    print_keybinding(win, &line, "select next link on page", "j");
    print_keybinding(win, &line, "select previous link on page", "k");
    print_keybinding(win, &line, "next page", "l/n");
    print_keybinding(win, &line, "show index page", "i");
    print_keybinding(win, &line, "go to selected link", "enter");
    print_keybinding(win, &line, "go back to previous page", "u/b");
    print_bold_title(win, &line, "Command mode");
    print_keybinding(win, &line, "enter command mode", ":");
    print_keybinding(win, &line, "go to page", ":<page-number>");
    print_bold_title(win, &line, "General");
    print_keybinding(win, &line, "display (this) help page", "?");
    print_keybinding(win, &line, "quit", "q");
    line = PAGE_LINES - 2;
    print_center(win, &line, "Page 1/1", PAGE_SIDE_PADDING_LG, COLOR_PAIR(COLORSCHEME_BLY));
    print_center_fill(win, &line, "? to close", COLOR_PAIR(COLORSCHEME_YBL));
}

void draw_empty_page(WINDOW *win) {
    mvwprintw(win, 0, 0, "Empty page!");
}

void draw_main(WINDOW *win, page_t *page) {
    if (error_is_set()) {
        draw_error(error_get_string());
    }

    if (!page || !page->tokens) {
        draw_empty_page(win);
        return;
    }

    page_token_t *cursor = page->tokens;
    wmove(win, 0, 0);
    current_link = -1;
    current_link_count = 0;

    while (cursor) {
        draw_token(win, cursor, true);
        cursor = cursor->next;
    }

    terminate_rendered_link_list();
}

void draw(WINDOW *win, view_t view, page_t *page) {
    clear();
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
    refresh();
    wrefresh(win);
}
