#pragma once
#include <curses.h>
#include <signal.h>
#include "pages.h"

void ui_initialize();
void ui_event_loop();
void ui_destroy();
