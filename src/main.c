#include "ui.h"
#include "api.h"
#include "pages.h"
#include "parser.h"

int main(void) {
    ui_initialize();
    api_initialize();
    
    page_collection_t *test = api_get_page_range(HOME, FOREIGN_NEWS);
    page_collection_print(test, "TEST");
    page_collection_destroy(test);
    api_destroy();
    ui_destroy();
    return 0;
}
