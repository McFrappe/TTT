#include "ui.h"
#include "api.h"
#include "pages.h"
#include "parser.h"
#include <stdlib.h>

int main(void) {
    api_initialize();
    page_collection_t *home = api_get_page_range(HOME, FOREIGN_NEWS);
    page_collection_t *news = api_get_page(NEWS);
    page_collection_print(home, "HOME");
    page_collection_print(news, "NEWS");
    page_collection_destroy(home);
    page_collection_destroy(news);
    api_destroy();
    return 0;
}
