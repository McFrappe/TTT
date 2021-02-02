#include "ui.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    ui_initialize();
    ui_event_loop();
    ui_destroy();

    if (argc > 1) {
        // Resetting is really slow, so we hide it behind '-r'
        if (strcmp(argv[1], "-r") == 0) {
            // Seems like this is one of the only ways to actually restore the terminal colors
            // https://stackoverflow.com/questions/8686368/how-can-the-original-terminal-palette-be-acquired-preferably-using-ncurses-rout
            execlp("reset", "reset", NULL);
        }
    }

    return 0;
}
