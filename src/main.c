#include "ui.h"
#include <unistd.h>

void print_help() {
    printf("usage ttt [-h] [-r]\n\n");
    printf("TTT - Terminal Text TV\n");
    printf("A terminal interface to Text TV written in C\n\n");
    printf("optional arguments:\n");
    printf("-h          show this help message\n");
    printf("-r          restore terminal colors on quit\n");
    printf("-d          do not overwrite terminal colors (might decrease readability)\n");
    printf("-t          transparent background for page content (works well with '-d')\n");
}

int main(int argc, char *argv[]) {
    bool reset = false;
    bool overwrite_colors = true;
    bool transparent_background = false;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            // Resetting is really slow, so we hide it behind '-r'
            if (strcmp(argv[i], "-r") == 0) {
                reset = true;
            } else if (strcmp(argv[i], "-d") == 0) {
                overwrite_colors = false;
            } else if (strcmp(argv[i], "-t") == 0) {
                transparent_background = true;
            } else {
                print_help();
                return 1;
            }
        }
    }

    ui_initialize(overwrite_colors, transparent_background);
    ui_event_loop();
    ui_destroy();

    if (reset) {
        // Seems like this is one of the only ways to actually restore the terminal colors
        // https://stackoverflow.com/questions/8686368/how-can-the-original-terminal-palette-be-acquired-preferably-using-ncurses-rout
        execlp("reset", "reset", NULL);
    }

    return 0;
}
