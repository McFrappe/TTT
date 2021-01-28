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

void print_parsed_collection(page_collection_t *collection, const char *name) {
    for (size_t i = 0; i < collection->size; i++) {
        printf("%s: PAGE %ld\n", name, i);
        printf("* id: %d\n", collection->pages[i]->id);
        printf("* prev_id: %d\n", collection->pages[i]->prev_id);
        printf("* next_id: %d\n", collection->pages[i]->next_id);
        printf("* unix_date: %zd\n", collection->pages[i]->unix_date);
        printf("* title: %s\n", collection->pages[i]->title);
        printf("* content_size: %zd\n", collection->pages[i]->content_size);
        // TODO: Print parsed content
        printf("\n");
    }
}

int main(void) {
    api_intialize();
    page_collection_t *home = api_get_page_range(HOME, FOREIGN_NEWS);
    page_collection_t *news = api_get_page(NEWS);
    print_parsed_collection(home, "HOME");
    print_parsed_collection(news, "NEWS");
    destroy_page_collection(home);
    destroy_page_collection(news);
    api_destroy();
    return 0;
}
