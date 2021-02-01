#pragma once
#include <curses.h>
#include <signal.h>

#include "api.h"
#include "pages.h"
#include "errors.h"
#include "shared.h"

void ui_initialize();
void ui_event_loop();
void ui_destroy();
