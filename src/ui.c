#include <curses.h>

void initialize_ui() {
    initscr();              //Start curses mode
    printw("Hello World!"); // Print to screen hello world.
    refresh();              // Print it to the real screen on the terminal
    getch();                // wait for user input
    endwin();               // End curses mode
}
