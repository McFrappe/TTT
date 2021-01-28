#include "pages.h"

void page_collection_print(page_collection_t *collection, const char *name) {
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

void page_destroy(page_t *page) {
    free(page->title);
    // TODO: Free content
}

void page_collection_destroy(page_collection_t *collection) {
    for (size_t i = 0; i < collection->size; i++) {
        page_destroy(collection->pages[i]);
    }

    free(collection);
}
