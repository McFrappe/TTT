#include "ui.h"
#include "api.h"
#include "parser.h"
#include <stdlib.h>

int main(void) {
    api_intialize();
    /* page_collection_t *home = api_get_page_range(HOME, FOREIGN_NEWS); */
    page_collection_t *news = api_get_page(NEWS);
    /* destroy_page_collection(home); */
    destroy_page_collection(news);
    api_destroy();
    return 0;
}
