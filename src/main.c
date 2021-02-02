#include "ui.h"
#include <unistd.h>

// TODO: Add CLI-arguments
int main(void) {
    ui_initialize();
    ui_event_loop();
    ui_destroy();

    execlp("reset", "reset", NULL);
    return 0;
}
