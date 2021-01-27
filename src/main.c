#include "ui.h"
#include "api.h"
#include "parser.h"
#include <stdlib.h>

int main(void) {
    api_intialize();
    page_t *home = api_get_page_range(HOME, FOREIGN_NEWS);
    page_t *news = api_get_page(NEWS);
    free(home);
    free(news);
    api_destroy();
    return 0;
}
