#include "ui.h"
#include "api.h"
#include "parser.h"
#include <stdlib.h>

// TODO: Move into its own module
void destroy_page_collection(page_collection_t *collection) {
    for (size_t i = 0; i < collection->size; i++) {
        free(collection->pages[i]);
    }

    free(collection);
}

int main(void) {
    api_intialize();
    /* page_collection_t *home = api_get_page_range(HOME, FOREIGN_NEWS); */
    page_collection_t *news = api_get_page(NEWS);
    /* destroy_page_collection(home); */
    destroy_page_collection(news);
    api_destroy();
    return 0;
}
