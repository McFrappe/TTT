#include "ui.h"

// TODO: Add CLI-arguments
int main(void) {
    ui_initialize();
    ui_event_loop();
    ui_destroy();
    return 0;
}
