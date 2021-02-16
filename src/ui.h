#pragma once
#include <curses.h>
#include <signal.h>
#include <locale.h>

#include "api.h"
#include "draw.h"
#include "pages.h"
#include "colors.h"
#include "shared.h"

void ui_initialize(bool overwrite_colors, bool transparent_background);
void ui_event_loop();
void ui_destroy();
