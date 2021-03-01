#pragma once
#include <curses.h>
#include <assert.h>

#include "pages.h"
#include "colors.h"
#include "errors.h"
#include "shared.h"

typedef enum view {
    VIEW_MAIN,
    VIEW_HELP
} view_t;

void draw_error(const char *str);
void draw_next_link(WINDOW *win);
void draw_previous_link(WINDOW *win);
void draw_help(WINDOW *win);
void draw_empty_page(WINDOW *win);
void draw_toggle_help(WINDOW *win, page_t *page);
void draw_refresh_current(WINDOW *win, page_t *page);
void draw(WINDOW *win, view_t current, page_t *page);

/// @brief Returns the page id of the currently highlighted link
/// @return page id or 0 if no link is selected
uint16_t draw_get_highlighted_link_href();

int draw_get_current_view();
int draw_get_highlighted_link_index();
void draw_set_highlighted_link_index(WINDOW *win, int new_index);

void draw_command_start(WINDOW *win);
void draw_command_key(WINDOW *win, char key, int index);
void draw_command_key_remove(WINDOW *win, int index);
void draw_command_message(WINDOW *win, char *str);
