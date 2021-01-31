#include <curses.h>

static WINDOW *mainwin;

void ui_initialize() {
    mainwin = initscr();              //Start curses mode
    printw("Hello World!"); // Print to screen hello world.
    refresh();              // Print it to the real screen on the terminal
    //getch();                // wait for user input
}

void ui_destroy() {
    delwin(mainwin);
    endwin();
}