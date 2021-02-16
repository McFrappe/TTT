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

void draw_next_link(WINDOW *win);
void draw_previous_link(WINDOW *win);
void draw(WINDOW *win, view_t current, page_t *page);
void draw_toggle_help(WINDOW *win, page_t *page);
void draw_refresh_current(WINDOW *win, page_t *page);
