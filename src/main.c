#include "ui.h"
#include <unistd.h>

void print_help() {
    printf("usage ttt [-h] [-r]\n\n");
    printf("TTT - Terminal Text TV\n");
    printf("A terminal interface to Text TV written in C\n\n");
    printf("optional arguments:\n");
    printf("-h          show this help message\n");
    printf("-r          restore terminal colors on quit\n");
}

int main(int argc, char *argv[]) {
    bool reset = false;

    if (argc > 1) {
        // Resetting is really slow, so we hide it behind '-r'
        if (strcmp(argv[1], "-r") == 0) {
            reset = true;
        } else {
            print_help();
            return 1;
        }
    }

    ui_initialize();
    ui_event_loop();
    ui_destroy();

    if (reset) {
        // Seems like this is one of the only ways to actually restore the terminal colors
        // https://stackoverflow.com/questions/8686368/how-can-the-original-terminal-palette-be-acquired-preferably-using-ncurses-rout
        execlp("reset", "reset", NULL);
    }

    return 0;
}
